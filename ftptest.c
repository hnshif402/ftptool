#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include "ftplib.h"

int main(int argc, char* argv[])
{
  if(argc < 6)
  {
    printf("./ftptest local_dir ftp_host ftp_user ftp_pass remote_dir\n");
    exit(1);
  }

  DIR *dir;
  struct dirent *dent;

  if(chdir(argv[1]) == -1)
  {
    printf("Enter dir %s failed.\n", argv[1]);
    exit(1);
  }
  
  if((dir = opendir("./")) == NULL) {
    printf("opendir %s failed.\n", argv[1]);
    exit(1);
  }


  netbuf *conn;
  FtpInit();
  if(!FtpConnect(argv[2], &conn))
  {
    printf("Connect ftp server %s failed.\n", argv[2]);
    exit(1);
  }

  if(!FtpLogin(argv[3], argv[4], conn))
  {
    printf("Login ftp server %s %s %s failed.\n", argv[2], argv[3], argv[4]);
    exit(1);
  }

  if(!FtpChdir(argv[5], conn))
  {
    printf("Chdir %s failed.\n", argv[5]);
    exit(1);
  }

  while((dent = readdir(dir)) != NULL) {
    if(!FtpPut(dent->d_name, dent->d_name, "FTP_ASCII", conn)) {
      printf("ftp put %s faild.\n", dent->d_name);
      exit(1);
    }

    printf("ftp put %s success.\n", dent->d_name);
  }
      
  FtpQuit(conn);

  return 0;
}
