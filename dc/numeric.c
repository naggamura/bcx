/* 
 * interface dc to the bc numeric routines
 *
 * Copyright (C) 1994, 1997, 1998, 2000, 2005, 2008, 2013, 2017
 * Free Software Foundation, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/* This should be the only module that knows the internals of type dc_num */
/* In this particular implementation we just slather out some glue and
 * make use of bc's numeric routines.
 */

/* make all the header files see that these are really the same thing;
 * this is acceptable because everywhere else dc_number is just referenced
 * as a pointer-to-incomplete-structure type
 */
#define dc_number bc_struct

#include "config.h"

#include <stdio.h>
#include <ctype.h>
#ifdef HAVE_LIMITS_H
# include <limits.h>
#endif
#ifndef UCHAR_MAX
# define UCHAR_MAX ((unsigned char)~0)
#endif
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif
#ifdef HAVE_ERRNO_H
# include <errno.h>
#else
  extern int errno;
#endif

#include "number.h"
#include "dc.h"
#include "dc-proto.h"

#ifdef __GNUC__
# if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__-0 >= 7) 
#  define ATTRIB(x) __attribute__(x)
# endif
#endif
#ifndef ATTRIB
# define ATTRIB(x)
#endif

/* Forward prototype */
static void out_char (int);

/* there is no POSIX standard for dc, so we'll take the GNU definitions */
int std_only = FALSE;

/* convert an opaque dc_num into a real bc_num */
/* by a freak accident, these are now no-op mappings,
 * but leave the notation here in case that changes later
 * */
#define CastNum(x)		(x)
#define CastNumPtr(x)	(x)

/* add two dc_nums, place into *result;
 * return DC_SUCCESS on success, DC_DOMAIN_ERROR on domain error
 */
int
dc_add DC_DECLARG((a, b, kscale, result))
	dc_num a DC_DECLSEP
	dc_num b DC_DECLSEP
	int kscale ATTRIB((unused)) DC_DECLSEP
	dc_num *result DC_DECLEND
{
	bc_init_num(CastNumPtr(result));
	bc_add(CastNum(a), CastNum(b), CastNumPtr(result), 0);
	return DC_SUCCESS;
}

/* subtract two dc_nums, place into *result;
 * return DC_SUCCESS on success, DC_DOMAIN_ERROR on domain error
 */
int
dc_sub DC_DECLARG((a, b, kscale, result))
	dc_num a DC_DECLSEP
	dc_num b DC_DECLSEP
	int kscale ATTRIB((unused)) DC_DECLSEP
	dc_num *result DC_DECLEND
{
	bc_init_num(CastNumPtr(result));
	bc_sub(CastNum(a), CastNum(b), CastNumPtr(result), 0);
	return DC_SUCCESS;
}

/* multiply two dc_nums, place into *result;
 * return DC_SUCCESS on success, DC_DOMAIN_ERROR on domain error
 */
int
dc_mul DC_DECLARG((a, b, kscale, result))
	dc_num a DC_DECLSEP
	dc_num b DC_DECLSEP
	int kscale DC_DECLSEP
	dc_num *result DC_DECLEND
{
	bc_init_num(CastNumPtr(result));
	bc_multiply(CastNum(a), CastNum(b), CastNumPtr(result), kscale);
	return DC_SUCCESS;
}

/* divide two dc_nums, place into *result;
 * return DC_SUCCESS on success, DC_DOMAIN_ERROR on domain error
 */
int
dc_div DC_DECLARG((a, b, kscale, result))
	dc_num a DC_DECLSEP
	dc_num b DC_DECLSEP
	int kscale DC_DECLSEP
	dc_num *result DC_DECLEND
{
	bc_init_num(CastNumPtr(result));
	if (bc_divide(CastNum(a), CastNum(b), CastNumPtr(result), kscale)){
		fprintf(stderr, "%s: divide by zero\n", progname);
		return DC_DOMAIN_ERROR;
	}
	return DC_SUCCESS;
}

/* divide two dc_nums, place quotient into *quotient and remainder
 * into *remainder;
 * return DC_SUCCESS on success, DC_DOMAIN_ERROR on domain error
 */
int
dc_divrem DC_DECLARG((a, b, kscale, quotient, remainder))
	dc_num a DC_DECLSEP
	dc_num b DC_DECLSEP
	int kscale DC_DECLSEP
	dc_num *quotient DC_DECLSEP
	dc_num *remainder DC_DECLEND
{
	bc_init_num(CastNumPtr(quotient));
	bc_init_num(CastNumPtr(remainder));
	if (bc_divmod(CastNum(a), CastNum(b),
						CastNumPtr(quotient), CastNumPtr(remainder), kscale)){
		fprintf(stderr, "%s: divide by zero\n", progname);
		return DC_DOMAIN_ERROR;
	}
	return DC_SUCCESS;
}

/* place the reminder of dividing a by b into *result;
 * return DC_SUCCESS on success, DC_DOMAIN_ERROR on domain error
 */
int
dc_rem DC_DECLARG((a, b, kscale, result))
	dc_num a DC_DECLSEP
	dc_num b DC_DECLSEP
	int kscale DC_DECLSEP
	dc_num *result DC_DECLEND
{
	bc_init_num(CastNumPtr(result));
	if (bc_modulo(CastNum(a), CastNum(b), CastNumPtr(result), kscale)){
		fprintf(stderr, "%s: remainder by zero\n", progname);
		return DC_DOMAIN_ERROR;
	}
	return DC_SUCCESS;
}

int
dc_modexp DC_DECLARG((base, expo, mod, kscale, result))
	dc_num base DC_DECLSEP
	dc_num expo DC_DECLSEP
	dc_num mod DC_DECLSEP
	int kscale DC_DECLSEP
	dc_num *result DC_DECLEND
{
	bc_init_num(CastNumPtr(result));
	if (bc_raisemod(CastNum(base), CastNum(expo), CastNum(mod),
					CastNumPtr(result), kscale)){
		if (bc_is_zero(CastNum(mod)))
			fprintf(stderr, "%s: remainder by zero\n", progname);
		return DC_DOMAIN_ERROR;
	}
	return DC_SUCCESS;
}

/* place the result of exponentiationg a by b into *result;
 * return DC_SUCCESS on success, DC_DOMAIN_ERROR on domain error
 */
int
dc_exp DC_DECLARG((a, b, kscale, result))
	dc_num a DC_DECLSEP
	dc_num b DC_DECLSEP
	int kscale DC_DECLSEP
	dc_num *result DC_DECLEND
{
	bc_init_num(CastNumPtr(result));
	bc_raise(CastNum(a), CastNum(b), CastNumPtr(result), kscale);
	return DC_SUCCESS;
}

/* take the square root of the value, place into *result;
 * return DC_SUCCESS on success, DC_DOMAIN_ERROR on domain error
 */
int
dc_sqrt DC_DECLARG((value, kscale, result))
	dc_num value DC_DECLSEP
	int kscale DC_DECLSEP
	dc_num *result DC_DECLEND
{
	bc_num tmp;

	tmp = bc_copy_num(CastNum(value));
	if (!bc_sqrt(&tmp, kscale)){
		fprintf(stderr, "%s: square root of negative number\n", progname);
		bc_free_num(&tmp);
		return DC_DOMAIN_ERROR;
	}
	*(CastNumPtr(result)) = tmp;
	return DC_SUCCESS;
}

/* compare dc_nums a and b;
 *  return a negative value if a < b;
 *  return a positive value if a > b;
 *  return zero value if a == b
 */
int
dc_compare DC_DECLARG((a, b))
	dc_num a DC_DECLSEP
	dc_num b DC_DECLEND
{
	return bc_compare(CastNum(a), CastNum(b));
}

/* attempt to convert a dc_num to its corresponding int value
 * If discard_p is DC_TOSS then deallocate the value after use.
 */
int
dc_num2int DC_DECLARG((value, discard_p))
	dc_num value DC_DECLSEP
	dc_discard discard_p DC_DECLEND
{
	long result;

	{
		len_t	t;
		bc_num2long(CastNum(value), &t);
		result = t;
	}
	if (result == 0 && !bc_is_zero(CastNum(value))) {
		fprintf(stderr, "%s: value overflows simple integer; punting...\n",
				progname);
		result = -1; /* more appropriate for dc's purposes */
	}
	if (discard_p == DC_TOSS)
		dc_free_num(&value);
	return (int)result;
}

/* convert a C integer value into a dc_num */
/* For convenience of the caller, package the dc_num
 * into a dc_data result.
 */
dc_data
dc_int2data DC_DECLARG((value))
	int value DC_DECLEND
{
	dc_data result;

	bc_init_num(CastNumPtr(&result.v.number));
	bc_int2num(CastNumPtr(&result.v.number), value);
	result.dc_type = DC_NUMBER;
	return result;
}

/* get a dc_num from some input stream;
 *  input is a function which knows how to read the desired input stream
 *  ibase is the input base (2<=ibase<=DC_IBASE_MAX)
 *  *readahead will be set to the readahead character consumed while
 *   looking for the end-of-number
 */
/* For convenience of the caller, package the dc_num
 * into a dc_data result.
 */
dc_data
dc_getnum DC_DECLARG((input, ibase, readahead))
	int (*input) DC_PROTO((void)) DC_DECLSEP
	int ibase DC_DECLSEP
	int *readahead DC_DECLEND
{
	bc_num	base;
	bc_num	result;
	bc_num	build;
	bc_num	tmp;
	bc_num	divisor;
	dc_data	full_result;
	int		negative = 0;
	int		digit;
	int		decimal;
	int		c;

	bc_init_num(&tmp);
	bc_init_num(&build);
	bc_init_num(&base);
	result = bc_copy_num(_zero_);
	bc_int2num(&base, ibase);
	c = (*input)();
	while (isspace(c))
		c = (*input)();
	if (c == '_' || c == '-'){
		negative = c;
		c = (*input)();
	}else if (c == '+'){
		c = (*input)();
	}
	while (isspace(c))
		c = (*input)();
	for (;;){
		if (isdigit(c))
			digit = c - '0';
		else if ('A' <= c && c <= 'F')
			digit = 10 + c - 'A';
		else
			break;
		c = (*input)();
		bc_int2num(&tmp, digit);
		bc_multiply(result, base, &result, 0);
		bc_add(result, tmp, &result, 0);
	}
	if (c == '.'){
		bc_free_num(&build);
		bc_free_num(&tmp);
		divisor = bc_copy_num(_one_);
		build = bc_copy_num(_zero_);
		decimal = 0;
		for (;;){
			c = (*input)();
			if (isdigit(c))
				digit = c - '0';
			else if ('A' <= c && c <= 'F')
				digit = 10 + c - 'A';
			else
				break;
			bc_int2num(&tmp, digit);
			bc_multiply(build, base, &build, 0);
			bc_add(build, tmp, &build, 0);
			bc_multiply(divisor, base, &divisor, 0);
			++decimal;
		}
		bc_divide(build, divisor, &build, decimal);
		bc_add(result, build, &result, 0);
	}
	/* Final work. */
	if (negative)
		bc_sub(_zero_, result, &result, 0);

	bc_free_num(&tmp);
	bc_free_num(&build);
	bc_free_num(&base);
	if (readahead)
		*readahead = c;
	*CastNumPtr(&full_result.v.number) = result;
	full_result.dc_type = DC_NUMBER;
	return full_result;
}


/* Return the "length" of the number, ignoring *all* leading zeros,
 * (including those to the right of the radix point!)
 */
int
dc_numlen DC_DECLARG((value))
	dc_num value DC_DECLEND
{
	/* XXX warning: unholy coziness with the internals of a bc_num! */
	bc_num num = CastNum(value);
	len_t	rscale = (num->n_dec_scale+LOG10_RADIX-1)/LOG10_RADIX;
	//len_t	rscale = TO_SCALE(num->n_dec_scale);
	return bi_b10_len(num->n_value + (num->n_scale - rscale), rscale + num->n_len);
}

/* return the scale factor of the passed dc_num
 * If discard_p is DC_TOSS then deallocate the value after use.
 */
int
dc_tell_scale DC_DECLARG((value, discard_p))
	dc_num value DC_DECLSEP
	dc_discard discard_p DC_DECLEND
{
	int kscale;

	kscale = CastNum(value)->n_scale;
	if (discard_p == DC_TOSS)
		dc_free_num(&value);
	return kscale;
}


/* initialize the math subsystem */
void
dc_math_init DC_DECLVOID()
{
	bc_init_numbers();
}

/* print out a dc_num in output base obase to stdout;
 * if discard_p is DC_TOSS then deallocate the value after use
 */
void
dc_out_num DC_DECLARG((value, obase, discard_p))
	dc_num value DC_DECLSEP
	int obase DC_DECLSEP
	dc_discard discard_p DC_DECLEND
{
	out_char('\0'); /* clear the column counter */
	bc_out_num(CastNum(value), obase, out_char, 0);
	if (discard_p == DC_TOSS)
		dc_free_num(&value);
}

/* dump out the absolute value of the integer part of a
 * dc_num as a byte stream, without any line wrapping;
 * if discard_p is DC_TOSS then deallocate the value after use
 */
void
dc_dump_num DC_DECLARG((dcvalue, discard_p))
	dc_num dcvalue DC_DECLSEP
	dc_discard discard_p DC_DECLEND
{
	struct digit_stack { int digit; struct digit_stack *link;};
	struct digit_stack *top_of_stack = NULL;
	struct digit_stack *cur;
	struct digit_stack *next;
	bc_num value;
	bc_num obase;
	bc_num digit;

	bc_init_num(&value);
	bc_init_num(&obase);
	bc_init_num(&digit);

	/* we only handle the integer portion: */
	bc_divide(CastNum(dcvalue), _one_, &value, 0);
	/* we only handle the absolute value: */
	if(value->n_sign < 0)
	{
		value->n_sign = 1;
	}
	/* we're done with the dcvalue parameter: */
	if (discard_p == DC_TOSS)
		dc_free_num(&dcvalue);

	bc_int2num(&obase, 1+UCHAR_MAX);
	do {
		(void) bc_divmod(value, obase, &value, &digit, 0);
		cur = dc_malloc(sizeof *cur);
		{
			len_t	t;
			bc_num2long(digit, &t);
			cur->digit = t;
		}
		cur->link = top_of_stack;
		top_of_stack = cur;
	} while (!bc_is_zero(value));

	for (cur=top_of_stack; cur; cur=next) {
		putchar(cur->digit);
		next = cur->link;
		free(cur);
	}

	bc_free_num(&digit);
	bc_free_num(&obase);
	bc_free_num(&value);
}

/* deallocate an instance of a dc_num */
void
dc_free_num DC_DECLARG((value))
	dc_num *value DC_DECLEND
{
	bc_free_num(CastNumPtr(value));
}

/* return a duplicate of the number in the passed value */
/* The mismatched data types forces the caller to deal with
 * bad dc_type'd dc_data values, and makes it more convenient
 * for the caller to not have to do the grunge work of setting
 * up a dc_type result.
 */
dc_data
dc_dup_num DC_DECLARG((value))
	dc_num value DC_DECLEND
{
	dc_data result;

	++CastNum(value)->n_refs;
	result.v.number = value;
	result.dc_type = DC_NUMBER;
	return result;
}



/*---------------------------------------------------------------------------\
| The rest of this file consists of stubs for bc routines called by numeric.c|
| so as to minimize the amount of bc code needed to build dc.                |
| The bulk of the code was just lifted straight out of the bc source.        |
\---------------------------------------------------------------------------*/

#ifdef HAVE_STDARG_H
# include <stdarg.h>
#else
# include <varargs.h>
#endif

#ifndef HAVE_STRTOL
/* Maintain some of the error checking of a real strtol() on
 * ancient systems that lack one, but punting on the niceties
 * of supporting bases other than 10 and overflow checking.
 */
long
strtol(const char *s, char **end, int base)
{
	int sign = 1;
	long result = 0;

	for (;; ++s) {
		if (*s == '-')
			sign = -sign;
		else if (*s != '+' && !isspace(*(const unsigned char *)s))
			break;
	}
	while (isdigit(*(const unsigned char *)s))
		result = 10*result + (*s++ - '0');
	*end = s;
	return result * sign;
}
#endif /*!HAVE_STRTOL*/


static int out_col = 0;
static int line_max = -1;	/* negative means "need to check environment" */
#define DEFAULT_LINE_MAX 70

static void
set_line_max_from_environment(void)
{
	const char *env_line_len = getenv("DC_LINE_LENGTH");
	line_max = DEFAULT_LINE_MAX;
	errno = 0;
	if (env_line_len) {
		char *endptr;
		long proposed_line_len = strtol(env_line_len, &endptr, 0);
		line_max = (int)proposed_line_len;

		/* silently enforce sanity */
		while (isspace(*(const unsigned char *)endptr))
			++endptr;
		if (*endptr || errno || line_max != proposed_line_len
					|| line_max < 0 || line_max == 1)
			line_max = DEFAULT_LINE_MAX;
	}
}

/* Output routines: Write a character CH to the standard output.
   It keeps track of the number of characters output and may
   break the output with a "\<cr>". */

static void
out_char (int ch)
{
	if (ch == '\0') {
		out_col = 0;
	} else {
		if (line_max < 0)
			set_line_max_from_environment();
		if (++out_col >= line_max && line_max != 0) {
			putchar ('\\');
			putchar ('\n');
			out_col = 1;
		}
		putchar(ch);
	}
}

/* Malloc could not get enough memory. */

void
out_of_memory()
{
	dc_memfail();
}

/* Runtime error --- will print a message and stop the machine. */

#ifdef HAVE_STDARG_H
#ifdef __STDC__
void
rt_error (char *mesg, ...)
#else
void
rt_error (mesg)
	char *mesg;
#endif
#else
void
rt_error (mesg, va_alist)
	char *mesg;
#endif
{
	va_list args;

	fprintf (stderr, "Runtime error: ");
#ifdef HAVE_STDARG_H
	va_start (args, mesg);
#else
	va_start (args);
#endif
	vfprintf (stderr, mesg, args);
	va_end (args);
	fprintf (stderr, "\n");
}


/* A runtime warning tells of some action taken by the processor that
   may change the program execution but was not enough of a problem
   to stop the execution. */

#ifdef HAVE_STDARG_H
#ifdef __STDC__
void
rt_warn (char *mesg, ...)
#else
void
rt_warn (mesg)
	char *mesg;
#endif
#else
void
rt_warn (mesg, va_alist)
	char *mesg;
#endif
{
	va_list args;

	fprintf (stderr, "Runtime warning: ");
#ifdef HAVE_STDARG_H
	va_start (args, mesg);
#else
	va_start (args);
#endif
	vfprintf (stderr, mesg, args);
	va_end (args);
	fprintf (stderr, "\n");
}


/*
 * Local Variables:
 * mode: C
 * tab-width: 4
 * End:
 * vi: set ts=4 :
 */
