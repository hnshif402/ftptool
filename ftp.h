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
#include "ftpregex.h"
#include "ftplib.h"

#define THREAD_NUM 3

#define QSIZE 255
#define BUFSIZE 1024

typedef struct ftp_operations {
  void (*Init)(void);
  int (*Connect)(const char *host, netbuf **nCtl);
  int (*Login)(const char *user, const char *pass, netbuf *nCtl);
   /* FTPLIB_CONNMODE: FTPLIB_PASSIVE or FTPLIB_PORT */
  int (*Options)(int opt, long val, netbuf *nCtl);
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

void *process_queue(void* arg);
char *fetch_one(queue_t* qp);
void *queue_fill(queue_t* qpt, DIR* dpt, regex_t* preg);
int uploadfile(char* filename, ftp_t* ftp);
void queue_add(char* filename, queue_t* qpt);
void queue_init(queue_t* q);
void ftp_release(ftp_t* ftp);
void ftp_init(ftp_t*, ftp_oper_t*, char*, char*, char*, char* , char*) ;
void copyfile(char *filename, char *path);
void movefile(char *filename, char *path);
int matchstr(char *str, char *substr);

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
#endif
