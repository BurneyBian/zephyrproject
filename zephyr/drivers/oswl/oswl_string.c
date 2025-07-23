#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <rtthread.h>
#include "oswl.h"

char *oswl_strcpy(char *dest, const char *src)
{
    return strcpy(dest, src);
}

extern size_t strlcpy(char *dest, const char *src, size_t size);
int oswl_strlcpy(char *dest, const char *src, int size)
{
    return strlcpy(dest, src, size);
}

char *oswl_strcat(char *dest, const char *src)
{
    return strcat(dest, src);
}
char *oswl_strncat(char *dest, const char *src, int count)
{
    return strncat(dest, src, count);
}

extern size_t strlcat(char *dest, const char *src, size_t destsz);
int oswl_strlcat(char *dest, const char *src, int count)
{
    return strlcat(dest, src, count);
}

extern int strncasecmp(const char *s1, const char *s2, size_t n);
int oswl_strncasecmp(const char *s1, const char *s2, int n)
{
    return strncasecmp(s1, s2, n);
}

char *oswl_strchr(const char *s, int c)
{
    return strchr(s, c);
}

char *oswl_strrchr(const char *s, int c)
{
    return strrchr(s, c);
}

extern char *strnstr(const char *s, const char *find, size_t n);
char *oswl_strnstr(const char *s1, const char *s2, int len)
{
    return strnstr(s1, s2, len);
}

char *oswl_strpbrk(const char *cs, const char *ct)
{
    return strpbrk(cs, ct);
}

extern char *strsep(char **stringp, const char *delim);
char *oswl_strsep(char **s, const char *ct)
{
    return strsep(s, ct);
}

int oswl_strspn(const char *s, const char *accept)
{
    return strspn(s, accept);
}
int oswl_strcspn(const char *s, const char *reject)
{
    return strcspn(s, reject);
}

void *oswl_memchr(const void *s, int c, int n)
{
    return memchr(s, c, n);
}

unsigned long oswl_strtoul(const char *cp, char **endp, unsigned int base)
{
    return strtoul(cp, endp, base);
}

int oswl_vsnprintf(char *str, int size, const char *fmt, va_list args)
{
    return vsnprintf(str, size, fmt, args);
}

void *oswl_memcpy(void *dst, const void *src, unsigned long count)
{
	return rt_memcpy(dst, src, count);
}

int oswl_memcmp(const void *cs, const void *ct, size_t count)
{
	return rt_memcmp(cs, ct, count);
}

void *oswl_memset(void *src, int c, unsigned long n)
{
	return rt_memset(src, c, n);
}

void *oswl_memmove(void *dest, const void *src, unsigned long n)
{
	return rt_memmove(dest, src, n);
}

char *oswl_strstr(const char *str1, const char *str2)
{
	return rt_strstr(str1, str2);
}

char *oswl_strncpy(char *dest, const char *src, size_t n)
{
	return rt_strncpy(dest, src, n);
}

int oswl_strncmp(const char *cs, const char *ct, size_t count)
{
	return rt_strncmp(cs, ct, count);
}

int oswl_strcmp(const char *cs, const char *ct)
{
	return rt_strcmp(cs, ct);
}

int oswl_strcasecmp(const char *a, const char *b)
{
	return rt_strcasecmp(a, b);
}

size_t oswl_strlen(const char *src)
{
	return rt_strlen(src);
}

size_t oswl_strnlen(const char *s, unsigned long maxlen)
{
	return rt_strnlen(s, maxlen);
}

int oswl_sprintf(char *buf, const char *format, ...)
{
    int n = 0;
    va_list arg_ptr;

    va_start(arg_ptr, format);
    n = rt_vsprintf(buf, format, arg_ptr);
    va_end(arg_ptr);

    return n;
}

int oswl_snprintf(char *buf, size_t size, const char *fmt, ...)
{
    int n = 0;
    va_list args;

    va_start(args, fmt);
    n = rt_vsnprintf(buf, size, fmt, args);
    va_end(args);

    return n;
}
RTM_EXPORT(oswl_snprintf);
RTM_EXPORT(oswl_strcpy);
RTM_EXPORT(oswl_strlcpy);
RTM_EXPORT(oswl_strcat);
RTM_EXPORT(oswl_strncat);
RTM_EXPORT(oswl_strlcat);
RTM_EXPORT(oswl_strncasecmp);
RTM_EXPORT(oswl_strchr);
RTM_EXPORT(oswl_strrchr);
RTM_EXPORT(oswl_strnstr);
RTM_EXPORT(oswl_strpbrk);
RTM_EXPORT(oswl_strsep);
RTM_EXPORT(oswl_strspn);
RTM_EXPORT(oswl_strcspn);
RTM_EXPORT(oswl_memchr);
RTM_EXPORT(oswl_strtoul);
// RTM_EXPORT(oswl_scnprintf);
// RTM_EXPORT(oswl_sscanf);
RTM_EXPORT(oswl_memcpy);
RTM_EXPORT(oswl_memcmp);
RTM_EXPORT(oswl_memset);
RTM_EXPORT(oswl_memmove);
RTM_EXPORT(oswl_strstr);
RTM_EXPORT(oswl_strncpy);
RTM_EXPORT(oswl_strncmp);
RTM_EXPORT(oswl_strcmp);
RTM_EXPORT(oswl_strcasecmp);
RTM_EXPORT(oswl_strlen);
RTM_EXPORT(oswl_strnlen);
RTM_EXPORT(oswl_sprintf);

#ifdef RT_USING_ULOG
#include <ulog.h>
void __ulog_e(const char *tag, const char *fmt, ...)
{
    va_list args;
    size_t length = 0;
    static char rt_log_buf[ULOG_LINE_BUF_SIZE];

    va_start(args, fmt);
    length = vsnprintf(rt_log_buf, sizeof(rt_log_buf) - 1, fmt, args);
    va_end(args);

    return ulog_e(tag, "%s", rt_log_buf);
}

void __ulog_w(const char *tag, const char *fmt, ...)
{
    va_list args;
    size_t length = 0;
    static char rt_log_buf[ULOG_LINE_BUF_SIZE];

    va_start(args, fmt);
    length = vsnprintf(rt_log_buf, sizeof(rt_log_buf) - 1, fmt, args);
    va_end(args);

    return ulog_w(tag, "%s", rt_log_buf);
}

void __ulog_i(const char *tag, const char *fmt, ...)
{
    va_list args;
    size_t length = 0;
    static char rt_log_buf[ULOG_LINE_BUF_SIZE];

    va_start(args, fmt);
    length = vsnprintf(rt_log_buf, sizeof(rt_log_buf) - 1, fmt, args);
    va_end(args);

    return ulog_i(tag, "%s", rt_log_buf);
}

void __ulog_d(const char *tag, const char *fmt, ...)
{
    va_list args;
    size_t length = 0;
    static char rt_log_buf[ULOG_LINE_BUF_SIZE];

    va_start(args, fmt);
    length = vsnprintf(rt_log_buf, sizeof(rt_log_buf) - 1, fmt, args);
    va_end(args);

    return ulog_d(tag, "%s", rt_log_buf);
}

#else
void __ulog_e(const char *tag, const char *fmt, ...)
{
    va_list args;
    size_t length = 0;
    static char rt_log_buf[256];

    va_start(args, fmt);
    length = vsnprintf(rt_log_buf, sizeof(rt_log_buf) - 1, fmt, args);
    va_end(args);

    return rt_kprintf("%s", rt_log_buf);
}

void __ulog_w(const char *tag, const char *fmt, ...)
{}

void __ulog_i(const char *tag, const char *fmt, ...)
{
    va_list args;
    size_t length = 0;
    static char rt_log_buf[256];

    va_start(args, fmt);
    length = vsnprintf(rt_log_buf, sizeof(rt_log_buf) - 1, fmt, args);
    va_end(args);

    return rt_kprintf("%s", rt_log_buf);
}

void __ulog_d(const char *tag, const char *fmt, ...)
{}
#endif

RTM_EXPORT(__ulog_e);
RTM_EXPORT(__ulog_w);
RTM_EXPORT(__ulog_i);
RTM_EXPORT(__ulog_d);
