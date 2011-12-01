/** @file util.h
 *  @brief Generally useful functions from libc.
 */

/* Copyright (c) 2007,2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#ifndef __NXOS_BASE_UTIL_H__
#define __NXOS_BASE_UTIL_H__

#include "base/types.h"

/** @addtogroup typesAndUtils */
/*@{*/

/** Return the numeric minimum of two parameters.
 *
 * @warning This macro will cause the arguments to be evaluated several
 *          times! Use only with pure arguments.
 */
#define MIN(x, y) ((x) < (y) ? (x): (y))

/** Return the numeric maximum of two parameters.
 *
 * @warning This macro will cause the arguments to be evaluated several
 *          times! Use only with pure arguments.
 */
#define MAX(x, y) ((x) > (y) ? (x): (y))

#define M_PI 3.14159265358979323846
#define cordic_1K 0x26DD3B6A
#define CORDIC_NTAB 32
#define MUL 1073741824.000000

float sinf(float a);

float cosf(float a); 

double pow(double a, double b);

int sqrti(int a);

/** Copy @a len bytes from @a src to @a dest.
 *
 * @param dest Destination of the copy.
 * @param src Source of the copy.
 * @param len Number of bytes to copy.
 *
 * @warning The source and destination memory regions must not overlap.
 */
void memcpy(void *dest, const void *src, U32 len);

/** Initialize @a len bytes of @a dest with the constant @a val.
 *
 * @param dest Start of the region to initialize.
 * @param val Constant initializer to use.
 * @param len Length of the region.
 */
void memset(void *dest, const U8 val, U32 len);

/** Return the length of the given null-terminated string.
 *
 * @param str The string to evaluate.
 * @return The length in bytes of the string.
 */
U32 strlen(const char *str);

/** Compare two string prefixes for equality.
 *
 * @param a First string to compare.
 * @param b Second string to compare.
 * @param n Number of bytes to compare.
 * @return TRUE if the first @a n bytes of @a a are equal to @a b,
 * FALSE otherwise.
 *
 * @note This function will halt on the first NULL byte it finds in
 * either string.
 */
bool streqn(const char *a, const char *b, U32 n);

/** compare two strings for equality.
 *
 * This is equivalent to @a strneq, with the maximum length being the
 * length of the shortest input string.
 *
 * @see strneq
 */
bool streq(const char *a, const char *b);

/** Locate leftmost instance of character @a c in string @a s.
 *
 * @param s The string to search.
 * @param c The character to find.
 * @return A pointer to the first occurence of @a c in @a s, or NULL if
 * there is none.
 */
char *strchr(const char *s, const char c);

/** Locate rightmost instance of character @a c in string @a s.
 *
 * @param s The string to search.
 * @param c The character to find.
 * @return A pointer to the last occurence of @a c in @a s, or NULL if
 * there is none.
 */
char *strrchr(const char *s, const char c);

/** Convert a string to the unsigned integer it represents, if possible.
 *
 * @param s The string to convert.
 * @param result A pointer to the integer that will contain the parsed
 * result, if the conversion was successful.
 * @return TRUE with *result set correctly if the conversion was
 * successful, FALSE if the conversion failed.
 *
 * @note If the conversion fails, the value of @a *result will still
 * be clobbered, but won't contain the true value.
 */
bool atou32(const char *s, U32* result);

/** Convert a string to the signed integer it represents, if possible.
 *
 * @param s The string to convert.
 * @param result A pointer to the integer that will contain the parsed
 * result, if the conversion was successful.
 * @return TRUE with *result set correctly if the conversion was
 * successful, FALSE if the conversion failed.
 *
 * @note If the conversion fails, the value of @a *result will still
 * be clobbered, but won't contain the true value.
 */
bool atos32(const char *s, S32* result);

/*@}*/

#endif /* __NXOS_BASE_UTIL_H__ */
