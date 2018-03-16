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
	pthread_mutex_init(&cond_lock, NULL);
	queue_t q;
	ftp_t ftp;

	queue_init(&q);
	ftp_init(&ftp, argv[2], argv[3], argv[4], argv[5]);
	DIR *dir;
	struct dirent *dent;

	if((dir = opendir("./")) == NULL) 
	{
		printf("open dir %s error.\n", argv[1]);
		exit(1);
	}
	int i, ret;
	pthread_t tid;
	arg_t t = {&ftp, &q};
	for(i = 0; i < THREAD_NUM; i++)
	{
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
	  while((dent = readdir(dir)) != NULL)
	  {
	    if(!(dent->d_type & DT_REG)) continue;
	    pthread_mutex_lock(&q.q_lock);
	     queue_add(dent->d_name, &q);
	     if(qfull(&q)) {
		 pthread_mutex_unlock(&q.q_lock);
		 break;
	       }
		pthread_mutex_unlock(&q.q_lock);
	     printf("file: %s\n", dent->d_name);
	  }
	  sleep(5);
	  printf("send broadcast\n");
	  for(int i=0; i < THREAD_NUM; i++)
	    pthread_cond_broadcast(&q.q_ready);
	 
	}
}
