#ifndef UTIL_H
#define UTIL_H
#include <math.h>
#include <string.h>
#include <stdio.h>
#include "frame.h"
#include <stdlib.h>

typedef struct vec2f {float x,y;} vec2f;

void vec2f_subtract(vec2f * a,vec2f * b,vec2f * result);
void vec2f_add(vec2f * a,vec2f * b,vec2f * result);
void vec2f_normalise(vec2f * a);
void vec2f_mul(vec2f * a,float f);
float vec2f_mag(vec2f * a);
void rt_intersection(float r1,float theta1,float r2,float theta2,int *x,int *y);
void rt_intersection_vec2f(float r1,float theta1,float r2,float theta2,vec2f * result); // XXX better method!
void vec2f_set(vec2f * a,vec2f *b);
void swap(int *a,int * b);
void swap (struct line ** a,struct line ** b);
bool scmp(struct line * a,struct line * b);
float angle_difference(float theta1, float theta2);
float dist(int x0,int y0,int x1,int y1);
namespace morphology{

    int * open_operation(int * buf,int w,int h,int * num_out,int startx,int starty,int cw,int ch,int * cx,int *cy);
};

#endif 
