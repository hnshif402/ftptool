#include "ftp.h"
	
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

void queue_init(queue_t *q)
{
	memset(q->fname, 0, QSIZE * sizeof(char *));
	pthread_mutex_init(&q->q_lock, NULL);
	pthread_barrier_init(&q->q_b, NULL, THREAD_NUM+1);
	q->first = 0;
	q->last = 0;
	q->qlen = 0;
	q->ready = 0;
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

void copyfile(char* filename, char* path)
{
	if(filename == NULL || path == NULL) 
	{
		printf("filename or path is NULL\n");
		exit(-1);
	}
	char buf[BUFSIZE]; 
	FILE *file;
	FILE *newfile;
	if ((file = fopen(filename, "rb")) == NULL )
	{
		printf("Open file %s failed.\n", filename);
		exit(-1);
	}
	if ((newfile = fopen(strcat(path, filename), "wb")) == NULL )
	{
		printf("create file %s failed.\n", strcat(path, filename));
		exit(-1);
	}
	
	while (fgets(buf, BUFSIZE, file))
		fputs(buf, newfile);
	
	fclose(file);
	fclose(newfile);
}

void movefile(char *filename, char *path)
{
	if(filename == NULL || path == NULL) 
	{
		printf("filename or path is NULL\n");
		exit(-1);
	}
	if( (rename(filename, strcat(path, filename))) == -1 )
	{
		printf("move %s to %s failed.\n", filename, strcat(path, filename));
		exit(-1);
	}
}

int matchstr(char *str, char *substr)
{
	if( substr == NULL || str == NULL )
	{
		printf("matchstr failed.\n");
		exit(-1);
	}
	if(strstr(str, substr) == NULL)
		return 0;
	return 1;
}
