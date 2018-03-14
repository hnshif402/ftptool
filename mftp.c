#include <unistd.h>
#include <stdio.h>

FILE *openftp(char *ip, char *user, char *pass);
int mftp(FILE *fp, char *cmd, char *file);
int main(int argc, char *argv[]) {
    if (argc < 5) {
        printf("Usage:mftp ip user pass cmd file");
        exit(-1);
    }
    FILE *ftp;

    if((ftp = openftp(argv[1], argv[2], argv[3])) == NULL) {
        printf("open ftp %s error.", argv[1]);
        exit(-1);
    }
    if (mftp(ftp, argv[4], argv[5]) > 0)
        printf("%s success.\n", argv[4]);

    pclose(ftp);

    return 0;
}

FILE *openftp(char *ip, char *user, char *pass)
{
    FILE *fp;
    if((fp = popen("/cygdrive/c/windows/system32/ftp -n\n", "w")) == NULL)
        return NULL;
    fprintf(fp, "open %s\n", ip);
    fprintf(fp, "user %s %s\n", user, pass);
    
    return fp;
}

int mftp(FILE *fp, char *cmd, char *file)
{
    if (!fp) return -1;
    return fprintf(fp, "%s %s\n", cmd, file);
}



