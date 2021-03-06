//
// "$Id$"
//
// Definition of SDL Window interface
// for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2016 by Bill Spitzak and others.
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

/**
 \file Fl_PicoSDL_Window_Driver.H
 \brief Definition of SDL Window interface.
 */

#ifndef FL_PICOSDL_WINDOW_DRIVER_H
#define FL_PICOSDL_WINDOW_DRIVER_H

#include "../Pico/Fl_Pico_Window_Driver.H"

#define __APPLE__
#include <SDL2/SDL.h>
#undef __APPLE__


class FL_EXPORT Fl_PicoSDL_Window_Driver : public Fl_Pico_Window_Driver
{
  SDL_Window *pNativeWindow;
  SDL_Texture *pNativeTexture;
public:
  Fl_PicoSDL_Window_Driver(Fl_Window *win);
  virtual ~Fl_PicoSDL_Window_Driver();

  virtual void show();
  virtual Fl_X *makeWindow();
  virtual void make_current();
  virtual void draw_end();
  
  // --- window management
//  virtual void flush_single();
};


#endif // FL_PICOSDL_WINDOW_DRIVER_H

//
// End of "$Id$".
//
