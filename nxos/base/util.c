/* Copyright (c) 2007,2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#include "base/types.h"
#include "base/util.h"
#include "base/assert.h"

int cordic_ctab [] = {0x3243F6A8, 0x1DAC6705, 0x0FADBAFC, 0x07F56EA6, 0x03FEAB76, 0x01FFD55B,
0x00FFFAAA, 0x007FFF55, 0x003FFFEA, 0x001FFFFD, 0x000FFFFF, 0x0007FFFF, 0x0003FFFF,
0x0001FFFF, 0x0000FFFF, 0x00007FFF, 0x00003FFF, 0x00001FFF, 0x00000FFF, 0x000007FF,
0x000003FF, 0x000001FF, 0x000000FF, 0x0000007F, 0x0000003F, 0x0000001F, 0x0000000F,
0x00000008, 0x00000004, 0x00000002, 0x00000001, 0x00000000, };

double power_of(double a, double b) {
  double result = 1;

  if(b > 0) {
    result = power_of(a, (b - 1)) ;
    result *= a;
  }

  return result;

}

int square_root(int a_nInput) {
  int op  = a_nInput;
  int res = 0;
  int one = 1uL << 30;

  while (one > op)
    one >>= 2;
    
  while (one != 0) {
    if (op >= res + one) {
      op = op - (res + one);
      res = res +  2 * one;
    }
    res >>= 1;
    one >>= 2;
  }

  return res;
}

float sine(float angle) {
  int i = 0;
  int minus = 0;
  int theta = 0;
  int n = 32;
  int k, d, tx, ty, tz;
  int x=cordic_1K,y=0,z=0;

  if(angle < 0) {
    angle = -angle;
    minus = 1;
  }

  while(angle > (M_PI*2))
    angle -= (M_PI * 2);

  for(angle = angle; angle > (M_PI); angle -= (M_PI))
    i++;

  if(angle > M_PI/2) {
    angle -= (M_PI/2);
    y = cosine(angle) * MUL;
  }

  else {
    theta = angle * MUL;
    z=theta;

    n = (n>CORDIC_NTAB) ? CORDIC_NTAB : n;

    for (k=0; k<n; ++k) {
      d = z>>31;
      tx = x - (((y>>k) ^ d) - d);
      ty = y + (((x>>k) ^ d) - d);
      tz = z - ((cordic_ctab[k] ^ d) - d);
      x = tx; 
      y = ty; 
      z = tz;
    }
  }

  return ((float) power_of((-1), i) * y * (float) power_of((-1), minus)) / MUL;
}

float cosine(float angle) {
  int i = 0;
  int theta = 0;
  int n = 32;
  int k, d, tx, ty, tz;
  int x=cordic_1K, y=0, z=0;

  if(angle < 0)
    angle = -angle;

  while(angle > (M_PI*2))
    angle -= (M_PI * 2);

  for(angle = angle; angle > (M_PI); angle -= (M_PI))
    i++;

  if(angle > M_PI/2) {
    angle -= (M_PI/2);
    x = sine(-angle) * MUL;
  }
  else {
    theta = angle * MUL;
    z=theta;
    n = (n>CORDIC_NTAB) ? CORDIC_NTAB : n;

    for (k=0; k<n; ++k) {
      d = z>>31;
      tx = x - (((y>>k) ^ d) - d);
      ty = y + (((x>>k) ^ d) - d);
      tz = z - ((cordic_ctab[k] ^ d) - d);
      x = tx; 
      y = ty; 
      z = tz;
    }
  }

  return ((float) power_of((-1), i) * x) / MUL;
}


void memcpy(void *dest, const void *source, U32 len) {
  U8 *dst = (U8*)dest;
  U8 *src = (U8*)source;

  NX_ASSERT(dst != NULL);
  NX_ASSERT(src != NULL);

  while (len--) {
    *dst++ = *src++;
  }
}

void memset(void *dest, const U8 val, U32 len) {
  U8 *dst = (U8*)dest;

  NX_ASSERT(dst != NULL);

  while (len--) {
    *dst++ = val;
  }
}

U32 strlen(const char *str) {
  U32 i = 0;

  NX_ASSERT(str != NULL);

  while (*str++) {
    i++;
  }

  return i;
}

bool streqn(const char *a, const char *b, U32 n) {
  NX_ASSERT(a != NULL && b != NULL);

  while (n--) {
    if (*a != *b++)
      return FALSE;
    if (*a++ == '\0')
      break;
  }

  return TRUE;
}

bool streq(const char *a, const char *b) {
  NX_ASSERT(a != NULL && b != NULL);

  while (*a != '\0' && *b != '\0') {
    if (*a++ != *b++)
      return FALSE;
  }

  return *a == *b ? TRUE : FALSE;
}

char *strchr(const char *s, const char c) {
  NX_ASSERT(s != NULL);

  while (*s) {
    if (*s == c)
      return (char*)s;
    s++;
  }

  return NULL;
}

char *strrchr(const char *s, const char c) {
  const char *ptr = NULL;

  NX_ASSERT(s != NULL);

  while (*s) {
    if (*s == c)
      ptr = s;
    s++;
  }

  return (char*)ptr;
}

bool atou32(const char *s, U32* result) {
  U32 prev = 0;

  NX_ASSERT(s != NULL && result != NULL);

  *result = 0;

  // Skip leading zeros
  while (*s && *s == '0')
    s++;

  for (; *s; s++) {
    // Return 0 on invalid characters.
    if (*s < '0' || *s > '9')
      return FALSE;

    *result = (10 * *result) + (*s - '0');
    // Naive overflow check. We could do better in pure asm by
    // checking the ALU flags.
    if (*result < prev)
      return FALSE;

    prev = *result;
  }

  return TRUE;
}

bool atos32(const char *s, S32 *result) {
  S32 prev = 0;
  bool negative = FALSE;

  NX_ASSERT(s != NULL && result != NULL);

  *result = 0;

  if (*s == '-') {
    negative = TRUE;
    s++;
  }

  // Skip leading zeros
  while (*s && *s == '0')
    s++;

  for (; *s; s++) {
    // Return 0 on invalid characters.
    if (*s < '0' || *s > '9')
      return FALSE;

    *result = (10 * *result) + (*s - '0');

    // Naive overflow check. We could do better in pure asm by
    // checking the ALU flags.
    if (*result < prev)
      return FALSE;

    prev = *result;
  }

  if (negative)
    *result = -(*result);

  return TRUE;
}
