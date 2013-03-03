
#include "path.h"

char * path_concat( char* path1, int len1, char * path2 ) 
{
    int len2 = strlen(path2);
    char * newpath = emalloc( sizeof(char) * (len1 + len2 + 2) );
    char * src = path1;
    char * p2 = newpath;
    while( len1-- ) {
        *p2 = *src;
        p2++;
        src++;
    }

    if(p2 > path1 && *(p2-1) == DEFAULT_SLASH ) {

    } else {
        *p2 = DEFAULT_SLASH;
        p2++;
    }

    src = path2;
    while( len2-- ) {
        *p2 = *src;
        src++;
        p2++;
    }
    *p2 = '\0';
    return newpath;
}
