#include "ftpregex.h"


int regex_init(regex_t *preg, char *p)
{
	int s;
	char err[255];
	if(!p)
    {
        printf("pattern is NULL.\n"); 
        exit(-1);
    }
	s = regcomp(preg, p, REG_EXTENDED | REG_NOSUB );
	if( s ) {
		regerror(s, NULL, err, sizeof(err));
		printf("regcomp error: %s\n", err);
	    exit(-1);
	}
	
	return 0;
}

int match(regex_t *preg, const char *filename)
{
	if( ! preg && filename ) 
    {
        printf("match error.\n"); 
        exit(-1);
    }
	int s;
	regmatch_t pmatch[1];
	const size_t nmatch = 1;
	s = regexec(preg, filename, nmatch, pmatch, 0);

	return s == REG_NOMATCH ? 0 : 1;
}
	


