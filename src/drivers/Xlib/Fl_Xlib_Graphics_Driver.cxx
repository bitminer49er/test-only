//
// "$Id$"
//
// Rectangle drawing routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2016 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//


#include "../../config_lib.h"
#include "Fl_Xlib_Graphics_Driver.H"
#include "Fl_Font.H"
#include <FL/fl_draw.H>
#include <FL/x.H>

#include <string.h>

#if HAVE_XRENDER
#include <X11/extensions/Xrender.h>
#endif

extern XIC fl_xim_ic;
extern char fl_is_over_the_spot;
#if !USE_XFT
extern char *fl_get_font_xfld(int fnum, int size);
#endif

/*
 * By linking this module, the following static method will instantiate the
 * X11 Xlib Graphics driver as the main display driver.
 */
Fl_Graphics_Driver *Fl_Graphics_Driver::newMainGraphicsDriver()
{
  return new Fl_Xlib_Graphics_Driver();
}

GC Fl_Xlib_Graphics_Driver::gc_ = NULL;

/* Reference to the current graphics context
 For back-compatibility only. The preferred procedure to get this pointer is
 Fl_Surface_Device::surface()->driver()->gc().
 */
GC fl_gc = 0;

Fl_Xlib_Graphics_Driver::Fl_Xlib_Graphics_Driver(void) {
  mask_bitmap_ = NULL;
  p_size = 0;
  p = NULL;
  line_width_ = 0;
#if USE_PANGO
  pfd_ = pango_font_description_new();
  Fl_Graphics_Driver::font(0, 0);
#endif
  offset_x_ = 0; offset_y_ = 0;
  depth_ = 0;
}

Fl_Xlib_Graphics_Driver::~Fl_Xlib_Graphics_Driver() {
  if (p) free(p);
#if USE_PANGO
  pango_font_description_free(pfd_);
#endif
}


void Fl_Xlib_Graphics_Driver::gc(void *value) {
  gc_ = (GC)value;
  fl_gc = gc_;
}

void Fl_Xlib_Graphics_Driver::copy_offscreen(int x, int y, int w, int h, Fl_Offscreen pixmap, int srcx, int srcy) {
  XCopyArea(fl_display, pixmap, fl_window, gc_, srcx, srcy, w, h, x+offset_x_, y+offset_y_);
}

void Fl_Xlib_Graphics_Driver::add_rectangle_to_region(Fl_Region r, int X, int Y, int W, int H) {
  XRectangle R;
  R.x = X; R.y = Y; R.width = W; R.height = H;
  XUnionRectWithRegion(&R, r, r);
}

void Fl_Xlib_Graphics_Driver::transformed_vertex0(short x, short y) {
  if (!n || x != p[n-1].x || y != p[n-1].y) {
    if (n >= p_size) {
      p_size = p ? 2*p_size : 16;
      p = (XPOINT*)realloc((void*)p, p_size*sizeof(*p));
    }
    p[n].x = x;
    p[n].y = y;
    n++;
  }
}

void Fl_Xlib_Graphics_Driver::fixloop() {  // remove equal points from closed path
  while (n>2 && p[n-1].x == p[0].x && p[n-1].y == p[0].y) n--;
}

// FIXME: should be members of Fl_Xlib_Graphics_Driver
XRectangle fl_spot;
int fl_spotf = -1;
int fl_spots = -1;

void Fl_Xlib_Graphics_Driver::reset_spot(void)
{
  fl_spot.x = -1;
  fl_spot.y = -1;
  //if (fl_xim_ic) XUnsetICFocus(fl_xim_ic);
}

void Fl_Xlib_Graphics_Driver::set_spot(int font, int size, int X, int Y, int W, int H, Fl_Window *win)
{
  int change = 0;
  XVaNestedList preedit_attr;
  static XFontSet fs = NULL;
  char **missing_list;
  int missing_count;
  char *def_string;
  char *fnt = NULL;
  bool must_free_fnt =true;

  static XIC ic = NULL;

  if (!fl_xim_ic || !fl_is_over_the_spot) return;
  //XSetICFocus(fl_xim_ic);
  if (X != fl_spot.x || Y != fl_spot.y) {
    fl_spot.x = X;
    fl_spot.y = Y;
    fl_spot.height = H;
    fl_spot.width = W;
    change = 1;
  }
  if (font != fl_spotf || size != fl_spots) {
    fl_spotf = font;
    fl_spots = size;
    change = 1;
    if (fs) {
      XFreeFontSet(fl_display, fs);
    }
#if USE_XFT

#if defined(__GNUC__)
    // FIXME: warning XFT support here
#endif /*__GNUC__*/

    fnt = NULL; // fl_get_font_xfld(font, size);
    if (!fnt) {fnt = (char*)"-misc-fixed-*";must_free_fnt=false;}
    fs = XCreateFontSet(fl_display, fnt, &missing_list,
                        &missing_count, &def_string);
#else
    fnt = fl_get_font_xfld(font, size);
    if (!fnt) {fnt = (char*)"-misc-fixed-*";must_free_fnt=false;}
    fs = XCreateFontSet(fl_display, fnt, &missing_list,
                        &missing_count, &def_string);
#endif
  }
  if (fl_xim_ic != ic) {
    ic = fl_xim_ic;
    change = 1;
  }

  if (fnt && must_free_fnt) free(fnt);
  if (!change) return;


  preedit_attr = XVaCreateNestedList(0,
                                     XNSpotLocation, &fl_spot,
                                     XNFontSet, fs, NULL);
  XSetICValues(fl_xim_ic, XNPreeditAttributes, preedit_attr, NULL);
  XFree(preedit_attr);
}

unsigned Fl_Xlib_Graphics_Driver::font_desc_size() {
  return (unsigned)sizeof(Fl_Fontdesc);
}

const char *Fl_Xlib_Graphics_Driver::font_name(int num) {
  return fl_fonts[num].name;
}

void Fl_Xlib_Graphics_Driver::font_name(int num, const char *name) {
  Fl_Fontdesc *s = fl_fonts + num;
  if (s->name) {
    if (!strcmp(s->name, name)) {s->name = name; return;}
    if (s->xlist && s->n >= 0) XFreeFontNames(s->xlist);
    for (Fl_Font_Descriptor* f = s->first; f;) {
      Fl_Font_Descriptor* n = f->next; delete f; f = n;
    }
    s->first = 0;
  }
  s->name = name;
  s->fontname[0] = 0;
  s->xlist = 0;
  s->first = 0;
}

void Fl_Xlib_Graphics_Driver::translate_all(int dx, int dy) { // reversibly adds dx,dy to the offset between user and graphical coordinates
  stack_x_[depth_] = offset_x_;
  stack_y_[depth_] = offset_y_;
  offset_x_ = stack_x_[depth_] + dx;
  offset_y_ = stack_y_[depth_] + dy;
  push_matrix();
  translate(dx, dy);
  if (depth_ < sizeof(stack_x_)/sizeof(int)) depth_++;
  else Fl::warning("%s: translate stack overflow!", "Fl_Xlib_Graphics_Driver");
}

void Fl_Xlib_Graphics_Driver::untranslate_all() { // undoes previous translate_all()
  if (depth_ > 0) depth_--;
  offset_x_ = stack_x_[depth_];
  offset_y_ = stack_y_[depth_];
  pop_matrix();
}

//
// End of "$Id$".
//
