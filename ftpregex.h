#ifndef __FTPREGEX_H
#define __FTPREGEX_H

#include <unistd.h>
#include <regex.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


/* init regex */
int regex_init(regex_t *preg, char *p) ;
/* define match function to match filename */
int match(regex_t *preg, const char *filename) ;

#endif
