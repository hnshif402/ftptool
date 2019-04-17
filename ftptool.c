#include <unistd.h>
#include "ftp.h"


int main(int argc, char *argv[])
{
	if(argc < 12) 
	{
		printf("./ftp localdir ftphost ftpuser ftppass remotedir ftpmode pattern kpitypes kpipath exclusivetypes exclupath\n");
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
	
	queue_t q;	
	queue_init(&q);
	pthread_t tid;
	int ret;
	ftp_arg_t t[THREAD_NUM];

	for(i = 0; i < THREAD_NUM; i++)
	{
	  t[i].ftp = &ftp[i] ;
	  t[i].q = &q;
	  ret = pthread_create(&tid, NULL, process_queue, &t[i]);
	  if(ret != 0)
	  {
	    printf("create thread failed.\n");
	    exit(1);
	  }
	  printf("create thread %ld success.\n", tid);
	}
	
    char *pattern = malloc(strlen(argv[7])+1);
	memcpy(pattern, argv[7], strlen(argv[7])+1);
	
	char *kpitypes = malloc(strlen(argv[8])+1);
	memcpy(kpitypes, argv[8], strlen(argv[8])+1);
	
    char *kpipath = malloc(strlen(argv[9]+1));
    memcpy(kpipath, argv[9], strlen(argv[9]+1));

	char *exclutypes = malloc(strlen(argv[10])+1);
	memcpy(exclutypes ,argv[10] , strlen(argv[10])+1);

    char *exclupath = malloc(strlen(argv[11])+1);
    memcpy(exclupath, argv[11], strlen(argv[11])+1);

	regex_t r1;
	if( regex_init(&r1, pattern) == -1 ) 
	{
		printf("init regex failed.\n"); 
		exit(-1);
	}

    char *filename;
    struct dirent *dent;
	for(;;)
	{
	  printf("readdir begin...\n");
	  rewinddir(dir);
	  pthread_mutex_lock(&q.q_lock);
	  while((dent = readdir(dir)) != NULL)
	  {
	    if(!(dent->d_type & DT_REG)) continue;
		filename = dent->d_name;
		if( ! match(&r1, filename) ) continue;
		if( matchstr(filename, kpitypes))
			copyfile(filename, kpipath);
		if( matchstr(filename, exclutypes) )
			movefile(filename, exclupath);
	    queue_add(filename, &q);
	    if(qfull(&q))  break;
	  }
	  printf("qlen = %d, first = %d, last = %d\n", q.qlen, q.first, q.last);
	  pthread_mutex_unlock(&q.q_lock);
	  if(qempty(&q))
      {
	    sleep(1);
	    continue;
	  }
	  pthread_barrier_wait(&q.q_b);
	}   
	
	exit(0);
}
