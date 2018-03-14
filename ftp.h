#ifndef __FTP_H
#define __FTP_H

#include <unistd.h>
#include <sys/types.h> 
#include <dirent.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
//#include <regex.h>
#include "ftplib.h"

#define THREAD_NUM 3
#define FTPMOD FTPLIB_ASCII

#define QSIZE 255
#define FTP_PUT_MODE "FTPLIB_ASCII"
#define PATTERN "*.bin$"


typedef struct ftp_operations {
  void (*Init)(void);
  int (*Connect)(const char *host, netbuf **nCtl);
  int (*Login)(const char *user, const char *pass, netbuf *nCtl);
  int (*Chdir)(const char *path, netbuf *nCtl);
  int (*Put)(const char *input, const char *path, char mode, netbuf *nCtl);
  void (*Quit)(netbuf *nCtl);
} ftp_oper_t;


typedef struct ftp {
	netbuf *nCtl;
	ftp_oper_t *ftp_ops;
} ftp_t;

static ftp_oper_t _ftpops = { \
	.Init = FtpInit,	\
	.Connect = FtpConnect,	\
	.Login = FtpLogin,	\
	.Chdir = FtpChdir,	\
	.Put = FtpPut,	\
	.Quit = FtpQuit,	\
};

void ftp_init(ftp_t *ftp, char *host, char *user, char *pass, char* path) 
{
  ftp->nCtl = NULL;
	ftp->ftp_ops = &_ftpops;
	ftp->ftp_ops->Init();
	if(!ftp->ftp_ops->Connect(host, &ftp->nCtl))
	{
		printf("connect to server %s failed.\n", host);
		exit(1);
	}

	if(!ftp->ftp_ops->Login(user, pass, ftp->nCtl))
	{
		printf("login server %s failed.\n", host);
		exit(1);
	}

	if(!ftp->ftp_ops->Chdir(path, ftp->nCtl))
	{
		printf("chdir %s failed\n", path);
		exit(1);
	}
	printf("login ftp server success.\n");
};

void ftp_release(ftp_t *ftp)
{
	ftp->ftp_ops->Quit(ftp->nCtl);
}


typedef struct queue {
  char *fname[QSIZE];
  int qlen;
  int first;
  int last;
  pthread_mutex_t q_lock;
  pthread_barrier_t q_b;
  pthread_cond_t q_ready;
} queue_t;

typedef struct param {
  ftp_t *ftp;
  queue_t *q;
} arg_t;

int queue_init(queue_t *q)
{
	memset(q->fname, 0, QSIZE * sizeof(char *));
	pthread_mutex_init(&q->q_lock, NULL);
	pthread_barrier_init(&q->q_b, NULL, THREAD_NUM+1);
	pthread_cond_init(&q->q_ready, NULL);
	q->first = 0;
	q->last = 0;
	q->qlen = 0;
};

static int qlen(const queue_t *q)
{
	return q->qlen;
};

static int qfull(const queue_t *q)
{
	return (q->qlen == QSIZE);
};

static int qempty(const queue_t *q)
{
	return q->qlen == 0;
};
void queue_add(char *filename, queue_t *q)
{
  while(pthread_mutex_lock(&q->q_lock) && qfull(q))
  {
    pthread_mutex_unlock(&q->q_lock);
    printf("Queue is full,waiting 1 second.");
    sleep(1);
   }
   q->fname[q->last] = filename;
   q->last = (q->last++) % QSIZE;
   q->qlen++;
   pthread_mutex_unlock(&q->q_lock);
};

int uploadfile(char *filename, ftp_t *ftp)
{
  printf("prepare to upload file %s\n", filename);
	return (ftp->ftp_ops->Put(filename, filename, FTPLIB_ASCII, ftp->nCtl));
};

void * queue_del(void *arg)
{
  arg_t *t = (arg_t*) arg;
  ftp_t *ftp = t->ftp;
  queue_t *q = t->q;
  char *filename;
  char errname[255];
  memset(errname, 0, 255 * sizeof(char));
  for(;;)
  {
    pthread_mutex_lock(&q->q_lock);
    if(qempty(q))
    {
      pthread_mutex_unlock(&q->q_lock);
      printf("qlen is 0, waiting\n");
      pthread_barrier_wait(&q->q_b);
      pthread_cond_wait(&q->q_ready, &q->q_lock);
      printf("continue..\n");
      continue;
    }
    printf("PWD:%s\n", getcwd(NULL, 0));
    printf("queue len is: %d\n", q->qlen);
    printf("q->first is: %d\n", q->first);
    printf("filename %s\n", q->fname[q->first]);
    filename = q->fname[q->first];
    q->first = q->first++ % QSIZE;
    printf("q->first is %d\n", q->first);
    q->qlen--;
    printf("queue len is: %d\n", q->qlen);
    pthread_mutex_unlock(&q->q_lock);
    if(!uploadfile(filename, ftp)){
      printf("Upload %s failed.\n", filename);
      memcpy(errname, filename, strlen(filename)+1);
      printf("filename %s\n", errname);
      rename(errname, strcat(errname, ".err"));
      char *mesg = strerror(errno);
      printf("error: %s\n", mesg);
      printf("errname %s\n", errname);
      exit(1);
    }
    printf("%s upload success.\n", filename);
    if(remove(filename))
    {
      printf("delete file %s failed.", filename);
      memcpy(errname, filename, strlen(filename)+1);
      printf("filename %s\n", errname);
      rename(errname, strcat(errname, ".err"));
    }
    }
};

/*
char* get_filename(struct dirent *dent)
{
	return dent->d_name;
};
*/
#endif
