#include "osal.h"

#define do_div_u64(n, base)              \
({                                      \
    unsigned int _base = (base), _rem;   \
    _rem = ((unsigned long long)(n)) % _base;  \
    (n) = ((unsigned long long)(n)) / _base;   \
    if (_rem > _base / 2)               \
        ++(n);                          \
    _rem;                               \
})

/* the result of u64/u32. */
unsigned long long osal_div_u64(unsigned long long dividend, unsigned int divisor)
{
	unsigned long long ret = dividend;
	do_div_u64(ret, divisor);
    return ret;
}

/* the remainder of u64/u32. */
unsigned long long osal_div_u64_rem(unsigned long long dividend, unsigned int divisor)
{
	return do_div_u64(dividend, divisor);
}


#define do_div_s64(n, base)              \
({                                      \
    int _base = (base), _rem;   \
    _rem = ((long long)(n)) % _base;  \
    (n) = ((long long)(n)) / _base;   \
    if (_rem > _base / 2)               \
        ++(n);                          \
    _rem;                               \
})

/* the result of s64/s32. */
long long osal_div_s64(long long dividend, int divisor)
{
	long long ret = dividend;
	do_div_s64(ret, divisor);
    return ret;
}

/* the remainder of s64/s32. */
long long osal_div_s64_rem(long long dividend, int divisor)
{
    return do_div_s64(dividend, divisor);
}

#define do_div64_u64(n, base)              \
({                                      \
    unsigned long long _base = (base), _rem;   \
    _rem = ((unsigned long long)(n)) % _base;  \
    (n) = ((unsigned long long)(n)) / _base;   \
    if (_rem > _base / 2)               \
        ++(n);                          \
    _rem;                               \
})
/* the result of u64/u64. */
unsigned long long osal_div64_u64(unsigned long long dividend, unsigned long long divisor)
{
	unsigned long long ret = dividend;
	do_div64_u64(ret, divisor);
    return ret;
}

/* the remainder of u64/u64. */
unsigned long long osal_div64_u64_rem(unsigned long long dividend, unsigned long long divisor)
{
    return do_div64_u64(dividend, divisor);
}

#define do_div64_s64(n, base)              \
({                                      \
    long long _base = (base), _rem;   \
    _rem = ((long long)(n)) % _base;  \
    (n) = ((long long)(n)) / _base;   \
    if (_rem > _base / 2)               \
        ++(n);                          \
    _rem;                               \
})
/* the result of s64/s64. */
long long osal_div64_s64(long long dividend, long long divisor)
{
	long long ret = dividend;
	do_div64_s64(ret, divisor);
    return ret;
}

long long osal_div64_s64_rem(long long dividend, long long divisor)
{
    return do_div64_s64(dividend, divisor);
}
