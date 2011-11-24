/* Copyright (c) 2009 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

/* Drawing example.
 *
 * This example demonstrates the use of the NxOS display
 * API to draw on the screen. Because displaying information is so useful, you've
 * probably seen a couple of its functions in use in other
 * examples. Here, we'll go into more detail, covering the whole
 * display API.
 */

#include "base/drivers/avr.h"
#include "base/core.h"
#include "base/display.h"
#include "base/drivers/systick.h" /* for nx_systick_wait_ms */
#include <math.h>

void main() {
  /* We want to draw a moving ellipse, two arcs and two lines */
  /* Declaring center point of the ellipse / arcs and  points representing the lines */
  point ellipse_c, q, r, s, t;

  /* Initialize ellipse center position */
  ellipse_c.x = 1;
  ellipse_c.y = 1;

  /* Initialize the first line position*/
  q.x = 3;
  q.y = 63;

  s.x = 28;
  s.y = 63;

  /* Initialize the second line position */
  r.x = 102;
  r.y = 63;

  t.x = 76;
  t.y = 63;

  /* Superior radius of the ellipse */
  U8 sradius = 85;
  /* Difference between the superior and inferior radius of the ellipse */
  U8 delta = 73;
  /* Offset angle for arcs rotation */
  U32 offset_angle = 60;
  /* Disable auto refresh of the screen, we want to control each frame */
  nx_display_auto_refresh(FALSE);

  /* Loop until the cancel button is pushed 
  Each loop iteration represent a frame composed of three steps*/
  while (nx_avr_get_button() != BUTTON_CANCEL) {
    /* Clear the screen */
    nx_display_clear();

    /* First step of the frame : calculate positions and parameters of shapes */

    /* If the ellipse is in the first half of the screen,
      make it follow a pseudo sinusoidal path */
    if(ellipse_c.x < 53) {
      ellipse_c.x ++;
      delta--;
      ellipse_c.y = (30 * sin( ((float) ellipse_c.x) / 25) + 4);
    }
    /* If the ellipse have past the first third of the screen,
      then start to move lines and rotate arcs */
    if(ellipse_c.x > 34) {

      if(ellipse_c.x < 49) {
        q.x = (q.x) + 2;
        s.x = (s.x) + 2;
        r.x = (r.x) - 2;
        t.x = (t.x) - 2;
        offset_angle += ellipse_c.x;
      }
      else if(offset_angle > 5)
        offset_angle -= 5;
    }

    /* Second step of the frame : draw shapes on the screen buffer*/

    /* If the ellipse have past the first third of the screen, 
    draw moving arcs around and lines at the bottom of the screen */    
    if(ellipse_c.x > (34)) {
      nx_display_arc(ellipse_c, 27, 45, offset_angle);
      nx_display_arc(ellipse_c, 27, 45, (180 + offset_angle));
      nx_display_line(q, s);
      nx_display_line(r, t);
    }
    /* If the ellipse is at the middle of the screen, 
    write "NxOS on the screen */
    if(ellipse_c.x > 52) {
      nx_display_cursor_set_pos(7, 3);
      nx_display_string("NxOS");
    }
    /* Just draw the ellipse, every frame */
    nx_display_ellipse(ellipse_c, (sradius / 4), ((sradius - delta) / 3), (ellipse_c.x * 20) ); 

    /* Last step of the frame : refresh the screen */
    nx_display_refresh();

    /* Wait a bit before next frame */
    nx_systick_wait_ms(85);
  }    
  /* Shutdown the brick */
  nx_core_halt();

}
