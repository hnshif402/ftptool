#include "ftp.h"


int main(int argc, char *argv[])
{
	if(argc < 6) 
	{
		printf("./ftp local_dir ftp_host, ftp_user, ftp_pass, remote_dir\n");
		exit(1);
	}

	if(chdir(argv[1]))
	{
	  printf("can't entry dir %s\n", argv[1]);
	  exit(1);
	}
	queue_t q;
	ftp_t ftp[THREAD_NUM];
	
	int i;
	queue_init(&q);
	for (i = 0; i < THREAD_NUM; i++)
	  ftp_init(&ftp[i], argv[2], argv[3], argv[4], argv[5]);
	DIR *dir;
	struct dirent *dent;

	if((dir = opendir("./")) == NULL) 
	{
		printf("open dir %s error.\n", argv[1]);
		exit(1);
	}
	
	int ret;
	pthread_t tid;
	arg_t t;
	for(i = 0; i < THREAD_NUM; i++)
	{
	  t.ftp = &ftp[THREAD_NUM];
	  t.q = &q;
	  ret = pthread_create(&tid, NULL, queue_del, &t);
	  if(ret != 0)
	  {
	    printf("create thread failed.\n");
	    exit(1);
	  }
	  printf("create thread %ld success.\n", tid);
	}
	for(;;)
	{
	  pthread_barrier_wait(&q.q_b);
	  printf("readdir begin...\n");
	  rewinddir(dir);
	  pthread_mutex_lock(&q.q_lock);
	  while((dent = readdir(dir)) != NULL)
	  {
	    if(!(dent->d_type & DT_REG)) continue;
	     queue_add(dent->d_name, &q);
	     //printf("file: %s\n", dent->d_name);
	     if(qfull(&q))  break;
	  }
	  pthread_mutex_unlock(&q.q_lock);
	  sleep(5);
	  printf("send broadcast\n");
	  pthread_mutex_lock(&q.c_lock);
	  if(!qempty(&q))
	    pthread_cond_broadcast(&q.q_ready);
	  pthread_mutex_unlock(&q.c_lock);
	 
	}
}
