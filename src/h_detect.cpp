#include "h_detect.h"
#include <stdio.h>

h_detect::h_detect(frame * f,m_rec * mr){
    m = mr;
    l_frame = f;
    
    buffer_size = l_frame->width*l_frame->height*sizeof(int);//f->width*f->height*sizeof(short); // XXXXX FIX
    recog_buffer = (int *) malloc(buffer_size);
    memset(recog_buffer,0,buffer_size);
    tmpcnt=0;
    
}

void h_detect::flood_fill_rec2(int x,int y,int y_val){

        int change_thresh = 3;
     //   printf("%i %i\n",x,y);

        if (! l_frame->in_bounds(x,y) || y > init_y){
            return;
        }
       
        if (recog_buffer[l_frame->width*y + x] != 0){
            return;
        }

        if (y > max_y){
            max_y= y;
        }
        if (y < min_y){
            min_y = y;
        }
        if (x > max_x){
            max_x=x;
        }
        if (x < min_x){
            min_x=x;
        }
        recog_buffer[l_frame->width*y + x] = 1;
       // edge_buffer[ebw*y + x] = 50;

        int nv;

        nv = l_frame->get_y(x+1,y);
        if (abs(y_val - nv) < change_thresh)
           flood_fill_rec2(x+1,y,nv);
        else{
            if (l_frame->in_bounds(x+1,y))
                recog_buffer[l_frame->width*(y) + (x+1)] = 2;
        }

        nv = l_frame->get_y(x-1,y);
        if (abs(y_val - nv) < change_thresh)
           flood_fill_rec2(x-1,y,nv);
        else{
            if (l_frame->in_bounds(x-1,y))
            recog_buffer[l_frame->width*(y) + (x-1)] = 2;
        }

        nv = l_frame->get_y(x,y+1);
        if (abs(y_val - nv) < change_thresh)
           flood_fill_rec2(x,y+1,nv);
        else{
            if (l_frame->in_bounds(x,y+1))
            recog_buffer[l_frame->width*(y+1) + (x)] = 2;
        }

        nv = l_frame->get_y(x,y-1);
        if (abs(y_val - nv) < change_thresh)
           flood_fill_rec2(x,y-1,nv);
        else{
            if (l_frame->in_bounds(x,y-1))
            recog_buffer[l_frame->width*(y-1) + (x)] = 2;
        }

}
void h_detect::flood_fill_rec(int x,int y){
//    printf("x,y = %i,%i\n",x,y);

   /*if (!l_frame->in_bounds(x,y))
      return; */

    // DO the in bounds check better XXX
    if (x <0 || x>= ebw || y <0 || y>= ebh){
        return;
    }

   if (recog_buffer[l_frame->width*y + x] != 0)
       return;
   if (accept(x,y)){

       recog_buffer[l_frame->width*y + x] = 1;
       num_points++;
        if (y > max_y){
            max_y= y;
        }
        if (y < min_y){
            min_y = y;
        }
        if (x > max_x){
            max_x=x;
        }
        if (x < min_x){
            min_x=x;
        }
       flood_fill_rec(x-1,y);
       flood_fill_rec(x,y-1);
       flood_fill_rec(x+1,y);
       flood_fill_rec(x,y+1);
        
   }
   else{
     //  ptsx.push_back(x);
      // ptsy.push_back(y);
       recog_buffer[l_frame->width*y + x] = 2;
   }
}
int  h_detect::pts_off(int i){
    if (i < 0)
            return ptsx.size() + i;
    if (i > ptsx.size())
        return i - ptsx.size();
    return i;
}

float h_detect::curvature(int i, int l){
    float px = (float) ptsx[i];
    float py = (float) ptsy[i];

    float p1x = (float) ptsx[pts_off(i-l)];
    float p1y = (float) ptsy[pts_off(i-l)];

    float p2x = (float) ptsx[pts_off(i+l)];
    float p2y = (float) ptsy[pts_off(i+l)];

    float pp1x = p1x - px;
    float pp1y = p1y - py;
    float pp2x =p2x - px;
    float pp2y = p2y - py;

    float num =  (pp1x)*(pp2x) + (pp1y)*(pp2y);
    float spp1 = sqrtf(pp1x*pp1x + pp1y*pp1y);
    float spp2 = sqrtf(pp2x*pp2x + pp2y*pp2y);
    float denom = spp1*spp2;
   // printf("%f curvature\n",0.5f*num/denom);
    return (num/denom);


}
inline int h_detect::get_recog_val(int x,int y){
    return recog_buffer[ebw*y + x];
}
void h_detect::next_clockwise(int px,int py,int *cx,int *cy){
    int moore_neighbourhood[][2] = {{0,-1},{-1,-1},{-1,0},{-1,+1},{0,+1},{+1,+1},{+1,0},{+1,-1}};
    int m_lookup[3][3] ={{2,3,4},{1,100000,5},{0,7,6}};
    int ofx=*cx-px + 1;
    int ofy=*cy -py + 1;
//    printf("offx %i %i, pxpy %i %i, cx cy, %i %i \n",ofx,ofy,px,py,*cx,*cy);
    int mn = m_lookup[ofx][ofy];
    *cx = px + moore_neighbourhood[mn][0];
    *cy = py + moore_neighbourhood[mn][1];
}
void h_detect::do_moore_neighbourhood(int startx,int starty){ // assumes startx,y is 'on'
    int currx,curry;
    currx=startx;
    curry=starty;
    int lastx,lasty;
    int cnt =0;
    int cx,cy;
    int px,py;
    cx = startx;
    cy=starty;
    lastx=startx;
    lasty=starty-1;
    px=cx;
    py=cy;
    int moore_neighbourhood[][2] = {{0,-1},{-1,-1},{-1,0},{-1,+1},{0,+1},{+1,+1},{+1,0},{+1,-1}};
    int m_lookup[3][3] ={{2,1,0},{3,100000,7},{3,4,6}};
    int mn=0;
    while(true){
        cnt++;
        //printf("%i %i\n",lastx,lasty);
  //      printf("------------------------------------\n");

        if ( cx==startx&& cy == starty && cnt>1){
            //printf ("Full circuit\n");
            return;
        }
        if (cnt > 10000){
            return;
        }
        int yv = get_recog_val(cx,cy);
    //   printf("cx,cy %i %i\n",cx,cy);

        if (yv > 0){
      //      printf("Good pixel, backtracking\n");
            l_frame->mark_r(cx,cy);
            ptsx.push_back(cx);
            ptsy.push_back(cy);
            palmx+=cx;
            palmy+=cy;
            px=cx;
            py=cy;
            cx=lastx;
            cy=lasty;

        }
        else{ // goto next clockwise pixel;
            lastx=cx;
            lasty=cy;
            next_clockwise(px,py,&cx,&cy);
        //    printf("Bad pixel\n");
            //printf("Pixel is bad, going to %i %i\n",cx,cy);
        }

        //printf("Moore alg failed\n");
    }
}
void h_detect::add_fingertip(int x,int y,float cval,int c_index){ // check locality && maximality && not wrist && and onscreen
    float palm_thresh = 60.0f; // must dynamically determine this
    float finger_thresh = 15.0f;
    
    if (!m->in_bounds(x,y)){
        return ;
    }
    float wrist_dist = dist(x,y,p_startx,p_starty);
    float palm_dist = dist(x,y,palmx,palmy);
    if ( palm_dist < palm_thresh && do_palm_thresh_check){
        return;
    }
    cval=wrist_dist;
    int cont_diff=-1;
    for (int i=0;i < fingertips.size();i++){
        struct fingertip * ft = fingertips[i];
        int fx=ft->x;
        int fy=ft->y;
        float fc=ft->cval;
        int fcont = ft->contour_index;
        int fs = ptsx.size();
        int round_diff = std::abs(std::abs(c_index - fcont) - fs);
        cont_diff = std::min(std::abs(c_index - fcont),round_diff);
        //if (dist(x,y,fx,fy) < finger_thresh){
        if (cont_diff < 60){
           // printf("Same one! new is %f, old is %f\n",cval,ft->cval);
            if (cval>fc){ // this is better
            //    printf("New wins!\n");
                ft->x=x;
                ft->y=y;
                ft->cval=cval;
                ft->contour_index= c_index;
            }
            return;
            
        }
        //printf("Compared %i %i, size = %i\n",c_index,fcont,fs);

    }
    // A new fingertip!
    struct fingertip * nf = new struct fingertip;
    //printf("New fingertip: %i\n",cont_diff);
    nf->x=x;
    nf->y=y;
    nf->cval=cval;
    nf->contour_index= c_index;
    fingertips.push_back(nf);
}
int max(int a,int b){ // XXXX this is gay
    if (a > b)
        return a;
    return b;
}
bool h_detect::hand_search(int startx,int starty, int * eb){
    max_y = l_frame->height-1;
    //starty-=8;
    //startx-=8;
    max_x=0;
    ptsx.clear();
    ptsy.clear();
    hand_ptsx.clear();
    hand_ptsy.clear();
    for (int i=0;i < fingertips.size();i++){
        delete fingertips[i];
    }
    fingertips.clear();
  //  printf("Hand searching %i %i \n",startx,starty);
    min_x=max_x=startx;
    min_y=max_y=starty;
    p_startx=startx;
    p_starty=starty;

    memset(recog_buffer,0,buffer_size);
    edge_buffer = eb;
    init_y = starty;
    ebw= l_frame->width;
    ebh = l_frame->height;
    //flood_fill_rec2(startx,starty,l_frame->get_y(startx,starty));
    num_points=0;
    flood_fill_rec(startx,starty);
    if (num_points > 5000){
        do_palm_thresh_check = true;
    }
    else{
        do_palm_thresh_check = false;
    }

   // printf("Complete: max x y = %i %i\n",max_x,max_y);
    int morphx = min_x;
    int morphy = min_y;
    int morphw = max_x-min_x;
    int morphh = max_y-min_y;
    int * opened;
    int n_times=15;
   palmx=0;palmy=0; 
    int num_out;
    //int palmx,palmy;
    if (false){
        opened = morphology::open_operation(recog_buffer,ebw,ebh,&num_out,morphx,morphy,morphw,morphh,&palmx,&palmy);
        int * new_o;// = opened;
        for (int i =0;i < n_times;i++){
            new_o = morphology::open_operation(opened,ebw,ebh,&num_out,morphx,morphy,morphw,morphh,&palmx,&palmy);
            free(opened);
            opened=new_o;
            printf("Palmx,palmy %i %i\n",palmx,palmy);

        }
        free(opened);
    }

    //palmx=palmy=0;
    int tn =0;
    bool done = false;
    for (int ty=0;ty < ebh;ty++){
        if (done)
            break;
        int y_offset = ebw*ty;
        for (int tx=0;tx < ebw;tx++){
            //printf("Offset %i\n",y_offset + tx);
            if (recog_buffer[y_offset + tx] >  0){
                do_moore_neighbourhood(tx,ty);
                palmx/=ptsx.size();
                palmy/=ptsx.size();
                done=true;
                break;
            }
        }
    }
    if (tn==0)
        tn=1;
    //palmx=startx;
    //palmy=starty;
    tmpcnt+=1;
    if (tmpcnt > ptsx.size()){
        tmpcnt=0;
    }
    
    int lastx=ptsx[0];
    int lasty = ptsy[0];
    for (int i =0;i < ptsx.size() ;i+=1){
            l_frame->mark_r(ptsx[i],ptsy[i]);
        //    palmx+=ptsx[i];
         //   palmy+=ptsy[i];
    //        continue;
        bool finger = false;
        int jump=35;
        for (int k=jump;k < jump+15;k++){
            float c = curvature(i,k);
            float a = acosf(c);

            if (a < 0.5f ){ // 0.4f
                add_fingertip(ptsx[i],ptsy[i],a,i);
            }
        }
    }
    if (fingertips.size()> 0){
        printf("Num fingertips: %i\n",fingertips.size());
    }

    for (int i =0;i < fingertips.size();i++){
        struct fingertip *f = fingertips[i];
        m->transform_xy(f->x,f->y,&(f->x),&f->y);
        int x0 = f->x;
        int y0=f->y;
        //printf("FINGERTIP %i %i\n",x0,y0);
    }
    
}
void h_detect::flood_fill(int x,int y){
    num_points=0;
    max_y = l_frame->width;
    if (!accept(x,y))
        return;
    memset(recog_buffer,0,buffer_size);
    flood_fill_rec(x,y);
}
bool h_detect::accept(int x,int y){
  //  printf("X,Y = %i %i = %i (init_y = %i)\n",x,y,edge_buffer[ebw*y + x],init_y);
    int edge_val = (edge_buffer[ebw*y +x]*255)/l_frame->edge_max;
   if (edge_val  < 10  && m->in_border_bounds(x,y) && l_frame->get_y(x,y) > l_frame->y_thresh){
        return true;
    }
    return false;
}

