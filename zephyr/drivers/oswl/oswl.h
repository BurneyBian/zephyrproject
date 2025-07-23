#ifndef SA_OSWL_H
#define SA_OSWL_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define NAME_LENGTH 24
#define OSWL_VM_DEBUG

typedef struct dmalloc_id{
	int module_id;
}dmalloc_id_t;

#define DECLEAR_DMALLOC_DEBUG(MODULE_ID,FILE_NAME)\
	static dmalloc_id_t  __attribute__((unused)) dmalloc_##FILE_NAME={.module_id=MODULE_ID}; 

extern void oswl_udelay(unsigned int usecs);
extern void oswl_mdelay(unsigned int msecs);

extern void *oswl_malloc(unsigned long size);
extern void oswl_free(const void *addr);
extern void *oswl_calloc(int count, unsigned long size);

extern void *oswl_malloc_debug(int module, unsigned long size);
extern void *oswl_malloc_debug_ext(int module, char *mod_name, unsigned long size);

extern void oswl_free_debug(int module, const void *addr);
extern void *oswl_calloc_debug(int module, int count, unsigned long size);

//#define MUTEX_STATIC_DEFINE

#ifdef MUTEX_STATIC_DEFINE
#include <pthread.h>
typedef pthread_mutex_t oswl_mutex_t;
#define _OSWL_MUTEX_INIT(name) const oswl_mutex_t name = PTHREAD_MUTEX_INITIALIZER
#else
typedef struct oswl_mutex {
    void *mutex;
} oswl_mutex_t;
#endif

extern int oswl_mutex_init(oswl_mutex_t *mutex);
extern int oswl_mutex_lock(oswl_mutex_t *mutex);
extern void oswl_mutex_unlock(oswl_mutex_t *mutex);
extern void oswl_mutex_destory(oswl_mutex_t *mutex);

// semaphore api
typedef struct oswl_sem {
    void *sem;
} oswl_sem_t;
extern int oswl_sem_init(oswl_sem_t *sem, int val);
extern int oswl_sem_wait(oswl_sem_t *sem, oswl_mutex_t *mutex);
extern void oswl_sem_signal(oswl_sem_t *sem);
extern void oswl_sem_destory(oswl_sem_t *sem);
extern int oswl_sem_wait_timeout(oswl_sem_t *sem, oswl_mutex_t *mutex, int ms);
// task api
typedef struct oswl_thread {
	void 	*thread;
	const char *name;
	void 	(*callback)(void*);
	void	*params;
	int		stack_size;
	int		priority;
	int		tick;
} oswl_thread_t;

#define OSWL_THREAD_PRIORITY_MAX (RT_THREAD_PRIORITY_MAX - 1)
#define OSWL_THREAD_PRIORITY_MIN 1

#if RT_THREAD_PRIORITY_MAX > 16
#define OSWL_THREAD_PRIORITY_1 2
#define OSWL_THREAD_PRIORITY_2 3
#define OSWL_THREAD_PRIORITY_3 6
#define OSWL_THREAD_PRIORITY_4 10
#define OSWL_THREAD_PRIORITY_5 16
#endif

extern int oswl_thread_init(oswl_thread_t *thread);
extern void oswl_kthread_destory(oswl_thread_t *thread);
#ifdef BSP_USING_DMAC
int oswl_dmacpy_init(unsigned long dev);
void oswl_dmacpy_deinit(int fd);
int oswl_dmacpy_start(int fd, unsigned long sar, unsigned long dar, unsigned int len);
#endif
char *rtt_get_optarg(void);
void rtt_reset_optind(void);
char oswl_toupper(char *str);
int oswl_isdigit(int ch);

#define oswl_getchar finsh_getchar

#define _LOG_TAG_STR(_name) (#_name)
#define LOG_TAG_STR(_name) _LOG_TAG_STR(_name)
#define ULOG_DECLARE_TAG(_t)
#define ULOG_USE_TAG(_t)

void __ulog_e(const char *tag, const char *fmt, ...);
void __ulog_w(const char *tag, const char *fmt, ...);
void __ulog_i(const char *tag, const char *fmt, ...);
void __ulog_d(const char *tag, const char *fmt, ...);

#define OSWL_LOG_E(fmt, ...)		__ulog_e(LOG_TAG_STR(LOG_TAG), "\033[1;31m"fmt"\033[0m", ##__VA_ARGS__)
#define OSWL_LOG_W(...)				__ulog_w(LOG_TAG_STR(LOG_TAG), __VA_ARGS__)
#define OSWL_LOG_I(...)				__ulog_i(LOG_TAG_STR(LOG_TAG), __VA_ARGS__)
#define OSWL_LOG_D(...)				__ulog_d(LOG_TAG_STR(LOG_TAG), __VA_ARGS__)

#define OSWL_TLOG_E(_t, ...)      	__ulog_e(LOG_TAG_STR(_t), __VA_ARGS__)
#define OSWL_TLOG_W(_t, ...)      	__ulog_w(LOG_TAG_STR(_t), __VA_ARGS__)
#define OSWL_TLOG_I(_t, ...)      	__ulog_i(LOG_TAG_STR(_t), __VA_ARGS__)
#define OSWL_TLOG_D(_t, ...)      	__ulog_d(LOG_TAG_STR(_t), __VA_ARGS__)

// TODO: Fixme
#define OSWL_NTLOG_W(...)      	__ulog_w(LOG_TAG_STR(LOG_TAG), __VA_ARGS__)
#define OSWL_NTLOG_I(...)      	__ulog_i(LOG_TAG_STR(LOG_TAG), __VA_ARGS__)
#define OSWL_NTLOG_D(...)      	__ulog_d(LOG_TAG_STR(LOG_TAG), __VA_ARGS__)
#define OSWL_NTLOG_E(...)      	__ulog_e(LOG_TAG_STR(LOG_TAG), __VA_ARGS__)


typedef struct oswl_pthread_cond
{
	void *data;
}oswl_pthread_cond_t;

struct oswl_timespec
{
	long tv_sec;
	long tv_nsec;
};

int oswl_pthread_cond_init(oswl_pthread_cond_t *p_cond);
int oswl_pthread_cond_wait(oswl_pthread_cond_t *p_cond, oswl_mutex_t *mutex);
int oswl_pthread_cond_timedwait(oswl_pthread_cond_t *p_cond, oswl_mutex_t *mutex,
	struct oswl_timespec *abstime);
int oswl_pthread_cond_signal(oswl_pthread_cond_t *p_cond);
int oswl_pthread_cond_destroy(oswl_pthread_cond_t *p_cond);
int oswl_pthread_cond_broadcast(oswl_pthread_cond_t *p_cond);

void *oswl_mmap(unsigned long phy_addr, unsigned long size, int *v);
void *oswl_mmap_cache(unsigned long phy_addr, unsigned long size, int *v);
void oswl_munmap(unsigned long virt_addr, unsigned long phy_addr, unsigned long size, int v);


#define OSWL_O_ACCMODE     00000003
#define OSWL_O_RDONLY      00000000
#define OSWL_O_WRONLY      00000001
#define OSWL_O_RDWR        00000002
#define OSWL_O_CREAT       00000100

void *oswl_open(const char *filename, int flags, int mode);
void oswl_close(void *filp);
int oswl_write(const char *buf, int len, void *filp);
int oswl_read(char *buf, unsigned int len, void *filp);
int oswl_ioctl(void *args, unsigned int cmd, void *filp);

char *oswl_strcpy(char *dest, const char *src);
int oswl_strlcpy(char *dest, const char *src, int size);
char *oswl_strcat(char *dest, const char *src);
char *oswl_strncat(char *dest, const char *src, int count);
int oswl_strlcat(char *dest, const char *src, int count);
int oswl_strncasecmp(const char *s1, const char *s2, int n);
char *oswl_strchr(const char *s, int c);
char *oswl_strrchr(const char *s, int c);
char *oswl_strnstr(const char *s1, const char *s2, int len);
char *oswl_strpbrk(const char *cs, const char *ct);
char *oswl_strsep(char **s, const char *ct);
int oswl_strspn(const char *s, const char *accept);
int oswl_strcspn(const char *s, const char *reject);
void *oswl_memchr(const void *s, int c, int n);
unsigned long oswl_strtoul(const char *cp, char **endp, unsigned int base);
int oswl_vsnprintf(char *str, int size, const char *fmt, va_list args);
void *oswl_memcpy(void *dst, const void *src, unsigned long count);
int oswl_memcmp(const void *cs, const void *ct, size_t count);
void *oswl_memset(void *src, int c, unsigned long n);
void *oswl_memmove(void *dest, const void *src, unsigned long n);
char *oswl_strstr(const char *str1, const char *str2);
char *oswl_strncpy(char *dest, const char *src, size_t n);
int oswl_strncmp(const char *cs, const char *ct, size_t count);
int oswl_strcmp(const char *cs, const char *ct);
int oswl_strcasecmp(const char *a, const char *b);
size_t oswl_strlen(const char *src);
size_t oswl_strnlen(const char *s, unsigned long maxlen);
int oswl_sprintf(char *buf, const char *format, ...);
int oswl_snprintf(char *buf, size_t size, const char *fmt, ...);


#endif
