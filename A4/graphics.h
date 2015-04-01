// Nathan Irwin
// CMPS 109
// Winter 2015
// Assignment 4
// $Id: graphics.h,v 1.9 2014-05-15 16:42:55-07 - - $

#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include <memory>
#include <vector>
#include <cmath>
using namespace std;

#include <GL/freeglut.h>

#include "rgbcolor.h"
#include "shape.h"

class object {
    friend class window;
private:
    shared_ptr<shape> pshape;
    vertex center;
    rgbcolor color;
public:
    // Default copiers, movers, dtor all OK.
    object(const shared_ptr<shape> &shape_ptr, const vertex& c,
           const rgbcolor& col){
        pshape = shape_ptr;
        center = c;
        color = col;
    };
    void draw() { pshape->draw (center, color); }
    void draw(const rgbcolor &col) { pshape->draw(center,col); }
    void move (GLfloat delta_x, GLfloat delta_y, float w, float h) {
        center.xpos = (fmod(delta_x + center.xpos, w));
        if(center.xpos < 0) center.xpos += w;
        center.ypos = fmod(delta_y + center.ypos, h);
    }
};

class mouse {
    friend class window;
private:
    int xpos {0};
    int ypos {0};
    int entered {GLUT_LEFT};
    int left_state {GLUT_UP};
    int middle_state {GLUT_UP};
    int right_state {GLUT_UP};
private:
    void set (int x, int y) { xpos = x; ypos = y; }
    void state (int button, int state);
    void draw();
};


class window {
    friend class mouse;
    friend class object;
private:
    static float moveBy;
    static float borderThick;
    static rgbcolor borderColor;
    static int width;         // in pixels
    static int height;        // in pixels
    static vector<object> objects;
    static size_t selected_obj;
    static mouse mus;
private:
    static void close();
    static void entry (int mouse_entered);
    static void display();
    static void reshape (int width, int height);
    static void keyboard (GLubyte key, int, int);
    static void special (int key, int, int);
    static void motion (int x, int y);
    static void passivemotion (int x, int y);
    static void mousefn (int button, int state, int x, int y);
    static void select_object (size_t obj);
    static void move_selected_object (float changeX, float changeY);
public:
    static void push_back (const object& obj) {
        objects.push_back (obj); }
    static void setwidth (int width_) { width = width_; }
    static void setheight (int height_) { height = height_; }
    static void set_moveBy (float move_by) { moveBy = move_by; }
    static void set_border (const rgbcolor& color, float thick) {
        borderThick = thick;
        borderColor = color;
    }
    static void main();
};

#endif
