// This is likely to change!

#ifndef fltk_Cursor_h
#define fltk_Cursor_h

#include "FL_API.h"

namespace fltk {

struct Cursor; // this is not public

class Image;
Cursor* cursor(Image*, int x, int y);

extern FL_API Cursor* const CURSOR_DEFAULT; // == NULL
extern FL_API Cursor* const CURSOR_ARROW;
extern FL_API Cursor* const CURSOR_CROSS;
extern FL_API Cursor* const CURSOR_WAIT;
extern FL_API Cursor* const CURSOR_INSERT;
extern FL_API Cursor* const CURSOR_HAND;
extern FL_API Cursor* const CURSOR_HELP;
extern FL_API Cursor* const CURSOR_MOVE;
extern FL_API Cursor* const CURSOR_NS;
extern FL_API Cursor* const CURSOR_WE;
extern FL_API Cursor* const CURSOR_NWSE;
extern FL_API Cursor* const CURSOR_NESW;
extern FL_API Cursor* const CURSOR_NO;
extern FL_API Cursor* const CURSOR_NONE;

}
#endif
