/* Copyright (c) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#include "base/types.h"
#include "base/interrupts.h"
#include "/opt/arm-gcc/src/newlib-1.17.0/newlib/libc/include/math.h" 
#include "base/util.h"
#include "base/assert.h"
#include "base/drivers/systick.h"
#include "base/drivers/aic.h"
#include "base/drivers/_lcd.h"

#include "base/_display.h"

/* A simple 8x5 font. This is in a separate file because the embedded
 * font is converted from a .png at compile time.
 */
#include "_font.h"

static struct {
  /* The display buffer, which is mirrored to the LCD controller's RAM. */
  U8 buffer[LCD_HEIGHT][LCD_WIDTH];

  /* Whether the display is automatically refreshed after every call
   * to display functions. */
  bool auto_refresh;

  /* The position of the text cursor. This is used for easy displaying
   * of text in a console-like manner.
   */
  struct {
    U8 x;
    U8 y;
    bool ignore_lf; /* If the display just wrapped from the right side
                       of the screen, ignore an LF immediately
                       after. */
  } cursor;
} display;


static inline void dirty_display(void) {
  if (display.auto_refresh)
    nx__lcd_dirty_display();
}

/* Clear the display. */
void nx_display_clear(void) {
  memset(&display.buffer[0][0], 0, sizeof(display.buffer));
  nx_display_cursor_set_pos(0, 0);
  dirty_display();
}


/* Enable or disable auto-refresh. */
void nx_display_auto_refresh(bool auto_refresh) {
  display.auto_refresh = auto_refresh;
  dirty_display();
}


/* Explicitely refresh the display. You only need to use this when
 * auto-refresh is disabled.
 */
inline void nx_display_refresh(void) {
  nx__lcd_dirty_display();
}

/* Simply test is the given point is on the screen */
static inline bool is_point_on_screen(point p) {
  if(p.x > 0 && p.x < LCD_PIXEL_WIDTH && p.y > 0 && p.y < LCD_PIXEL_WIDTH)
    return TRUE;

  else
    return FALSE;
}
/* Simply swap the values of two points */
static inline void swap_points(point *a, point *b) {
  point temp;

  temp.x = a->x;
  temp.y = a->y;
  a->x = b->x;
  a->y = b->y;
  b->x = temp.x;
  b->y = temp.y;
}

/* Rotate of "angle" degrees a point around another point */
static point rotate_point(point a, point center, U32 angle) {
  point result;
  float rangle;

  if(angle == 0)
    return a;

  rangle = (angle * M_PI) / 180;

  result.x = (a.x - center.x) * cosf(rangle) - (a.y - center.y) * sinf(rangle) + center.x;
  result.y = (a.x - center.x) * sinf(rangle) + (a.y - center.y) * cosf(rangle) + center.y;

  return result;
}

/* Draw a point on the screen buffer, pixels coordinates (100 * 64) */
bool nx_display_point(point p) {
  if(!is_point_on_screen(p))
    return 1;
  
  memset(&display.buffer[p.y / 8][p.x], ((0x1 << (p.y % 8)) | display.buffer[p.y / 8][p.x]), sizeof(U8));
  nx_display_cursor_set_pos(0, 0);
  dirty_display();

  return 0;
}

/* Draw a line represented by 2 points */
S8 nx_display_line(point a, point b) {
  point current;

  if(!(is_point_on_screen(a) && is_point_on_screen(b)))
    return -1;
  /* We want Xa < Xb */
  if(a.x > b.x)
    swap_points(&a, &b);
  /* Display the ends of the line */
  nx_display_point(a);
  nx_display_point(b);
  /* Calculate the slope */
  S8 dx = (b.x - a.x);
  S8 dy = (b.y - a.y);

  current = a;

  if(dx == 0)
    for(current.y = a.y; current.y != b.y; a.y < b.y ? current.y++ : current.y--)
      nx_display_point(current);
  else {  
    /* For each abscissa, calculate the ordinate and display all points between two calculated points*/
    for(current.x = a.x; current.x <= b.x; current.x++) {
      current.y =  a.y + ( ( dy *  (current.x - a.x) ) / dx  );
      nx_display_point(current);
      /* Display all points between two points to avoid gaps */
      while(current.y != a.y + ( ( dy *  (current.x+1 - a.x) ) / dx  )) {
        nx_display_point(current);
        a.y < b.y ? current.y++ : current.y--;
      }
    }
  }
  return 0;
}

/* Draw an ellipse represented by its center and its 2 radius */
S8 nx_display_ellipse(point center, U8 smj, U8 smn, U32 angle) {
  point current;
  point to_draw;

  if(!is_point_on_screen(center))
    return -1;
  /* For each abscissa, calculate the ordinates and display two point*/
  for(current.x = (center.x - smj); current.x < (center.x + smj); current.x++) {
    current.y = sqrti( (pow(smn, 2) * (pow(smj, 2) - pow((current.x - center.x), 2))) / pow(smj, 2) ) + center.y;
    to_draw = rotate_point(current, center, angle);
    nx_display_point(to_draw);

    current.y = center.y - (current.y - center.y);
    to_draw = rotate_point(current, center, angle);
    nx_display_point(to_draw);
  }
  /* For each ordinate, calculate the abscissas and display two point*/
  for(current.y = (center.y - smn); current.y < (center.y + smn); current.y++) {
    current.x = sqrti( (pow(smj, 2) * (pow(smn, 2) - pow((current.y - center.y), 2))) / pow(smn, 2) ) + center.x;
    to_draw = rotate_point(current, center, angle);
    nx_display_point(to_draw); 

    current.x = center.x - (current.x - center.x);
    to_draw = rotate_point(current, center, angle);
    nx_display_point(to_draw);
  }

  return 0;
}
/* Draw an arc represented by its center, its radius, its angle, and an angle offset */
S8 nx_display_arc(point center, U8 radius, U32 angle, U32 offset) {
  point current;
  point to_draw;
  float rangle;

  if(!is_point_on_screen(center))
    return -1;

  rangle = (angle * M_PI) / 180;
  /* If the angle is inferior to 90 degrees, display it */
  if(angle <= 90) {
    /* For each abscissa, calculate the ordinates and display two point*/
    for(current.y = center.y; current.y <= (center.y + (radius * sinf(rangle))); current.y++) {
      current.x = sqrti(pow(radius, 2) - pow((current.y - center.y), 2)) + center.x;
      to_draw = rotate_point(current, center, offset);
      nx_display_point(to_draw);
    }
    /* For each ordinate, calculate the abscissas and display two point*/
    for(current.x = center.x; current.x <= (center.x + (radius * cosf(rangle))); current.x++) {
      current.y = sqrti(pow(radius, 2) - pow((current.x - center.x), 2)) + center.y;
      to_draw = rotate_point(current, center, offset);
      nx_display_point(to_draw);
    }

  }
  /* If the angle is superior to 90 degrees, recursion occurs */
  else {
    nx_display_arc(center, radius, 90, offset);
    nx_display_arc(center, radius, (angle - 90), (offset + 90));
  }

  return 0;
}

/*
 * Text display functions.
 */
static inline bool is_on_screen(U8 x, U8 y) {
  if (x < NX__DISPLAY_WIDTH_CELLS &&
      y < NX__DISPLAY_HEIGHT_CELLS)
    return TRUE;
  else
    return FALSE;
}

static inline const U8 *char_to_font(const char c) {
  if (c >= NX__FONT_START)
    return nx__font_data[c - NX__FONT_START];
  else
    return nx__font_data[0]; /* Unprintable characters become spaces. */
}

static inline void update_cursor(bool inc_y) {
  if (!inc_y) {
    display.cursor.x++;

    if (display.cursor.x >= LCD_WIDTH / NX__CELL_WIDTH) {
      display.cursor.x = 0;
      display.cursor.y++;
      display.cursor.ignore_lf = TRUE;
    } else {
      display.cursor.ignore_lf = FALSE;
    }
  } else if (display.cursor.ignore_lf) {
    display.cursor.ignore_lf = FALSE;
  } else {
    display.cursor.x = 0;
    display.cursor.y++;
  }

  if (display.cursor.y >= LCD_HEIGHT)
    display.cursor.y = 0;
}

void nx_display_cursor_set_pos(U8 x, U8 y) {
  NX_ASSERT(is_on_screen(x, y));
  display.cursor.x = x;
  display.cursor.y = y;
}

inline void nx_display_end_line(void) {
  update_cursor(TRUE);
}

void nx_display_string(const char *str) {
  while (*str != '\0') {
    if (*str == '\n')
      update_cursor(TRUE);
    else {
      int x_offset = display.cursor.x * NX__CELL_WIDTH;
      memcpy(&display.buffer[display.cursor.y][x_offset],
             char_to_font(*str), NX__FONT_WIDTH);
      update_cursor(FALSE);
    }
    str++;
  }
  dirty_display();
}

void nx_display_hex(U32 val) {
  const char hex[16] = "0123456789ABCDEF";
  char buf[9];
  char *ptr = &buf[8];

  if (val == 0) {
    ptr--;
    *ptr = hex[0];
  } else {
    while (val != 0) {
      ptr--;
      *ptr = hex[val & 0xF];
      val >>= 4;
    }
  }

  buf[8] = '\0';

  nx_display_string(ptr);
  dirty_display();
}

void nx_display_uint(U32 val) {
  char buf[11];
  char *ptr = &buf[10];

  if (val == 0) {
    ptr--;
    *ptr = '0';
  } else {

    while (val > 0) {
      ptr--;
      *ptr = val % 10 + '0';
      val /= 10;
    }

  }

  buf[10] = '\0';

  nx_display_string(ptr);
  dirty_display();
}

void nx_display_int(S32 val) {
  if( val < 0 ) {
    nx_display_string("-");
    val = -val;
  }
  nx_display_uint(val);
}

/*
 * Display initialization.
 */
void nx__display_init(void) {
  display.auto_refresh = FALSE;
  nx_display_clear();
  display.cursor.x = 0;
  display.cursor.y = 0;
  display.cursor.ignore_lf = FALSE;
  nx__lcd_set_display(&display.buffer[0][0]);
  display.auto_refresh = TRUE;
  dirty_display();
}
