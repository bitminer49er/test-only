//
// "$Id: fl_font_win32.cxx,v 1.13 1999/08/27 14:00:13 carl Exp $"
//
// WIN32 font selection routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-1999 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to "fltk-bugs@easysw.com".
//

#include <config.h>
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/win32.H>
#include "Fl_FontSize.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>


const char *Fl::encoding = "iso8859-1";

Fl_FontSize::Fl_FontSize(const char* name, int size) {
  int weight = FW_NORMAL;
  int italic = 0;
  switch (*name++) {
  case 'I': italic = 1; break;
  case 'P': italic = 1;
  case 'B': weight = FW_BOLD; break;
  case ' ': break;
  default: name--;
  }
  fid = CreateFont(
    -size, // negative makes it use "char size"
    0,	            // logical average character width 
    0,	            // angle of escapement 
    0,	            // base-line orientation angle 
    weight,
    italic,
    FALSE,	        // underline attribute flag 
    FALSE,	        // strikeout attribute flag 
    DEFAULT_CHARSET,    // character set identifier 
    OUT_DEFAULT_PRECIS,	// output precision 
    CLIP_DEFAULT_PRECIS,// clipping precision 
    DEFAULT_QUALITY,	// output quality 
    DEFAULT_PITCH,	// pitch and family 
    name	        // pointer to typeface name string 
    );
  if (!fl_gc) fl_GetDC(0);
  SelectObject(fl_gc, fid);
  GetTextMetrics(fl_gc, &metr);
//  BOOL ret = GetCharWidthFloat(fl_gc, metr.tmFirstChar, metr.tmLastChar, font->width+metr.tmFirstChar);
// ...would be the right call, but is not implemented into Window95! (WinNT?)
  GetCharWidth(fl_gc, 0, 255, width);
#if HAVE_GL
  listbase = 0;
#endif
  minsize = maxsize = size;
}

Fl_FontSize::~Fl_FontSize() {
#if HAVE_GL
// Delete list created by gl_draw().  This is not done by this code
// as it will link in GL unnecessarily.  There should be some kind
// of "free" routine pointer, or a subclass?
// if (listbase) {
//  int base = font->min_char_or_byte2;
//  int size = font->max_char_or_byte2-base+1;
//  int base = 0; int size = 256;
//  glDeleteLists(listbase+base,size);
// }
#endif
  if (this == fl_font_) fl_font_ = 0;
  DeleteObject(fid);
}

////////////////////////////////////////////////////////////////

// The predefined fonts that fltk has:  bold:       italic:
Fl_Font_ fl_fonts[] = {
{" Arial",				fl_fonts+1, fl_fonts+2},
{"BArial", 				fl_fonts+1, fl_fonts+3},
{"IArial",				fl_fonts+3, fl_fonts+2},
{"PArial",				fl_fonts+3, fl_fonts+3},
{" Courier New",			fl_fonts+5, fl_fonts+6},
{"BCourier New",			fl_fonts+5, fl_fonts+7},
{"ICourier New",			fl_fonts+7, fl_fonts+6},
{"PCourier New",			fl_fonts+7, fl_fonts+7},
{" Times New Roman",			fl_fonts+9, fl_fonts+10},
{"BTimes New Roman",			fl_fonts+9, fl_fonts+11},
{"ITimes New Roman",			fl_fonts+11,fl_fonts+10},
{"PTimes New Roman",			fl_fonts+11,fl_fonts+11},
{" Symbol",				fl_fonts+12,fl_fonts+12},
{" Terminal",				fl_fonts+14,fl_fonts+14},
{"BTerminal",				fl_fonts+14,fl_fonts+14},
{" Wingdings",				fl_fonts+15,fl_fonts+15},
};

static Fl_FontSize* find(Fl_Font font, unsigned size) {
  Fl_Font_* s = (Fl_Font_*)font;
  if (!s->name_) s = fl_fonts; // use 0 if fnum undefined
  Fl_FontSize* f;
  for (f = s->first; f; f = f->next)
    if (f->minsize <= size && f->maxsize >= size) return f;
  f = new Fl_FontSize(s->name_, size);
  f->next = s->first;
  s->first = f;
  return f;
}

////////////////////////////////////////////////////////////////
// Public interface:

Fl_FontSize* fl_font_;
//static HDC font_gc;

void fl_font(Fl_FontSize* s) {
  if (s != fl_font_) {
    fl_font_ = s;
/* CET - FIXME
    fl_xfont = s->font;
    font_gc = 0;
*/
  }
}

void fl_font(Fl_Font s, unsigned size) {
  if (!s) s = FL_HELVETICA;
  static Fl_Font curfont;
  if (s == curfont) { // see if it is the current font...
      Fl_FontSize* f = fl_font_;
      if (f->minsize <= size && f->maxsize >= size) return;
  }
  curfont = s;

  Fl_FontSize* f;
  // search the fonts we have generated already:
  for (f = s->first; f; f = f->next)
    if (f->minsize <= size && f->maxsize >= size) {
      fl_font(f); return;
    }

  fl_font_ = find(s, size);
}

const char *fl_encoding = "Windows";  // CET - FIXME

void fl_font(Fl_Font s, unsigned size, const char *encoding) {
// CET - FIXME - Are we doing this encoding stuff on Windoze?
  fl_font(s, size);
}

int fl_height() {
  return (fl_font_->metr.tmAscent + fl_font_->metr.tmDescent);
}

int fl_descent() {
  return fl_font_->metr.tmDescent;
}

double fl_width(const char* c) {
  double w = 0.0;
  while (*c) w += fl_font_->width[uchar(*c++)];
  return w;
}

double fl_width(const char* c, int n) {
  double w = 0.0;
  while (n--) w += fl_font_->width[uchar(*c++)];
  return w;
}

double fl_width(uchar c) {
  return fl_font_->width[c];
}

void fl_draw(const char* str, int n, int x, int y) {
  SetTextColor(fl_gc, fl_RGB());
  SelectObject(fl_gc, fl_font_->fid);
  TextOut(fl_gc, x, y, str, n);
}

void fl_draw(const char* str, int x, int y) {
  fl_draw(str, strlen(str), x, y);
}

//
// End of "$Id: fl_font_win32.cxx,v 1.13 1999/08/27 14:00:13 carl Exp $".
//
