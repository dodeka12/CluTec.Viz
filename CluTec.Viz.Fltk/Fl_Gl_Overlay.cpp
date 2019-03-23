//
// "$Id: Fl_Gl_Overlay.cxx,v 1.1 2004/02/18 16:10:25 chp Exp $"
//
// OpenGL overlay code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2003 by Bill Spitzak and others.
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
// Please report all bugs and problems to "fltk-bugs@fltk.org".
//

#include "Fl_config.h"
#if HAVE_GL

#include "Fl.H"
#include "x.H"
#include "Fl_Gl_Choice.H"
#include "Fl_Gl_Window.H"
#include <stdlib.h>

#if !HAVE_GL_OVERLAY

int Fl_Gl_Window::can_do_overlay() {return 0;}

void Fl_Gl_Window::make_overlay() {overlay = this;}

#else

// Methods on Fl_Gl_Window that create an overlay window.  Because
// many programs don't need the overlay, this is seperated into this
// source file so it is not linked in if not used.

// Under X this is done by creating another window, of class _Fl_Gl_Overlay
// which is a subclass of Fl_Gl_Window except it uses the overlay planes.
// A pointer to this is stored in the "overlay" pointer of the Fl_Gl_Window.

// Under win32 another GLX context is created to draw into the overlay
// and it is stored in into the "overlay" pointer.

// In both cases if overlay hardware is unavailable, the overlay is
// "faked" by drawing into the main layers.  This is indicated by
// setting overlay == this.

#ifndef WIN32
////////////////////////////////////////////////////////////////
// X version

extern XVisualInfo *fl_find_overlay_visual();
extern XVisualInfo *fl_overlay_visual;
extern Colormap fl_overlay_colormap;
extern unsigned long fl_transparent_pixel;
extern uchar fl_overlay;

class _Fl_Gl_Overlay : public Fl_Gl_Window {
  void flush();
  void draw();
public:
  void show();
  _Fl_Gl_Overlay(int x, int y, int w, int h) :
    Fl_Gl_Window(x,y,w,h) {
    set_flag(INACTIVE);
  }
};

void _Fl_Gl_Overlay::flush() {
  make_current();
#ifdef BOXX_BUGS
  // The BoXX overlay is broken and you must not call swap-buffers. This
  // code will make it work, but we lose because machines that do support
  // double-buffered overlays will blink when they don't have to
  glDrawBuffer(GL_FRONT);
  draw();
#else
  draw();
  swap_buffers();
#endif
  glFlush();
  valid(1);
}

void _Fl_Gl_Overlay::draw() {
  if (!valid_) glClearIndex((GLfloat)fl_transparent_pixel);
  if (damage() != FL_DAMAGE_EXPOSE) glClear(GL_COLOR_BUFFER_BIT);
  Fl_Gl_Window *w = (Fl_Gl_Window *)parent();
  uchar save_valid = w->valid_;
  w->valid_ = valid_;
  fl_overlay = 1;
  w->draw_overlay();
  fl_overlay = 0;
  valid_ = w->valid_;
  w->valid_ = save_valid;
}

void _Fl_Gl_Overlay::show() {
  if (!shown()) {
    fl_background_pixel = int(fl_transparent_pixel);
    Fl_X::make_xid(this, fl_overlay_visual, fl_overlay_colormap);
    fl_background_pixel = -1;
    // find the outermost window to tell wm about the colormap:
    Fl_Window *w = window();
    for (;;) {Fl_Window *w1 = w->window(); if (!w1) break; w = w1;}
    XSetWMColormapWindows(fl_display, fl_xid(w), &(Fl_X::i(this)->xid), 1);
    context(fl_create_gl_context(fl_overlay_visual), 1);
    valid(0);
  }
  Fl_Gl_Window::show();
}

int Fl_Gl_Window::can_do_overlay() {
  return fl_find_overlay_visual() != 0;
}

void Fl_Gl_Window::make_overlay() {
  if (overlay) return;
  if (can_do_overlay()) {
    _Fl_Gl_Overlay* o = new _Fl_Gl_Overlay(0,0,w(),h());
    overlay = o;
    add(*o);
    o->show();
  } else {
    overlay = this; // fake the overlay
  }
}

#else
////////////////////////////////////////////////////////////////
// WIN32 version:

//static COLORREF *palette;
extern int fl_overlay_depth;

void Fl_Gl_Window::make_overlay() {
  if (overlay) return;

  GLContext context = fl_create_gl_context(this, g, 1);
  if (!context) {overlay = this; return;} // fake the overlay

  HDC hdc = Fl_X::i(this)->private_dc;
  overlay = context;
  LAYERPLANEDESCRIPTOR pfd;
  wglDescribeLayerPlane(hdc, g->pixelformat, 1, sizeof(pfd), &pfd);
  if (!pfd.iPixelType) {
    ; // full-color overlay
  } else {
    fl_overlay_depth = pfd.cColorBits; // used by gl_color()
    if (fl_overlay_depth > 8) fl_overlay_depth = 8;
    COLORREF palette[256];
    int n = (1<<fl_overlay_depth)-1;
    // copy all colors except #0 into the overlay palette:
    for (int i = 0; i <= n; i++) {
      uchar r,g,b; Fl::get_color((Fl_Color)i,r,g,b);
      palette[i] = RGB(r,g,b);
    }
    // always provide black & white in the last 2 pixels:
    if (fl_overlay_depth < 8) {
      palette[n-1] = RGB(0,0,0);
      palette[n] = RGB(255,255,255);
    }
    // and use it:
    wglSetLayerPaletteEntries(hdc, 1, 1, n, palette+1);
    wglRealizeLayerPalette(hdc, 1, TRUE);
  }
  valid(0);
  return;
}

int Fl_Gl_Window::can_do_overlay() {
  if (!g) {
    g = Fl_Gl_Choice::find(mode_,alist);
    if (!g) return 0;
  }
  return (g->pfd.bReserved & 15) != 0;
}

////////////////////////////////////////////////////////////////
#endif

#endif

void Fl_Gl_Window::redraw_overlay() {
  if (!shown()) return;
  make_overlay();
#ifndef WIN32
  if (overlay != this)
    ((Fl_Gl_Window*)overlay)->redraw();
  else
#endif
    damage(FL_DAMAGE_OVERLAY);
}

void Fl_Gl_Window::make_overlay_current() {
  make_overlay();
#if HAVE_GL_OVERLAY
  if (overlay != this) {
#ifdef WIN32
    fl_set_gl_context(this, (GLContext)overlay);
//  if (fl_overlay_depth)
//    wglRealizeLayerPalette(Fl_X::i(this)->private_dc, 1, TRUE);
#else
    ((Fl_Gl_Window*)overlay)->make_current();
#endif
  } else
#endif
    glDrawBuffer(GL_FRONT);
}

void Fl_Gl_Window::hide_overlay() {
#if HAVE_GL_OVERLAY
#ifdef WIN32
  // nothing needs to be done?  Or should it be erased?
#else
  if (overlay && overlay!=this) ((Fl_Gl_Window*)overlay)->hide();
#endif
#endif
}

#endif

//
// End of "$Id: Fl_Gl_Overlay.cxx,v 1.1 2004/02/18 16:10:25 chp Exp $".
//
