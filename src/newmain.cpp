#include "monitor.h"
#include "video_driver.h"
#include "frame.h"
#include "l_rec.h"
#include "m_rec.h"
#include "v_util.h"
#include "h_detect.h"
#include <SDL/SDL.h>
#include <SDL/SDL_compat.h>
#include <iostream>
#include <opencv/cv.h>

using namespace std;
    int vidwidth;
    int vidheight;

void big_dot(SDL_Surface *s,int x,int y);
void rect (SDL_Surface * s,int x,int y, int w,int h,int r,int g,int b){
    Uint32 color = SDL_MapRGB(s->format, r, g, b);
    for (int j=0;j < h;j++)
        for (int i =0;i < w;i++){
              Uint8 * bufp = (Uint8 *)s->pixels + (y+j)*s->pitch + (x + i) * 3;
                    if(SDL_BYTEORDER == SDL_LIL_ENDIAN)
                    {
                      bufp[0] = color;
                      bufp[1] = color >> 8;
                      bufp[2] = color >> 16;
                    } else {
                      bufp[2] = color;
                      bufp[1] = color >> 8;
                      bufp[0] = color >> 16;
                    }

    }
}
void render(SDL_Surface * surface,int offx,int offy,frame & f){
               bool do_sync = false;
    for (int y  =0;y < f.height;y++){
        for (int x = 0;x < f.width;x++){
//              Uint32 cam_val = (Uint32) ( (Uint8 *) buffers[0].start)[y*288 + x];
       //       Uint8 * base = &((Uint8 *) vd.buffers[0].start)[(y*352 + x)*3];
              //printf ("%i %i %i\n",R,G,B);
              //printf ("%i\n",cam_val);
              Uint32  color;
                    //color = SDL_MapRGB(surface->format, R, G, B);
                    if (f.get_y(x,y) < f.y_thresh){
                    color = SDL_MapRGB(surface->format, f.get_y(x,y), f.get_y(x,y), f.get_y(x,y));
                    }
                    else{
                    color = SDL_MapRGB(surface->format, 255, 255, 255);
                    }
                       Uint8 *bufp;
                    bufp = (Uint8 *)surface->pixels + (y+offy)*surface->pitch + (x + offx) * 3;
                    if(SDL_BYTEORDER == SDL_LIL_ENDIAN)
                    {
                      bufp[0] = color;
                      bufp[1] = color >> 8;
                      bufp[2] = color >> 16;
                    } else {
                      bufp[2] = color;
                      bufp[1] = color >> 8;
                      bufp[0] = color >> 16;
                    }

        }

    }
}
void draw_line (SDL_Surface * surface,int offx,int offy, float theta,float r,int w,int h){
    //theta = -theta + M_PI;
    float m = -(cosf(theta)/sinf(theta));
   // theta = -(theta - M_PI);
    int c = (int) (r/(sinf(theta)));
    //printf("R,T %f %f\n",r,360.f*theta/(2*M_PI));
    //printf("M,C  %f %i \n",m,c);
    
        for (int x = 0;x < w;x++){
              Uint32  color;
                    color = SDL_MapRGB(surface->format, 0, 0,255);
                    Uint8 *bufp;
                    if ((m*x + c)<0 || (m*x + c) > h)
                        continue;
     //                bufp = (Uint8 *)surface->pixels + ((int)(m*x + c)+offy  )*surface->pitch + (x + offx) * 3;
                    big_dot(surface,x+offx,(m*x + c)+offy ); 
                    continue;
                    if(SDL_BYTEORDER == SDL_LIL_ENDIAN){
                      bufp[0] = color;
                      bufp[1] = color >> 8;
                      bufp[2] = color >> 16;
                    } else {
                      bufp[2] = color;
                      bufp[1] = color >> 8;
                      bufp[0] = color >> 16;
                    }

        }

}
void draw_line2 (SDL_Surface * surface,int offx,int offy, float theta,float r,int w,int h){
    //theta = -theta + M_PI;
    float m = -(cosf(theta)/sinf(theta));
   // theta = -(theta - M_PI);
    int c = (int) (r/(sinf(theta)));
    //printf("R,T %f %f\n",r,360.f*theta/(2*M_PI));
    //printf("M,C  %f %i \n",m,c);
    
        for (int x = 0;x < w;x++){
              Uint32  color;
                    color = SDL_MapRGB(surface->format, 0, 255,0);
                    Uint8 *bufp;
                    if ((m*x + c)<0 || (m*x + c) > h)
                        continue;
                     bufp = (Uint8 *)surface->pixels + ((int)(m*x + c)+offy  )*surface->pitch + (x + offx) * 3;
                    if(SDL_BYTEORDER == SDL_LIL_ENDIAN){
                      bufp[0] = color;
                      bufp[1] = color >> 8;
                      bufp[2] = color >> 16;
                    } else {
                      bufp[2] = color;
                      bufp[1] = color >> 8;
                      bufp[0] = color >> 16;
                    }

        }

}
void draw_line3 (SDL_Surface * surface,int offx,int offy, float theta,float r,int w,int h){
    //theta = -theta + M_PI;
    float m = -(cosf(theta)/sinf(theta));
   // theta = -(theta - M_PI);
    int c = (int) (r/(sinf(theta)));
    //printf("R,T %f %f\n",r,360.f*theta/(2*M_PI));
    //printf("M,C  %f %i \n",m,c);
    
        for (int x = 0;x < w;x++){
              Uint32  color;
                    color = SDL_MapRGB(surface->format,  255,0,0);
                    Uint8 *bufp;
                    if ((m*x + c)<0 || (m*x + c) > h)
                        continue;
                     bufp = (Uint8 *)surface->pixels + ((int)(m*x + c)+offy  )*surface->pitch + (x + offx) * 3;
                    if(SDL_BYTEORDER == SDL_LIL_ENDIAN){
                      bufp[0] = color;
                      bufp[1] = color >> 8;
                      bufp[2] = color >> 16;
                    } else {
                      bufp[2] = color;
                      bufp[1] = color >> 8;
                      bufp[0] = color >> 16;
                    }

        }

}
void accum_render(SDL_Surface * surface,int offx,int offy,frame & f,int * accum,int m, m_rec & mon_rec ){
               bool do_sync = false;
               if (m==0){
                   printf("Nothing to display\n");
                   return;
               }
               printf("Max is %i\n",m);
    goto lines;
    for (int y  =0;y < f.numrho/20;y++){
        for (int x = 0;x < f.numangle/20;x++){
              Uint32  color;
                    //int val = accum[y*f.numangle + x];
                    int val = accum[(x+1) * (f.numrho+2) + y+1];
                
                  //  val*=255;
                   // val/=m;
                    if (val ==0){
     //               color = SDL_MapRGB(surface->format, 0, 0, 0);
                    color = SDL_MapRGB(surface->format, val, val, val);
                    }
                    else{
                //    draw_line(surface,100,100,M_PI*(float)x/(float)f.n_theta,((float)y /(float)f.n_r) * 1600.0f - 800.0f,640,480);
                    val*=255;
                    val/=m;
                    color = SDL_MapRGB(surface->format, val, val, val);
                    }
                       Uint8 *bufp;
                    bufp = (Uint8 *)surface->pixels + (x+offy)*surface->pitch + (y + offx) * 3;
                    if(SDL_BYTEORDER == SDL_LIL_ENDIAN)
                    {
                      bufp[0] = color;
                      bufp[1] = color >> 8;
                      bufp[2] = color >> 16;
                    } else {
                      bufp[2] = color;
                      bufp[1] = color >> 8;
                      bufp[0] = color >> 16;
                    }

        }

    }
lines:
    bool done = false;
    vector <struct pair2 *> plells;
    Uint32 color = SDL_MapRGB(surface->format, 255, 0, 0);
    int max_r = (int)sqrt(f.width*f.width+f.height*f.height);
    for (int i=0;i < min(45,(int)f.line_accum.size());i++){
        struct line *l = f.line_accum[i];
                  draw_line2(surface,100,300,l->theta,l->r,f.width,f.height);
                  continue;
                        int r_off = ((l->r + max_r)/(2*max_r))*f.n_r;
                        int theta_off = (int) roundf((float)f.n_theta*l->theta/M_PI);//angle_offset + (t-4);
                       Uint8 *bufp;
                    bufp = (Uint8 *)surface->pixels + (r_off+offy)*surface->pitch + (theta_off + offx) * 3;
                    if(SDL_BYTEORDER == SDL_LIL_ENDIAN)
                    {
                      bufp[0] = color;
                      bufp[1] = color >> 8;
                      bufp[2] = color >> 16;
                    } else {
                      bufp[2] = color;
                      bufp[1] = color >> 8;
                      bufp[0] = color >> 16;
                    }
    }
    printf("=================================================================\n");
}
void sobel_render(SDL_Surface * surface,int offx,int offy,frame & f){
               bool do_sync = false;
            int * sob = f.op;//f.compute_sobel(true);
            int em = f.edge_max;
    for (int y  =0;y < f.height;y++){
        for (int x = 0;x < f.width;x++){
            int val = sob[f.width*y + x];
            val*=255;
            val/=em;
            if (val > 15){
                val=255;
            }
            else
                val=0;
              Uint32  color;
                    color = SDL_MapRGB(surface->format, val, val, val);
                       Uint8 *bufp;
                    bufp = (Uint8 *)surface->pixels + (y+offy)*surface->pitch + (x + offx) * 3;
                    if(SDL_BYTEORDER == SDL_LIL_ENDIAN)
                    {
                      bufp[0] = color;
                      bufp[1] = color >> 8;
                      bufp[2] = color >> 16;
                    } else {
                      bufp[2] = color;
                      bufp[1] = color >> 8;
                      bufp[0] = color >> 16;
                    }

        }

    }
}
void sobel_render2(SDL_Surface * surface,int offx,int offy,frame & f, int * edge){
               bool do_sync = false;
            int * sob = f.op;//f.compute_sobel(true);
    for (int y  =0;y < f.height;y++){
        for (int x = 0;x < f.width;x++){
            int val = edge[f.width*y + x];
              Uint32  color;
                    color = SDL_MapRGB(surface->format, val, val, val);
                       Uint8 *bufp;
                    bufp = (Uint8 *)surface->pixels + (y+offy)*surface->pitch + (x + offx) * 3;
                    if(SDL_BYTEORDER == SDL_LIL_ENDIAN)
                    {
                      bufp[0] = color;
                      bufp[1] = color >> 8;
                      bufp[2] = color >> 16;
                    } else {
                      bufp[2] = color;
                      bufp[1] = color >> 8;
                      bufp[0] = color >> 16;
                    }

        }

    }
}

void show_lrec(SDL_Surface * surface,int offx,int offy,l_rec & lr,int t){
    if (lr.state == 0)
        return;
    for (int y  =0;y < lr.l_frame->height;y++){
        for (int x = 0;x < lr.l_frame->width;x++){

            Uint32  color;
            if (lr.recog_buffer[lr.l_frame->width*y + x] == 1){
                color = SDL_MapRGB(surface->format, t, t, t);
            }
            else{
                continue;
            }
            Uint8 *bufp;
            bufp = (Uint8 *)surface->pixels + (y+offy)*surface->pitch + (x + offx) * 3;
            if(SDL_BYTEORDER == SDL_LIL_ENDIAN)
            {
              bufp[0] = color;
              bufp[1] = color >> 8;
              bufp[2] = color >> 16;
            } else {
              bufp[2] = color;
              bufp[1] = color >> 8;
              bufp[0] = color >> 16;
            }

        }

    }
}
void vline(SDL_Surface * s,int x){
    x+=100; 
    Uint32 color = SDL_MapRGB(s->format, 0, 255, 0);
    for (int ty=100;ty < vidheight + 100;ty++){ 
        Uint8 * bufp = (Uint8 *)s->pixels + ty*s->pitch +x * 3;
        bufp[0] = color;
        bufp[1] = color >> 8;
        bufp[2] = color >> 16;
        }
    }
void hline(SDL_Surface * s,int y){
    y+=100;
    Uint32 color = SDL_MapRGB(s->format, 0, 255, 0);
    for (int tx=100;tx < vidwidth + 100;tx++){ 
        Uint8 * bufp = (Uint8 *)s->pixels + y*s->pitch +tx * 3;
        bufp[0] = color;
        bufp[1] = color >> 8;
        bufp[2] = color >> 16;
        }
    }
void dot(SDL_Surface *s,int x,int y){
    Uint32 color = SDL_MapRGB(s->format, 0, 255, 0);
    for(int tx=x-1;tx <= x+1;tx++){
        for (int ty=y-1;ty <= y+1;ty++){ 
        Uint8 * bufp = (Uint8 *)s->pixels + ty*s->pitch + tx * 3;
        bufp[0] = color;
        bufp[1] = color >> 8;
        bufp[2] = color >> 16;
        }
    }
}
void gdot(SDL_Surface *s,int x,int y,int size,int r,int g,int b){
    Uint32 color = SDL_MapRGB(s->format, r, g, b);
    for(int tx=x-size;tx <= x+size;tx++){
        for (int ty=y-size;ty <= y+size;ty++){ 
        Uint8 * bufp = (Uint8 *)s->pixels + ty*s->pitch + tx * 3;
        bufp[0] = color;
        bufp[1] = color >> 8;
        bufp[2] = color >> 16;
        }
    }
}
void gdot2(frame & f,int x,int y,int size){
    for(int tx=x-size;tx <= x+size;tx++){
        for (int ty=y-size;ty <= y+size;ty++){ 
            f.mark_r(tx,ty);
        }
    }
}
void big_dot(SDL_Surface *s,int x,int y){
    Uint32 color = SDL_MapRGB(s->format, 0, 255, 0);
    for(int tx=x-2;tx <= x+2;tx++){
        for (int ty=y-2;ty <= y+2;ty++){ 
        Uint8 * bufp = (Uint8 *)s->pixels + ty*s->pitch + tx * 3;
        bufp[0] = color;
        bufp[1] = color >> 8;
        bufp[2] = color >> 16;
        }
    }
}
void fill (Uint32 color,SDL_Surface * s,int w,int h){
    for (int y  =0;y < h;y++){
        for (int x = 0;x < w;x++){
            Uint8 * bufp = (Uint8 *)s->pixels + y*s->pitch + x * 3;
            bufp[0] = color;
            bufp[1] = color >> 8;
            bufp[2] = color >> 16;
        }
    }
}
void line(SDL_Surface * s,int sx,int sy,int ex,int ey){
    int np = 150;
    for (int i = 0;i < np;i++){
        dot(s,sx + i *(ex-sx)/np,sy + i * (ey-sy)/np);
    }
}
void screen_ren(int offx,int offy,SDL_Surface *s, m_rec & m,frame & f){
    int max_height, max_width;
    int diff1 = m.bottom_left[0] - m.top_left[0];
    int diff2 = m.bottom_left[1] - m.top_left[1];
    int diff3 = m.bottom_right[0] - m.top_right[0];
    int diff4 = m.bottom_right[1] - m.top_right[1];
    max_height = max(m.bottom_left[1] - m.top_left[1],m.bottom_right[1] - m.top_right[1]);
    max_width = max (m.top_right[0] - m.top_left[0],m.bottom_right[0] - m.bottom_left[0]);
    max_height = 480;
    max_width = 768;
    for (int y=0;y < max_height;y++){
        int lx,ly;
        int rx,ry;
        lx = m.top_left [0] + y*(diff1)/max_height;
        ly =m.top_left[1] + y*(diff2)/max_height;
        rx = m.top_right[0] + y*(diff3)/max_height;
        ry = m.top_right[1] + y * (diff4)/max_height;
        for(int x=0;x < max_width;x++){
            int tx = lx + x * (rx - lx)/max_width;
            int ty = ly + x * (ry-ly)/max_width;
            Uint32 color = SDL_MapRGB(s->format, f.get_r(tx,ty), f.get_g(tx,ty), f.get_b(tx,ty));
            Uint8 * bufp = (Uint8 *)s->pixels + (offy + y)*s->pitch + (x + offx) * 3;
            bufp[0] = color;
            bufp[1] = color >> 8;
            bufp[2] = color >> 16;
        }
    }
}
void draw_orientation(SDL_Surface * s,monitor & mon){
    int o_size = 50;
    int side_l = 400;
    int height_l = 200;
    int SURFACE_HEIGHT = mon.out_height;
    int SURFACE_WIDTH = mon.out_width;

    rect(s,0,0,side_l,o_size,0,255,0);
    rect(s,0,0,o_size,height_l,0,255,0);

    rect(s,0,SURFACE_HEIGHT-o_size,side_l,o_size,0,255,0);
    rect(s,0,SURFACE_HEIGHT-height_l,o_size,height_l,0,255,0);

    rect(s,SURFACE_WIDTH-side_l,0,side_l,o_size,0,255,0);
    rect(s,SURFACE_WIDTH-o_size,0,o_size,height_l,0,255,0);

    rect(s,SURFACE_WIDTH-side_l,SURFACE_HEIGHT-o_size,side_l,o_size,0,255,0);
    rect(s,SURFACE_WIDTH-o_size,SURFACE_HEIGHT-height_l,o_size,height_l,0,255,0);
}


int main(int argc,char ** argv){

     bool prod = true;

    int y_thresh = 60;
    if ( argc == 2){
        y_thresh = atoi(argv[1]);
    }
    else if (argc == 4){
        y_thresh=atoi(argv[1]);
        vidwidth=atoi(argv[2]);
        vidheight=atoi(argv[3]);
    }
    else if (argc == 5){
        y_thresh=atoi(argv[1]);
        vidwidth=atoi(argv[2]);
        vidheight=atoi(argv[3]);
        prod=false;
    }
    monitor mon(1920,1138);
   // monitor mon(800,600);

    video_driver vid(vidwidth,vidheight);
    vid.open_device();
    vid.init_device();
    vid.start_capturing();
    frame f((char*)vid.buffers[0].start,vidwidth,vidheight);
    printf("Y thresh %i \n",y_thresh);
    f.y_thresh = y_thresh;
    printf("vid.fd = %i\n",vid.fd);
    int cnt = 0;

    bool done=false;
    bool check = false;
   SDL_Event event;
    SDL_Surface * outs = SDL_SetVideoMode( mon.out_width, mon.out_height, 0, SDL_HWSURFACE|SDL_DOUBLEBUF|SDL_HWPALETTE);
    SDL_Overlay * yuv = SDL_CreateYUVOverlay(f.width,f.height,SDL_YUY2_OVERLAY,outs);
    SDL_Overlay * yuv2 = SDL_CreateYUVOverlay(10,10,SDL_YUY2_OVERLAY,outs);
        SDL_Rect sr;
        sr.x = 100;
        sr.y=100;
        sr.w = f.width;
        sr.h=f.height;
    SDL_Rect sr2;
    sr2.w=20;
    sr2.h=20;
    char * test = (char*)malloc(1000);
    memset(test,255,1000);

     int grey = SDL_MapRGB(outs->format, 80, 125, 70);
     m_rec monitor_rec(f);
     h_detect hand(&f,&monitor_rec);
     double screen_corners[4][2] = {{0.,0.},{1920.,0.},{1920.,1200.},{0.,1200.}};
     int frmcnt=0;





     //printf("%f %f %f\n %f %f %f\n%f %f %f\n");
     //abc[0][0]=1.0f;



            fill(grey,outs,mon.out_width,mon.out_height);
        SDL_Flip(outs);
   while(!done){
     while ( SDL_PollEvent(&event) ){ //Check for events
       if ( event.type == SDL_QUIT ) //Check if the 'x' button has been pressed
         { done = 1; }
     }
        cap:
        int d = vid.read_frame();
     /*   if (d ==0){

         //   usleep(10);
            goto cap;
        }*/
        if (d==0){
            continue;
        }
        f.buffer = (char*)vid.buffers[vid.last_index].start;
//        printf("Last index %i\n",vid.last_index);
        if (d< 0){
            printf("Restarting Video\n");
            vid.restart_video();
            f.buffer = (char*)vid.buffers[vid.last_index].start;
            goto cap;
        }
        cnt++;
        frmcnt++;
       printf("==================== frame %i\n",frmcnt);
       if (cnt==200){ 
        fill(grey,outs,mon.out_width,mon.out_height);
        SDL_Flip(outs);
        cnt=0;
        continue;
       }
        if (prod){
        f.compute_canny();
        //goto ren;
        if (monitor_rec.find_monitor()){
            
                gdot2(f,monitor_rec.ptl.x,monitor_rec.ptl.y,4);
                gdot2(f,monitor_rec.ptr.x,monitor_rec.ptr.y,4);
                gdot2(f,monitor_rec.pbl.x ,monitor_rec.pbl.y,4);
                gdot2(f,monitor_rec.pbr.x ,monitor_rec.pbr.y,4);
            //goto ren;

                if (monitor_rec.hand_detected){
          //          printf("Got a Hand!\n");
                    //goto ren;
                    hand.hand_search(monitor_rec.handx,monitor_rec.handy,f.op);
                    gdot2(f,monitor_rec.handx,monitor_rec.handy - 8,2);
                    int realx,realy;
                    for (int i=0;i < hand.fingertips.size();i++){
                        monitor_rec.transform_xy(hand.fingertips[i]->x,hand.fingertips[i]->y,&realx,&realy);
                        struct fingertip * ft= hand.fingertips[i];
            //            printf("Winner cval = %f\n",ft->cval);
                        gdot2(f,ft->x,ft->y,4);
                        sr2.x=realx;
                        sr2.y=realy-70;
                        SDL_LockYUVOverlay(yuv2);
                        memcpy(*(yuv2->pixels),test,100*2);
                        SDL_UnlockYUVOverlay(yuv2);
                        SDL_DisplayYUVOverlay(yuv2,&sr2);
                    }
                //gdot2(f,hand.palmx,hand.palmy,4);
                   // sr2.x =(sr2.x +realx)/2;
                   // sr2.y=(sr2.y + (realy-70))/2;

                }
                //SDL_Flip(outs);
        }
ren:
            SDL_LockYUVOverlay(yuv);
            memcpy(*(yuv->pixels),f.buffer,f.width*f.height*2);
            SDL_UnlockYUVOverlay(yuv);
            SDL_DisplayYUVOverlay(yuv,&sr);


//               accum_render(outs,100,800,f,f.accum,f.max,monitor_rec);
       //     sobel_render(outs,800,300,f);
         //printf("Max accum value: %i\n",f.max);
         //printf("Line accum size() %i\n",(int)f.line_accum.size());

        }
        else{
            f.compute_canny();
            fill(grey,outs,mon.out_width,mon.out_height);
            sobel_render(outs,100,300,f);
            //accum_render(outs,800,100,f,f.accum,f.max,monitor_rec);
            SDL_Flip(outs);

        }
                   

 
   }
 
   SDL_Quit();
}
