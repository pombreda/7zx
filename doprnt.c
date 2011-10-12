#ifdef __DJGPP__
// ***
//     this file is hacked to only support %s and %d for 7ZDEC
//     (although unused in current build), saves 2k in the compressed .EXE,
//     so throw it in the same dir as 7Z*.[ch] and MAKE-DJ.BAT !!
//     P.S. "-O -mpentiumpro" is smallest size for GCC 2.95.3
// ***

/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <sys/types.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <libc/file.h>
#include <libc/local.h>
#include <libc/ieee.h>

static char decimal = '.';

/* 11-bit exponent (VAX G floating point) is 308 decimal digits */
#define	MAXEXP		308
#define MAXEXPLD        4952 /* this includes subnormal numbers */
/* 128 bit fraction takes up 39 decimal digits; max reasonable precision */
#define	MAXFRACT	39

#define	DEFPREC		6
#define	DEFLPREC	6

#define	BUF		(MAXEXPLD+MAXFRACT+1)	/* + decimal point */

#define	PUTC(ch)	(void) putc(ch, fp)

#define ARG(basetype) _ulonglong = \
		flags&LONGDBL ? va_arg(argp, long long basetype) : \
		flags&LONGINT ? va_arg(argp, long basetype) : \
		flags&SHORTINT ? (short basetype)va_arg(argp, int) : \
		flags&CHARINT ? (char basetype)va_arg(argp, int) : \
		(basetype)va_arg(argp, int)

static int nan_p = 0;

static __inline__ int todigit(char c)
{
  if (c<='0') return 0;
  if (c>='9') return 9;
  return c-'0';
}
static __inline__ char tochar(int n)
{
  if (n>=9) return '9';
  if (n<=0) return '0';
  return n+'0';
}

/* have to deal with the negative buffer count kludge */

#define	LONGINT		0x01		/* long integer */
#define	LONGDBL		0x02		/* long double */
#define	SHORTINT	0x04		/* short integer */
#define	CHARINT		0x08		/* char */
#define	ALT		0x10		/* alternate form */
#define	LADJUST		0x20		/* left adjustment */
#define	ZEROPAD		0x40		/* zero (as opposed to blank) pad */
#define	HEXPREFIX	0x80		/* add 0x or 0X prefix */

#ifdef __GO32__
static int isspeciall(long double d, char *bufp);
#endif

static char NULL_REP[] = "(null)";
static char UNNORMAL_REP[] = "Unnormal";

int
_doprnt(const char *fmt0, va_list argp, FILE *fp)
{
  const char *fmt;		/* format string */
  int ch;			/* character from fmt */
  int cnt;			/* return value accumulator */
  int n;			/* random handy integer */
  char *t;			/* buffer pointer */
  long double _ldouble;		/* double and long double precision arguments
				   %L.[eEfgG] */
  unsigned long long _ulonglong=0; /* integer arguments %[diouxX] */
  int base;			/* base for [diouxX] conversion */
  int dprec;			/* decimal precision in [diouxX] */
  int fieldsz;			/* field size expanded by sign, etc */
  int flags;			/* flags as above */
  int fpprec;			/* `extra' floating precision in [eEfgG] */
  int prec;			/* precision from format (%.3d), or -1 */
  int realsz;			/* field size expanded by decimal precision */
  int size;			/* size of converted field or string */
  int width;			/* width from format (%8d), or 0 */
  char sign;			/* sign prefix (' ', '+', '-', or \0) */
  char softsign;		/* temporary negative sign for floats */
  const char *digs;		/* digits for [diouxX] conversion */
  char buf[BUF];		/* space for %c, %[diouxX], %[eEfgG] */
  int neg_ldouble = 0;		/* non-zero if _ldouble is negative */

  decimal = localeconv()->decimal_point[0];

  if (fp->_flag & _IORW)
  {
    fp->_flag |= _IOWRT;
    fp->_flag &= ~(_IOEOF|_IOREAD);
  }
  if ((fp->_flag & _IOWRT) == 0)
    return (EOF);

  fmt = fmt0;
  digs = "0123456789abcdef";
  for (cnt = 0;; ++fmt)
  {
    while ((ch = *fmt) && ch != '%')
    {
      PUTC (ch);
      fmt++;
      cnt++;
    }
    if (!ch)
      return cnt;
    flags = 0; dprec = 0; fpprec = 0; width = 0;
    prec = -1;
    sign = '\0';
  rflag:
    switch (*++fmt)
    {
    case ' ':
      /*
       * ``If the space and + flags both appear, the space
       * flag will be ignored.''
       *	-- ANSI X3J11
       */
      if (!sign)
	sign = ' ';
      goto rflag;
    case '-':
      flags |= LADJUST;
      goto rflag;
    case '0':
      /*
       * ``Note that 0 is taken as a flag, not as the
       * beginning of a field width.''
       *	-- ANSI X3J11
       */
      flags |= ZEROPAD;
      goto rflag;
    case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      n = 0;
      do {
	n = 10 * n + todigit(*fmt);
      } while (isascii((unsigned char)*++fmt) && isdigit((unsigned char)*fmt));
      width = n;
      --fmt;
      goto rflag;
    case 'D':
      flags |= LONGINT;
      /*FALLTHROUGH*/
    case 'd':
    case 'i':
      ARG(signed);
      if ((long long)_ulonglong < 0)
      {
        _ulonglong = -_ulonglong;
	sign = '-';
      }
      base = 10;
      goto number;
    case 's':
      if (!(t = va_arg(argp, char *)))
	t = NULL_REP;
      if (prec >= 0)
      {
	/*
	 * can't use strlen; can only look for the
	 * NUL in the first `prec' characters, and
	 * strlen() will go further.
	 */
	char *p			/*, *memchr() */;

	if ((p = memchr(t, 0, (size_t)prec)))
	{
	  size = p - t;
	  if (size > prec)
	    size = prec;
	}
	else
	  size = prec;
      }
      else
	size = strlen(t);
      sign = '\0';
      goto pforw;
    number:
      /*
       * ``... diouXx conversions ... if a precision is
       * specified, the 0 flag will be ignored.''
       *	-- ANSI X3J11
       */
      if ((dprec = prec) >= 0)
	flags &= ~ZEROPAD;

      /*
       * ``The result of converting a zero value with an
       * explicit precision of zero is no characters.''
       *	-- ANSI X3J11
       */
      t = buf + BUF;

      if (_ulonglong != 0 || prec != 0)
      {
        /* conversion is done separately since operations
	  with long long are much slower */
#define CONVERT(type) \
	{ \
	  register type _n = (type)_ulonglong; \
	  do { \
	    *--t = digs[_n % base]; \
	    _n /= base; \
	  } while (_n); \
	}
	if (flags&LONGDBL)
	  CONVERT(unsigned long long) /* no ; */
	else
	  CONVERT(unsigned long) /* no ; */
#undef CONVERT
      }

      digs = "0123456789abcdef";
      size = buf + BUF - t;

    pforw:
      /*
       * All reasonable formats wind up here.  At this point,
       * `t' points to a string which (if not flags&LADJUST)
       * should be padded out to `width' places.  If
       * flags&ZEROPAD, it should first be prefixed by any
       * sign or other prefix; otherwise, it should be blank
       * padded before the prefix is emitted.  After any
       * left-hand padding and prefixing, emit zeroes
       * required by a decimal [diouxX] precision, then print
       * the string proper, then emit zeroes required by any
       * leftover floating precision; finally, if LADJUST,
       * pad with blanks.
       */

      /*
       * compute actual size, so we know how much to pad
       * fieldsz excludes decimal prec; realsz includes it
       */
      fieldsz = size + fpprec;
      realsz = dprec > fieldsz ? dprec : fieldsz;

      /* right-adjusting blank padding */
      if ((flags & (LADJUST|ZEROPAD)) == 0 && width)
	for (n = realsz; n < width; n++)
	  PUTC(' ');
      /* prefix */
      /* right-adjusting zero padding */
      if ((flags & (LADJUST|ZEROPAD)) == ZEROPAD)
	for (n = realsz; n < width; n++)
	  PUTC('0');
      /* leading zeroes from decimal precision */
      for (n = fieldsz; n < dprec; n++)
	PUTC('0');

      /* the string or number proper */
      for (n = size; n > 0; n--)
        PUTC(*t++);
      /* left-adjusting padding (always blank) */
      if (flags & LADJUST)
	for (n = realsz; n < width; n++)
	  PUTC(' ');
      /* finally, adjust cnt */
      cnt += width > realsz ? width : realsz;
      break;
    case '\0':			/* "%?" prints ?, unless ? is NULL */
      return cnt;
    default:
      PUTC((char)*fmt);
      cnt++;
    }
  }
  /* NOTREACHED */
}

#define MAXP 4096
#define NP   12
#define P    (4294967296.0L * 4294967296.0L * 2.0L)   /* 2^65 */
static long double INVPREC = P;
static long double PREC = 1.0L/P;
#undef P
/*
 * Defining FAST_LDOUBLE_CONVERSION results in a little bit faster
 * version, which might be less accurate (about 1 bit) for long
 * double. For 'normal' double it doesn't matter.
 */
/* #define FAST_LDOUBLE_CONVERSION */

static int
isspeciall(long double d, char *bufp)
{
  typedef struct {
    unsigned manl:32;
    unsigned manh:32;
    unsigned exp:15;
    unsigned sign:1;
  } IEEExp;

  typedef union {
    IEEExp ip;
    long double ldouble;  /* double and long double precision arguments */
  } ip_union;

  ip_union ip;

  ip.ldouble = d;

  nan_p = 0;  /* don't assume the static is 0 (emacs) */

  /* Unnormals: the MSB of mantissa is non-zero, but the exponent is
     not zero either.  */
  if ((ip.ip.manh & 0x80000000U) == 0 && ip.ip.exp != 0)
  {
    if (ip.ip.sign)
      *bufp++ = '-';
    strcpy(bufp, UNNORMAL_REP);
    return strlen(bufp) + ip.ip.sign;
  }
  if (ip.ip.exp != 0x7fff)
    return(0);
  if ((ip.ip.manh & 0x7fffffff) || ip.ip.manl)
  {
    strcpy(bufp, "NaN");
    nan_p = ip.ip.sign ? -1 : 1; /* kludge: we don't need the sign, it's
				    not nice, but it should work */
  }
  else
    (void)strcpy(bufp, "Inf");
  return(3);
}
#endif
