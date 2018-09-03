#include "ftp.h"
	
int main(int argc, char *argv[])
{
	if(argc < 7) 
	{
		printf("./ftp local_dir ftp_host, ftp_user, ftp_pass, remote_dir ftp_mode\n");
		exit(1);
	}

	if(chdir(argv[1]))
	{
	  printf("can't entry dir %s\n", argv[1]);
	  exit(1);
	}

	ftp_t ftp[THREAD_NUM];	
	ftp_oper_t ftp_op[THREAD_NUM];
	int i;
	for (i = 0; i < THREAD_NUM; i++)
		FTP_OPS_INIT(ftp_op[i]);

	queue_t q;	
	queue_init(&q);
	for (i = 0; i < THREAD_NUM; i++)
	{
	  ftp_init(&ftp[i], &ftp_op[i], argv[2], argv[3], argv[4], argv[5], argv[6]);
	}
	
	DIR *dir;
	if((dir = opendir("./")) == NULL) 
	{
		printf("open dir %s error.\n", argv[1]);
		exit(1);
	}
	
	pthread_t tid;
	int ret;
	ftp_arg_t t[THREAD_NUM];
	printf("ftp addr: %X %X %X\n", &ftp[0], &ftp[1], &ftp[2]);
	
	for(i = 0; i < THREAD_NUM; i++)
	{
	  t[i].ftp = &ftp[i] ;
	  t[i].q = &q;
	  printf("before create pthread: t->ftp: %X, t->q: %X\n", t[i].ftp, t[i].q);
	  ret = pthread_create(&tid, NULL, process_queue, &t[i]);
	  if(ret != 0)
	  {
	    printf("create thread failed.\n");
	    exit(1);
	  }
	  printf("create thread %ld success.\n", tid);
	}

	//char *pattern = "*[245]\\.bin$";
	regex_t r1;
	char *ppattern = "f*[245]\\.bin$";
	if( regex_init(&r1, ppattern) == -1 ) {printf("init regex failed.\n"); exit(1);}
	queue_fill(&q, dir, &r1) ;
	
	exit(0);
}
