#ifndef __FTP_H
#define __FTP_H

#include <unistd.h>
#include <sys/types.h> 
#include <dirent.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>
#include "ftplib.h"

#define THREAD_NUM 3

#define QSIZE 255
#define PATTERN "*.bin$"

typedef struct ftp_operations {
  void (*Init)(void);
  int (*Connect)(const char *host, netbuf **nCtl);
  int (*Login)(const char *user, const char *pass, netbuf *nCtl);
  int (*Options)(int opt, long val, netbuf *nCtl); /* FTPLIB_CONNMODE: FTPLIB_PASSIVE or FTPLIB_PORT */
  int (*Chdir)(const char *path, netbuf *nCtl);
  int (*Put)(const char *input, const char *path, char mode, netbuf *nCtl);
  void (*Quit)(netbuf *nCtl);
} ftp_oper_t;

#define FTP_OPS_INIT(ops) { \
	ops.Init = FtpInit; \
	ops.Connect = FtpConnect; \
	ops.Login = FtpLogin; \
	ops.Options = FtpOptions; \
	ops.Chdir = FtpChdir; \
	ops.Put = FtpPut; \
	ops.Quit = FtpQuit; \
}

typedef struct ftp {
	netbuf *nCtl;
	char *rpath;
	int mode;
	ftp_oper_t *ftp_ops;
} ftp_t;

typedef struct queue {
  char *fname[QSIZE];
  int qlen;
  int first;
  int last;
  int ready;
  pthread_barrier_t q_b;
  pthread_mutex_t q_lock;
} queue_t;

typedef struct param {
  ftp_t *ftp;
  queue_t *q;
} ftp_arg_t;

void ftp_init(ftp_t *ftp, ftp_oper_t *f_ops, char *host, char *user, char *pass, char* path, char *ftp_mode) 
{

    ftp->nCtl = NULL;
	ftp->rpath = path;
	ftp->ftp_ops = f_ops;
	ftp->mode = (int)(*ftp_mode - 48);
	ftp->ftp_ops->Init();
	if(!ftp->ftp_ops->Connect(host, &(ftp->nCtl)))
	{
		printf("connect to server %s failed.\n", host);
		exit(1);
	}
	
	if(!ftp->ftp_ops->Login(user, pass, ftp->nCtl))
	{
		printf("login server %s failed.\n", host);
		exit(1);
	}

	if(!ftp->ftp_ops->Options(FTPLIB_CONNMODE, ftp->mode, ftp->nCtl))
	{
		printf("ftp passive %s failed.\n", host);
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

int queue_init(queue_t *q)
{
	memset(q->fname, 0, QSIZE * sizeof(char *));
	pthread_mutex_init(&q->q_lock, NULL);
	pthread_barrier_init(&q->q_b, NULL, THREAD_NUM+1);
	q->first = 0;
	q->last = 0;
	q->qlen = 0;
	q->ready = 0;
};

static int qlen(const queue_t *q)
{
	return q->qlen;
};

static int qfull(const queue_t *q)
{
  return q->qlen == QSIZE;
};

static int qempty(const queue_t *q)
{
	return q->qlen == 0;
};
void queue_add(char *filename, queue_t *qpt)
{
   if( ! filename || ! qpt) {
	   printf("file name or q value is invalid.\n");
	   exit(1);
   }
   qpt->fname[qpt->last] = filename;
   qpt->last = (++qpt->last) % QSIZE;
   qpt->qlen++;
};

int uploadfile(char *filename, ftp_t *ftp)
{
  printf("prepare to upload file %s\n", filename);
  return (ftp->ftp_ops->Put(filename, filename, FTPLIB_ASCII, ftp->nCtl));
};


void * queue_fill(queue_t *qpt, DIR *dpt) {
	queue_t *qp = qpt;
    DIR *dp = dpt;
    struct dirent *dent;
	for(;;)
	{
	  printf("readdir begin...\n");
	  rewinddir(dp);
	  pthread_mutex_lock(&qp->q_lock);
	  while((dent = readdir(dp)) != NULL)
	  {
	    if(!(dent->d_type & DT_REG)) continue;
	      queue_add(dent->d_name, qp);
	     if(qfull(qp))  break;
	  }
	  printf("qlen = %d, first = %d, last = %d\n", qp->qlen, qp->first, qp->last);
	  pthread_mutex_unlock(&qp->q_lock);
	  if(qempty(qp))
      {
	    sleep(1);
	    continue;
	  }
	  pthread_barrier_wait(&qp->q_b);
	}    
}

char *fetch_one(queue_t *qp) {
	if( qp == NULL ) {
		printf("fetch_one get null parameter.\n");
		exit(1);
	}
	char *fname = NULL;
	while(1)
    {
      pthread_mutex_lock(&qp->q_lock);
      if( !qempty(qp) )
      {
        pthread_mutex_unlock(&qp->q_lock);
	    break;
      }
      pthread_mutex_unlock(&qp->q_lock);
      pthread_barrier_wait(&qp->q_b);
    }
    pthread_mutex_lock(&qp->q_lock);
    fname = qp->fname[qp->first];
    qp->fname[qp->first] = NULL;
    qp->first = (++qp->first) % QSIZE;
    qp->qlen--;
    pthread_mutex_unlock(&qp->q_lock);
	
	return fname;
}
void * process_queue(void *arg)
{
  sleep(5);
  pthread_t pid;
  pid = pthread_self();
  ftp_arg_t *t = (ftp_arg_t*) arg;
  ftp_t *ftp = t->ftp;
  queue_t *qpt = t->q;
  printf("%ld: t->q: %X.\n", pid, t->q);
  printf("%ld: t->ftp: %X.\n", pid, t->ftp);
  printf("%ld: netbuf->handle: %X.\n", pid, ftp->nCtl);
  char *filename;
  for(;;)
   {
	filename = fetch_one(qpt);
	if(filename == NULL) {
		printf("fetch item error.\n");
		exit(1);
	}
	printf("%ld: netbuf->handle: %X.\n", pid, ftp->nCtl);
    printf("%ld: prepare to upload %s...\n", pid, filename);
    if(!ftp->ftp_ops->Put(filename, filename, FTPLIB_ASCII, ftp->nCtl))
      printf("%ld: Upload %s failed.\n", pid, filename);
      //exit(1);
    else
      printf("%ld: %s upload success.\n",pid, filename);
    
    if(remove(filename) == -1)
       printf("%ld: delete file %s failed.", pid, filename);
    else
        printf("%ld: delete success %s\n", pid, filename);
   }
}
/*
char* get_filename(struct dirent *dent)
{
	return dent->d_name;
};
*/
#endif
