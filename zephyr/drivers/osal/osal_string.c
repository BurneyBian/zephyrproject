#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include "osal.h"

char *osal_strcpy(char *dest, const char *src)
{
    return strcpy(dest, src);
}

extern size_t strlcpy(char *dest, const char *src, size_t size);
int osal_strlcpy(char *dest, const char *src, int size)
{
    return strlcpy(dest, src, size);
}

char *osal_strcat(char *dest, const char *src)
{
    return strcat(dest, src);
}
char *osal_strncat(char *dest, const char *src, int count)
{
    return strncat(dest, src, count);
}

extern size_t strlcat(char *dest, const char *src, size_t destsz);
int osal_strlcat(char *dest, const char *src, int count)
{
    return strlcat(dest, src, count);
}

int osal_strnicmp(const char *s1, const char *s2, int len)
{
	osal_printk("%s Not support!\n", __FUNCTION__);
    return 0;
}

extern int strncasecmp(const char *s1, const char *s2, size_t n);
int osal_strncasecmp(const char *s1, const char *s2, int n)
{
    return strncasecmp(s1, s2, n);
}

char *osal_strchr(const char *s, int c)
{
    return strchr(s, c);
}
char *osal_strnchr(const char *s, int count, int c)
{
	osal_printk("%s Not support!\n", __FUNCTION__);
    return 0;
}

char *osal_strrchr(const char *s, int c)
{
    return strrchr(s, c);
}

extern char *strnstr(const char *s, const char *find, size_t n);
char *osal_strnstr(const char *s1, const char *s2, int len)
{
    return strnstr(s1, s2, len);
}

char *osal_strpbrk(const char *cs, const char *ct)
{
    return strpbrk(cs, ct);
}

extern char *strsep(char **stringp, const char *delim);
char *osal_strsep(char **s, const char *ct)
{
    return strsep(s, ct);
}

int osal_strspn(const char *s, const char *accept)
{
    return strspn(s, accept);
}
int osal_strcspn(const char *s, const char *reject)
{
    return strcspn(s, reject);
}

void *osal_memscan(void *addr, int c, int size)
{
    return NULL;
}

void *osal_memchr(const void *s, int c, int n)
{
    return memchr(s, c, n);
}
void *osal_memchr_inv(const void *start, int c, int bytes)
{
    return NULL;
}
unsigned long long osal_strtoull(const char *cp, char **endp, unsigned int base)
{
    return 0;
}
unsigned long osal_strtoul(const char *cp, char **endp, unsigned int base)
{
    return strtoul(cp, endp, base);
}
long osal_strtol(const char *cp, char **endp, unsigned int base)
{
	osal_printk("%s Not support!\n", __FUNCTION__);
    return 0;
}
long long osal_strtoll(const char *cp, char **endp, unsigned int base)
{
    return 0;
}

int osal_scnprintf(char *buf, int size, const char *fmt, ...)
{
	osal_printk("%s Not support!\n", __FUNCTION__);
    return 0;
}

int osal_sscanf(const char *buf, const char *fmt, ...)
{
    osal_printk("Do not support\n");

    return -ENOTSUP;
}

int osal_vsnprintf(char *str, int size, const char *fmt, osal_va_list args)
{
    return vsnprintf(str, size, fmt, args);
}

void *osal_memcpy(void *dst, const void *src, unsigned long count)
{
	return memcpy(dst, src, count);
}

int osal_memcmp(const void *cs, const void *ct, size_t count)
{
	return memcmp(cs, ct, count);
}

void *osal_memset(void *src, int c, unsigned long n)
{
	return memset(src, c, n);
}

void *osal_memset_io(void *src, int c, unsigned long n)
{
	return memset(src, c, n);
}

void *osal_memmove(void *dest, const void *src, unsigned long n)
{
	return memmove(dest, src, n);
}

char *osal_strstr(const char *str1, const char *str2)
{
	return strstr(str1, str2);
}

char *osal_strncpy(char *dest, const char *src, size_t n)
{
	return strncpy(dest, src, n);
}

int osal_strncmp(const char *cs, const char *ct, size_t count)
{
	return strncmp(cs, ct, count);
}

int osal_strcmp(const char *cs, const char *ct)
{
	return strcmp(cs, ct);
}

int osal_strcasecmp(const char *a, const char *b)
{
	return strcasecmp(a, b);
}

size_t osal_strlen(const char *src)
{
	return strlen(src);
}

size_t osal_strnlen(const char *s, unsigned long maxlen)
{
	return strnlen(s, maxlen);
}

void __osal_assert(void)
{
	assert(0);
}
