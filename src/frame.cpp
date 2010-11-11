#include "frame.h"
#define COEFF1 0.03141592653f       // M_PI/100.0f

frame::frame(char * b,int w,int h){
    width = w;
    height = h;
    buffer=b;
    accum=NULL;

    drho = 1;
    dtheta=M_PI/180.f;
    float irho= 1./drho;

    float ang;
    int n;

    numangle = (int) roundf(M_PI / dtheta);
    numrho = (int) roundf(((width + height) * 2 + 1) / drho);
    sort_buf = new int[numangle*numrho];
    tabsin = new float[numangle];
    tabcos = new float[numangle];

    printf("AAAAAAAAA angle, rho %i %i\n",numangle,numrho);

    for( ang = 0,  n = 0; n < numangle; ang += dtheta, n++ )
    {
        tabsin[n] = (float)(sinf(ang) * irho);
        tabcos[n] = (float)(cosf(ang) * irho);
    }
    
    int s = width*height*sizeof(int);
    accum =  (int *) malloc((numangle+2) * (numrho+2) * sizeof(int)); // r , theta parameters
    op = (int *) malloc(s); // XXX don't forget to free
    y_thresh=60;
}

int frame::widthclip(int x){
    if (x <0)
        return 0;
    if (x >= width)
        return width-1;
    return x;
}
int frame::heightclip(int y){
    if (y <0)
        return 0;
    if (y >= height)
        return height-1;
    return y;
}
inline int frame::get_off(int x,int y){
    return ((y*width + x)/2)*4;
}

bool frame::score_func(float r1,float t1,float r2,float t2){
    float dr = r1-r2;
//    float dt=t1-t2;
    float dt = angle_difference(t1,t2);
    float t = sqrtf(dr*dr + dt*dt);
    //return false;
    if (t < 15)
        return true;
    return false;
}


void frame::mark_r(int x,int y){
    if (x >= width || x < 0 || y < 0 || y>=height)
        return;
    set_r(x,y,255);
    set_g(x,y,0);
    set_b(x,y,0);
}

void frame::mark_g(int x,int y){
    if (x >= width || x < 0 || y < 0 || y>=height)
        return;
    set_r(x,y,125);
    set_g(x,y,125);
    set_b(x,y,100);
}
int frame::get_normalised_edge(int x,int y){
    return (op[width*y + x]*255 )/edge_max;
}
int * frame::compute_canny(){
    //int s = width*height*sizeof(int);
   // int * op = (int *) malloc(s); // XXX don't forget to free
//    int * blurred = (int*)malloc(s);
    
  /*  int max_r = (int)sqrt(width*width+height*height);
    max=0;
    int gaussian[5][5] = {{2,4,5,4,2},{4,9,12,9,4},{5,12,15,12,5},{4,9,12,9,4},{2,4,5,4,2}};
    for (int y=0;y < height;y+=1){
        for (int x=0;x < width;x+=1){
            int gsum = 0;
            for (int oy=-2;oy < 3;oy++){
                for (int ox=-2;ox < 3;ox++){
                    int tx = widthclip(x+ox);
                    int ty = heightclip(y+oy);
                    int val = get_y(tx,ty);
                    gsum+=gaussian[oy+2][ox+2]*val;
                }
            }
            blurred[width*y + x] = (int) (gsum/159.f);
        }
    }
*/
    int max_r = (int)sqrt(width*width+height*height);
    line_accum.clear(); // XXX free the memory first
    edge_max=0;
    max = 0;
    memset(accum,0,(numangle+2) * (numrho+2) * sizeof(int));
    float s1[][3]={{-1,-2,-1},{0,0,0},{1,2,1}};
    float s2[][3] ={{-1,0,1},{-2,0,2},{-1,0,1}};
            int minr=10000,maxr=0;
    for (int y=1;y < height-1;y++){
        for (int x=1;x < width-1;x++){
            int sum1 = 0;
            int sum2 = 0;
            int width_offset = width*y;
            bool on_border=false;
            bool off_border=false;
            for (int oy=-1;oy < 2;oy++){
                for (int ox=-1;ox < 2;ox++){
                    int tx = x + ox;//;widthclip(x+ox);
                    int ty = y+oy;////heightclip(y+oy);
                    int val = get_y(tx,ty);//blurred[width*ty + tx];
                    sum1+=s1[oy+1][ox+1]*val;
                    sum2+=s2[oy+1][ox+1]*val;
                    if (val < y_thresh){
                        on_border=true;
                    }
                    else{
                        off_border=true;
                    }

                }
            }
            int s_val=0;
           // int asum1=abs(sum1);
          //  int asum2=abs(sum2);
            int asum1=abs(sum1);
            int asum2=abs(sum2);
            op [width_offset + x] = asum1+asum2;
            if ((asum1+asum2) > edge_max){
                edge_max=asum1+asum2;
            }

            int r;
 //           printf("ANGLE offset %i\n",angle_offset);
            if (on_border && off_border ){
                if (asum1+asum2 > 40){
                    float angle;
                    if (sum1 == 0){
                        angle = 0;
                    }
                    else if (sum2==0){
                        angle = 1.5707963f;
                    }
                    else{
                        angle = atanf(((float)sum1)/((float)sum2));
                    }
                    if (angle < 0.0f ){
                       angle+=M_PI;
                    }
                    int angle_offset = (int) (angle/M_PI * (float)numangle);
                    int temp;
                    for(int n = angle_offset-10; n < angle_offset+10; n++){
                        temp = n;
                        if (n<0){
                            temp+=numangle;
                        }
                        else if (n >= numangle){
                            temp-=numangle;
                        }
                        //printf("newn = %i\n",n);
                        r = (int) ( x * tabcos[temp] + y * tabsin[temp] );
                        r += (numrho - 1) / 2;
                        accum[(temp+1) * (numrho+2) + r+1]++;
                        int t = accum[(temp+1) * (numrho+2) + r+1];
                        if (t > max)
                            max=t;
                    }
                }
            }
                        /*
            if (op [width_offset + x] > 255)
                op[width_offset+x]=255;*/
        }
    }
    int threshold=100;
    int total=0;
                float fr,ft;
                std::vector<int> sb;
    //printf("MMMMMMMMMMMMMM %i %i\n",minr,maxr);
    for(int r = 0/*470*/; r <numrho/* 1920*/; r++ )
        for(int n = 0; n < numangle; n++ )
        {
            int base = (n+1) * (numrho+2) + r+1;
            if( accum[base] > threshold &&
                accum[base] > accum[base - 1] && accum[base] >= accum[base + 1] &&
                accum[base] > accum[base - numrho - 2] && accum[base] >= accum[base + numrho + 2] ){
                fr = (r - (numrho - 1)*0.5f) * drho;
                ft = n * dtheta;
                struct line *l = new struct line;
                l->r = fr;
                l->theta=ft;
                l->accum = accum[base];
                line_accum.push_back(l);

                //printf("Line r,t = %f %f\n",fr,ft);
                sort_buf[total++] = base;
                sb.push_back(base);
            }
        }

    std::sort(line_accum.begin(),line_accum.end(),&scmp);

    return op;
}

void frame::search_box(int x,int y,int w,bool init){
    //printf("Box searching (%i %i) %i \n",x,y,w);
    compute_sobel(x,y,w,w,init);
}
void frame::whole_search(){
    compute_sobel(0,0,width,height,true);
}
int * frame::compute_sobel(int startx,int starty,int boxwidth,int boxheight,bool initialise){ // init : 1 reinit, 0 dont init
    if (accum == NULL){
        accum =  (int *) malloc(n_r*n_theta*sizeof(int)); // r , theta parameters
        memset(accum,0,n_r*n_theta*sizeof(int));
    }
    else if (initialise){
        free(accum);
        accum =  (int *) malloc(n_r*n_theta*sizeof(int)); // r , theta parameters
        memset(accum,0,n_r*n_theta*sizeof(int));
        memset(op,0,width*height*sizeof(int));
        line_accum.clear(); // XXX free the memory first
    }
    int max_r = (int)sqrt(width*width+height*height);
    max=0;

    float s1[][3]={{-1,-2,-1},{0,0,0},{1,2,1}};
    float s2[][3] ={{-1,0,1},{-2,0,2},{-1,0,1}};
    for (int y=heightclip(starty);y < heightclip(starty+boxheight);y+=1){
        for (int x=widthclip(startx);x < widthclip(startx + boxwidth);x+=1){
            int sum1 = 0;
            int sum2 = 0;
            bool on_border=false;
            bool off_border=false;
            for (int oy=-1;oy < 2;oy++){
                for (int ox=-1;ox < 2;ox++){
                    int tx = widthclip(x+ox);
                    int ty = heightclip(y+oy);
                    //int val = get_y(tx,ty);//get_y(tx,ty) + get_u(tx,ty) + get_v(tx,ty);
                    //int val = get_r(tx,ty) + get_g(tx,ty) + get_b(tx,ty);
                    int val = get_y(tx,ty);
                    if (val < y_thresh){
                        on_border=true;
                    }
                    else{
                        off_border=true;
                    }
                    sum1+=s1[oy+1][ox+1]*val;
                    sum2+=s2[oy+1][ox+1]*val;
                }
            }
            int asum1=abs(sum1);
            int asum2=abs(sum2);
            if ((asum1 + asum2) < 60){
                op[y*width+x] = 0;
                continue;
            }
            else if ((asum1+asum2) >= 60){
                op[y*width +x] = asum1 + asum2;
                if (op[y*width + x] > 255)
                    op[y*width + x]=255;
            }
//            printf("s12 = %i %i\n",sum1,sum2);
            if (!(on_border)){
                continue;
            }

            // HU HU HU HU HU HOUGH TANSFORM

            float angle;
            if (sum1 == 0){
                angle = 0;
            }
            else if (sum2==0){
                angle = 1.5707963f;
            }
            else{
                angle = atanf(((float)sum1)/((float)sum2));
            }
           if (angle < 0.0f ){
               angle+=M_PI;
           }
            int angle_offset = (int) (angle/M_PI * (float)n_theta);


            for (int t = 0 ; t < 3;t++){
                float theta = angle + ((float)(t-1))*(M_PI/(float)n_theta);
                if (theta < 0){
                    theta+=M_PI;
                }
                if (theta > M_PI){
                    theta-=M_PI;
                }

                float r = x*cosf(theta) + y*sinf(theta);
                int r_off = ((r + max_r)/(2*max_r))*n_r;
                int theta_off = (int) roundf((float)n_theta*theta/M_PI);//angle_offset + (t-4);
                accum[r_off*n_theta + theta_off]++;
                int av = accum[r_off*n_theta + theta_off];

                if (av > max){
                    max=av;
                }

                if (av > 20){
                    bool found=false;
                    for (int i=0;i < line_accum.size();i++){
                        struct line *l = line_accum[i];

                        if (score_func(r,theta,l->r,l->theta) ){ // same locality
                            found = true;
                            int bestr_off = ((l->r + max_r)/(2*max_r))*n_r;
                            int besttheta_off = (int) roundf((float)n_theta*l->theta/M_PI);//angle_offset + (t-4);
                            
                            if (av > accum[bestr_off*n_theta + besttheta_off]){
                                l->r=r;
                                l->theta=theta;
                            }
                            break;
                        }
                    }

                    if (!found){
                        struct line *nl = (struct line *) malloc(sizeof(struct line));
                        nl->r= r;
                        nl->theta=theta;
                        line_accum.push_back(nl);
                        
                    }
                }
            }
        }
    }
    return op;
}
unsigned int frame::get_y(int x,int y){
    int k = get_off(x,y) ;
    k+= ((width*y + x)&1)*2; // XXX Possible optimisation
    return (unsigned int) buffer[k]&255;
}
unsigned int frame::get_u(int x,int y){
    int k = get_off(x,y)  +1;
    return (unsigned int) buffer[k]&255;
}
unsigned int frame::get_v(int x,int y){
    int k = get_off(x,y) + 3;
    return (unsigned int) buffer[k]&255;
}
unsigned int frame::get_r(int x,int y){
    int C = get_y(x,y) - 16;
    int E = get_v(x,y) - 128; 
    int r =  (298*C + 409*E +128)>>8;
    if (r < 0)
        return 0;
    return r > 255? 255: r;
//    return (unsigned int)buffer[get_off(x,y)]&255;
}
unsigned int frame::get_g(int x,int y){
    int C = get_y(x,y) - 16;
    int D = get_u(x,y) -128;
    int E = get_v(x,y) - 128;
    //printf("%i %i %i\n",C,D,E);
    int g =  (298*C -100*D -208*E + 128)>>8; 
    if (g < 0)
        return 0;
    return g>255? 255:g;

    //return (unsigned int) buffer[get_off(x,y) + 1]&255;
}
unsigned int frame::get_b(int x,int y){
    int C = get_y(x,y) - 16;
    int D = get_u(x,y) -128;
    //return (unsigned int) buffer[get_off(x,y) + 2]&255;
    int b =  (298*C + 516*D + 128)>>8;
    if (b < 0)
        return 0;
    return b > 255? 255: b;
}

void frame::set_r(int x,int y,char val){
    buffer[get_off(x,y)] = val;
}
void frame::set_g(int x,int y,char val){
    buffer[get_off(x,y) + 1] = val;
}
void frame::set_b(int x,int y,char val){
    buffer[get_off(x,y) + 2] = val;
}
bool frame::in_bounds(int x,int y){
    if (x >= width || y >= height || x <0 || y <0 )
        return false;
    return true;
}

