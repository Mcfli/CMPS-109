// Nathan Irwin
// CMPS 109
// Winter 2015
// Assignmnet 4
// $Id: shape.cpp,v 1.7 2014-05-08 18:32:56-07 - - $

#include <typeinfo>
#include <unordered_map>
#include <cmath>

using namespace std;

#include "shape.h"
#include "util.h"

static unordered_map<void*,string> fontname {
    {GLUT_BITMAP_8_BY_13       , "Fixed-8x13"    },
    {GLUT_BITMAP_9_BY_15       , "Fixed-9x15"    },
    {GLUT_BITMAP_HELVETICA_10  , "Helvetica-10"  },
    {GLUT_BITMAP_HELVETICA_12  , "Helvetica-12"  },
    {GLUT_BITMAP_HELVETICA_18  , "Helvetica-18"  },
    {GLUT_BITMAP_TIMES_ROMAN_10, "Times-Roman-10"},
    {GLUT_BITMAP_TIMES_ROMAN_24, "Times-Roman-24"},
};

static unordered_map<string,void*> fontcode {
    {"Fixed-8x13"    , GLUT_BITMAP_8_BY_13       },
    {"Fixed-9x15"    , GLUT_BITMAP_9_BY_15       },
    {"Helvetica-10"  , GLUT_BITMAP_HELVETICA_10  },
    {"Helvetica-12"  , GLUT_BITMAP_HELVETICA_12  },
    {"Helvetica-18"  , GLUT_BITMAP_HELVETICA_18  },
    {"Times-Roman-10", GLUT_BITMAP_TIMES_ROMAN_10},
    {"Times-Roman-24", GLUT_BITMAP_TIMES_ROMAN_24},
};

ostream& operator<< (ostream& out, const vertex& where) {
    out << "(" << where.xpos << "," << where.ypos << ")";
    return out;
}

// Shape constructors

shape::shape() {
    DEBUGF ('c', this);
}

text::text (const string& fontStr, const string& textdata):
fontStr(fontStr), textdata(textdata) {
    DEBUGF ('c', this);
    glut_bitmap_font = fontcode[fontStr];
}

ellipse::ellipse (GLfloat width, GLfloat height):
dimension ({width, height}) {
    DEBUGF ('c', this);
}

circle::circle (GLfloat diameter): ellipse (diameter, diameter) {
    DEBUGF ('c', this);
}

polygon::polygon (const vertex_list& vertices): vertices(vertices) {
    DEBUGF ('c', this);
}

rectangle::rectangle (GLfloat width, GLfloat height):
polygon({{width/2, -height/2},
        {width/2, height/2},
        {-width/2, height/2},
        {-width/2, -height/2}}) {
    DEBUGF ('c', this << "(" << width << "," << height << ")");
}

square::square (GLfloat width): rectangle (width, width) {
    DEBUGF ('c', this);
}

diamond::diamond(GLfloat width, GLfloat height):
polygon({{width/2, 0},
        {0, height/2},
        {-width/2, 0},
        {0, -height/2}}){
    DEBUGF ('c', this << "(" << width << "," << height << ")");
}

triangle::triangle(const vertex v0, const vertex v1, const vertex v2):
polygon({v0,v1,v2}){
    DEBUGF('c', this);
}

isosceles::isosceles(GLfloat width, GLfloat height):
triangle({width/2, 0},
         {0, height},
         {-width/2, 0}){
    DEBUGF('c', this);
}

right_triangle::right_triangle(GLfloat width, GLfloat height):
triangle({width, 0},
         {0, height},
         {0, 0}){
    DEBUGF('c', this);
}

equilateral::equilateral(GLfloat width):
isosceles(width, width){
    DEBUGF('c', this);
}

// Draw functions

void text::draw (const vertex& center, const rgbcolor& color) const {
    DEBUGF ('d', this << "(" << center << "," << color << ")");
    glColor3ubv(color.ubvec);
    glRasterPos2f(center.xpos, center.ypos);
    for(auto c:textdata) glutBitmapCharacter(glut_bitmap_font, c);
}

void ellipse::draw (const vertex& center, const rgbcolor& color) const {
    DEBUGF ('d', this << "(" << center << "," << color << ")");
    glBegin (GL_POLYGON);
    glEnable (GL_LINE_SMOOTH);
    glColor3ubv (color.ubvec);
    const float delta = 2 * M_PI / 32;
    float width = dimension.xpos;
    float height = dimension.ypos;
    for(float theta = 0; theta < 2 * M_PI; theta += delta){
        float xpos = width * cos (theta) + center.xpos;
        float ypos = height * sin (theta) + center.ypos;
        glVertex2f (xpos, ypos);
    }
    glEnd();
}

void polygon::draw (const vertex& center, const rgbcolor& color) const {
    DEBUGF ('d', this << "(" << center << "," << color << ")");
    glBegin (GL_POLYGON);
    glEnable (GL_LINE_SMOOTH);
    glColor3ubv (color.ubvec);
    for(auto v : vertices){
        float xDraw = v.xpos + center.xpos;
        float yDraw = v.ypos + center.ypos;
        glVertex2f(xDraw, yDraw);
    }
    glEnd();
}

// Show functions

void shape::show (ostream& out) const {
    out << this << "->" << demangle (*this) << ": ";
}

void text::show (ostream& out) const {
    shape::show (out);
    out << glut_bitmap_font << "(" << fontname[glut_bitmap_font]
    << ") \"" << textdata << "\"";
}

void ellipse::show (ostream& out) const {
    shape::show (out);
    out << "{" << dimension << "}";
}

void polygon::show (ostream& out) const {
    shape::show (out);
    out << "{" << vertices << "}";
}

ostream& operator<< (ostream& out, const shape& obj) {
    obj.show (out);
    return out;
}
