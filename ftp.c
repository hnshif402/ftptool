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
	  printf("create thread %d success.\n", tid);
	}
	for(;;)
	{
	  pthread_barrier_wait(&q.q_b);
	  printf("readdir begin...\n");
	  while((dent = readdir(dir)) != NULL)
	  {
	    if(!(dent->d_type & DT_REG)) continue;
	     queue_add(dent->d_name, &q);
	     printf("file: %s\n", dent->d_name);
	  }
	  pthread_cond_broadcast(&q.q_ready);
	}
}
