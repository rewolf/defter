#include "v_util.h"




void vec2f_subtract(vec2f * a,vec2f * b,vec2f * result){
    result->x = a->x - b->x;
    result->y = a->y - b->y;
}
void vec2f_mul(vec2f * a,float f){
    a->x*=f;
    a->y*=f;
}
float vec2f_mag(vec2f * a){
    return sqrtf(a->x*a->x + a->y*a->y);
}
void vec2f_add(vec2f * a,vec2f * b,vec2f * result){
    result->x = a->x + b->x;
    result->y = a->y + b->y;
}
void vec2f_normalise(vec2f * a){
    float d = sqrtf(a->x*a->x + a->y*a->y);
    a->x/=d;
    a->y/=d;
}

void rt_intersection(float r1,float theta1,float r2,float theta2,int *x,int *y){ // XXX better method!
    float m1 = -(cosf(theta1)/sinf(theta1));
    float c1 =  (r1/(sinf(theta1)));

    float m2 = -(cosf(theta2)/sinf(theta2));
    float c2 = (r2/(sinf(theta2)));
 //   printf("Theta 1 , 2 = %f %f\n",theta1,theta2);

    float tempx = (c2-c1)/(m1-m2);
    float tempy = m1*tempx + c1;
    *x = (int) roundf(tempx);
    *y = (int) roundf(tempy);
}

bool scmp(struct line * a,struct line * b){
    return a->accum > b->accum;
}
void rt_intersection_vec2f(float r1,float theta1,float r2,float theta2,vec2f * result){ // XXX better method!
    float m1 = -(cosf(theta1)/sinf(theta1));
    float c1 =  (r1/(sinf(theta1)));

    float m2 = -(cosf(theta2)/sinf(theta2));
    float c2 = (r2/(sinf(theta2)));
   // printf("Theta 1 , 2 = %f %f\n",theta1,theta2);

    float tempx = (c2-c1)/(m1-m2);
    float tempy = m1*tempx + c1;
    result->x = tempx;
    result->y = tempy;
}

float angle_difference(float theta1, float theta2){
    float min = fmin(theta1,theta2);
    float max = fmax(theta1,theta2);
    return fmin( fabs(theta1-theta2), fabs(min - (max - M_PI)));
}
void swap (int *a,int *b){
	int t = *a;
	*a=*b;
	*b=t;
}
void vec2f_set(vec2f * a,vec2f *b){ // a=b
    a->x=b->x;
    a->y=b->y;
}
float dist(int x0,int y0,int x1,int y1){
    int dx = x1-x0;
    int dy=y1-y0;
    return sqrtf(dx*dx + dy*dy);
}
void swap (struct line ** a,struct line ** b){
	struct line * t = *a;
	*a=*b;
	*b=t;
}

namespace morphology{
    int * open_operation(int * buf,int w,int h,int * num_found,int sx,int sy,int cw,int ch,int *cx,int *cy){ // Looping dodgy with bounding box, look at start end
        int s = 3;
        *cx=0;
        *cy=0;
        int *output = (int *) malloc(w*h*sizeof(int));
        *num_found=0;
        memset(output,0,w*h*sizeof(int));
        
        for (int y =sy+s;y < sy+ch-s;y++){
            int y_offset= w*y;
            for (int x=sx+s;x < sx+cw-s;x++){

                bool copy=true;

                if (buf[y*w + x] > 0){
                   /*for (int t = 0; t < s;t++){
                       for (int k = 0;k < s;k++){
                           int ty = y+ t-s/2;
                           int tx = x + k-s/2;
                           if (buf[ty*w + tx] == 0){
                               copy=false;
                           }
                       }
                   }*/
                    bool a,b,c,d;
                    a = buf[y*w +x+1] > 0;
                    b = buf[y*w +x-1] > 0;
                    c = buf[(y+1)*w +x] > 0;
                    d = buf[(y-1)*w +x] > 0;
                    copy = a&&b&&c&&d;
                } 

               if (copy){
                   output[y*w+x] = buf[y*w+x];
                   (*cx)=*cx+x;
                   (*cy)=*cy+y;
                   (*num_found)++;
               }else{
                   output[y*w+x] = 0;
               }
            }

        }
        if (*num_found > 0){
            (*cx)=(*cx)/(*num_found);
            (*cy)=(*cy)/(*num_found);
        }
        return output;
    }
};
