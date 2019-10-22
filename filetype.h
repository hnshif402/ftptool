#ifndef  _FILE_TYPE_H
#define _FILE_TYPE_H

#include <unistd.h>
#include <string.h>


#define MV 0
#define CP 1
#define EX 2

#define CHECK_STRING_NUM 9 /* floor(64/7) */
#define CHAR_SHIFT_NUM 7
struct filetype {
    char filname[255];
    int op;
    struct filetype *next;
};

#define MAX_TYPE_NUM 2048
#define TYPE_SHIFT_NUM 11
//struct filetype *head[MAX_TYPE_NUM];

#define INIT_FILETYPE_HEAD(head, l) { memset(head, 0, l); }

static inline void _link_add( struct filetype *head, struct filetype *newtype )
{
    newtype->next = head;
    head = newtype;
}

#define _link_for_each(pos, head) \
    for (pos = head; pos->next != NULL; pos = pos->next)


static unsigned int  hash_index(unsigned long  val)
{
	unsigned long hash = val;

	/*  Sigh, gcc can't optimise this alone like it does for 32 bits. */
	unsigned long n = hash;
	n <<= 18;
	hash -= n;
	n <<= 33;
	hash -= n;
	n <<= 3;
	hash += n;
	n <<= 3;
	hash -= n;
	n <<= 4;
	hash += n;
	n <<= 2;
	hash += n;

	/* High bits are more random, so use them. */
	return hash >> (64 - TYPE_SHIFT_NUM) ;
}

unsigned long string_code(const char *s)
{
    if (! s) {
        printf("parameter error in string_code func."); 
        exit(-1);
    }
    long int retcode = 0;
    char *p = s;
    int i = 0;
    for(; i < CHECK_STRING_NUM && *p; i++, p++)
        retcode = retcode + *p << CHAR_SHIFT_NUM ;
    return retcode;
}

int filetype_index(const char *filetype)
{
    unsigned long filetype_code = 0;
    int index = 0;
    filetype_code = string_code(filetype);
    index = hash_index(filetype_code);
    return index;
}

void filetype *alloc_filetype(char *type, int op, struct filetype *head)
{
    struct filetype *t = (struct filetype *) malloc(sizeof(struct filetype));
    t->next = NULL;
    t->op = op;
    memset(t->filname, 0, 255);
    int l = strlen(type) > 255 ? 255 : strlen(type) + 1;
    strncp(t->filename, type, l);
    int index = filetype_index( type );
    _link_add(head[index], t);
}

struct filetype *find_type(char *type, struct filetype *head) 
{
    if (! type || ! head) 
    {
        printf("type or head is wrong in find_type func\n");
        exit(-1);
    }
    int index = file_type_index(type);
    struct typefile *h = head[index];
    _link_for_each(p, h)
    {
        if( strcmp(p.filename, type) == 0 )
            return p;
    }
    return NULL;
}
#endif
