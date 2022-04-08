
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


u_char *str_nstr(u_char *s1, char *s2, size_t len)
{
    u_char  c1, c2;
    size_t  n;

    c2 = *(u_char *) s2++;
    n = strlen(s2);
    do {
        do {
            if (len-- == 0) {
                return NULL;
            }
            c1 = *s1++;

            if (c1 == 0) {
                return NULL;
            }

        } while (c1 != c2);

        if (n > len) {
            return NULL;
        }

    } while (strncmp((char *)s1,  s2, n) != 0);
    return --s1;
}




