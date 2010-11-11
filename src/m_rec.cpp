#include "m_rec.h"
#include "pose.h"
#include "v_util.h"
#include <stdio.h>
#include <vector>
#include <math.h>
#include <opencv/cv.h>

m_rec::m_rec(frame & f) : l_frame(f){
    pose_im  = new pose;
    float temp [4][2] = {{0.,0.},{1920.,0.},{1920.,1200.},{0.,1200.}};
    for (int i=0;i < 4;i++){
        for (int j=0;j <4 ;j++){
            screen_corners[i][j] = temp[i][j];
        }
    }
    _display_points = cvMat(4,2,CV_64F,screen_corners);
    _h = cvMat(3,3,CV_64F,homography_matrix);
    _src_points = cvMat(4,2,CV_64F,image_corners);
    lastdr1=0;
    lastdr2=0;

}


bool m_rec::find_monitor(){

    struct screen_obstruction scr_ob;
    if (detected && false){
        if (check_quadrangle(&old_a,&old_b,&old_i,&old_j,&scr_ob)){
           /*( if (scr_ob.N_out > 10) { // lower obstruction is too big
                continue;
            }*/
            //printf("Number of checks %i, Plells.size() = %i, First %i\n",num_checks,(int)plells.size(),n);
            detected = true;
            printf("N_OUT: %i\n",scr_ob.N_out);
            if (scr_ob.N_out > 30 ){
                printf("Hand point begining : %i %i\n",scr_ob.centre_x,scr_ob.centre_y);
                int * can = l_frame.op;//l_frame.compute_canny();
                hand_detected=true;
                handx= scr_ob.centre_x;
                handy = scr_ob.centre_y;

            }
            set_found(&old_a,&old_b,&old_i,&old_j);
            
            pose_im->local_image_points[0] = ptl.x - l_frame.width/2;
            pose_im->local_image_points[1] = ptl.y - l_frame.height/2;
            pose_im->local_image_points[2] = ptr.x - l_frame.width/2;
            pose_im->local_image_points[3] = ptr.y - l_frame.height/2;
            pose_im->local_image_points[4] = pbr.x - l_frame.width/2;
            pose_im->local_image_points[5] = pbr.y - l_frame.height/2;
            pose_im->local_image_points[6] = pbl.x - l_frame.width/2;
            pose_im->local_image_points[7] = pbl.y - l_frame.height/2;
            //pose_im->compute_pose(pose_im->local_image_points);

            printf("Last check found it.\n");
            return true;
            }

    }



    frame & f = (l_frame);
    std::vector <struct pair2 *> plells;


    for (int i=0;i < std::min(25,(int)f.line_accum.size());i++){
        struct line *l = f.line_accum[i];
        for (int j=i+1;j < std::min(35,(int)f.line_accum.size());j++){
            struct line *l2 = f.line_accum[j];
            if (angle_difference(l->theta,l2->theta) < 0.35 && fabs(l->r -l2->r) > 100){
                    struct pair2 * p = new struct pair2;
                    p->i=i;
                    p->j=j;
                    plells.push_back(p);
            }
    }
    }
    int num_checks =0;

    hand_detected=false;
    
   // detected=false;return false; 

    for (int n=0;n < plells.size();n++){
        struct pair2 * pn = plells[n];
        struct line * l = f.line_accum[pn->i];
        struct line * lb = f.line_accum[pn->j];
        for (int m=n+1;m < plells.size();m++){
            struct pair2 * pm = plells[m];
            struct line * l2 = f.line_accum[pm->i];
            struct line * l2b = f.line_accum[pm->j];
            if (angle_difference(l->theta,l2->theta)  >1.25  && angle_difference(l->theta,l2->theta) < 1.92){
//                printf("Angle difference passed\n");
            float inc = 3;
                num_checks++;
                if (check_quadrangle(l,lb,l2,l2b,&scr_ob)){
                   // printf("Passed check quadrangle\n");
                   /*( if (scr_ob.N_out > 10) { // lower obstruction is too big
                        continue;
                    }*/
                    //printf("Number of checks %i, Plells.size() = %i, First %i\n",num_checks,(int)plells.size(),n);
                    detected = true;
                    set_found(l,lb,l2,l2b);
                   // printf("N_OUT: %i\n",scr_ob.N_out);
                    if (scr_ob.N_out > 15 ){
                      //  printf("Hand point begining : %i %i\n",scr_ob.centre_x,scr_ob.centre_y);
                        int * can = l_frame.op;//l_frame.compute_canny();
                        hand_detected=true;
                        //printf("centre_x = %i centre_y = %i\n",scr_ob.centre_x,scr_ob.centre_y);
                        if (validate_hand(&scr_ob,&handx,&handy) == -1){
                            hand_detected = false;
                        }
                        else{
                            hand_detected=true;
                        }

                    }
                    float ar1 = std::abs(l->r);
                    float ar2 = std::abs(lb->r);
                    float br1 = std::abs(l2->r);
                    float br2 = std::abs(l2b->r);
                    lastdr1 = std::abs(ar1-ar2);
                    lastdr2 = std::abs(br1-br2);
                    
                    pose_im->local_image_points[0] = ptl.x - l_frame.width/2;
                    pose_im->local_image_points[1] = ptl.y - l_frame.height/2;
                    pose_im->local_image_points[2] = ptr.x - l_frame.width/2;
                    pose_im->local_image_points[3] = ptr.y - l_frame.height/2;
                    pose_im->local_image_points[4] = pbr.x - l_frame.width/2;
                    pose_im->local_image_points[5] = pbr.y - l_frame.height/2;
                    pose_im->local_image_points[6] = pbl.x - l_frame.width/2;
                    pose_im->local_image_points[7] = pbl.y - l_frame.height/2;
                    pose_im->compute_pose(pose_im->local_image_points);

                    //printf("FOUND!!!!!!!!!!!\n");
                    return true;
                    }
                
            }

        }
    }
    printf(":: FAILED :: Number of checks %i, Plells.size() = %i\n",num_checks,(int)plells.size());
    struct line temp_line;
    for (int n=0;n < plells.size();n++){
        struct pair2 * pn = plells[n];
        struct line * l = f.line_accum[pn->i];
        struct line * lb = f.line_accum[pn->j];
        for (int k=0;k < (int)f.line_accum.size();k++){
            struct line * l2 = f.line_accum[k];
            if (angle_difference(l->theta,l2->theta)  >1.25  && angle_difference(l->theta,l2->theta) < 1.92){
                float tdr = std::abs(l->r - lb->r);
                float extr_dr;
                if (std::abs(tdr - lastdr1) < std::abs(tdr-lastdr2)){
                    extr_dr = lastdr2;
                }
                else{
                    extr_dr = lastdr1;
                }
                temp_line.theta=l2->theta;
                temp_line.r=l2->r+extr_dr;

                if (check_quadrangle(l,lb,l2,&temp_line,&scr_ob)){
                    detected=true;
                    set_found(l,lb,l2,&temp_line);
                    if (scr_ob.N_out > 15 ){
                      //  printf("Hand point begining : %i %i\n",scr_ob.centre_x,scr_ob.centre_y);
                        hand_detected=true;
                        //printf("centre_x = %i centre_y = %i\n",scr_ob.centre_x,scr_ob.centre_y);
                        if (validate_hand(&scr_ob,&handx,&handy) == -1){
                            hand_detected = false;
                        }
                        else{
                            hand_detected=true;
                        }

                    }
                    printf("Found it!\n");
                    printf("Using tdr = %f\n",extr_dr);
                    return true;
                }
                temp_line.r=l2->r-extr_dr;
                if (check_quadrangle(l,lb,l2,&temp_line,&scr_ob)){
                    detected=true;
                    if (scr_ob.N_out > 15 ){
                      //  printf("Hand point begining : %i %i\n",scr_ob.centre_x,scr_ob.centre_y);
                        hand_detected=true;
                        //printf("centre_x = %i centre_y = %i\n",scr_ob.centre_x,scr_ob.centre_y);
                        if (validate_hand(&scr_ob,&handx,&handy) == -1){
                            hand_detected = false;
                        }
                        else{
                            hand_detected=true;
                        }

                    }
                    set_found(l,lb,l2,&temp_line);
                    printf("Found it!\n");
                    printf("Using tdr = %f\n",extr_dr);
                    return true;
                }

            }
        }
    }
    // NOT DETECTED! , Go for broke, 3 lines
    detected = false;
    return false;
}


/*         Rotation invariant
 *      _______i__________>
 *     |                 |
 *     |                 |
 *     |                 |
 *    a|                 |b
 *     |                 |
 *     |_______j________>|
 *     v                 v   
 *
 *
 *      _______horiz0_____>
 *     |                 |
 *     |                 |
 *     |                 |
  vert0|                 |vert1
 *     |                 |
 *     |_____horiz1_____>|
 *     v                 v   
 *
 *
  */
bool m_rec::check_quadrangle(struct line * a,struct line * b, struct line * i,struct line * j, struct screen_obstruction * scr_ob){

    vec2f ai,aj,bi,bj,av,bv,iv,jv;

 //   printf("Check quadrangle: abij %f %f %f %f\n",a->theta,b->theta,i->theta,j->theta);

    // classify line
    struct line * horiz[2];
    struct line * vert[2];
    if ( a->theta < M_PI/4.f || a->theta > 3.f*M_PI/4.f){ // a,b are vertical
        vert[0]=a;
        vert[1]=b;
        horiz[0]=i;
        horiz[1]=j;
    }
    else{ // I, J is vertical
        vert[0]=i;
        vert[1]=j;
        horiz[0]=a;
        horiz[1]=b;
    }
    if (fabs(horiz[0]->r) > horiz[1]->r){
        struct line * t= horiz[0];
        horiz[0]=horiz[1];
        horiz[1]=t;
    }
    if (fabs(vert[0]->r) > vert[1]->r){
        struct line * t= vert[0];
        vert[0]=vert[1];
        vert[1]=t;
    }


    float boam = 8;
    rt_intersection_vec2f(vert[0]->r,vert[0]->theta,horiz[0]->r,horiz[0]->theta,&ai);
    rt_intersection_vec2f(vert[0]->r,vert[0]->theta,horiz[1]->r,horiz[1]->theta,&aj);
    rt_intersection_vec2f(vert[1]->r,vert[1]->theta,horiz[0]->r,horiz[0]->theta,&bi);
    rt_intersection_vec2f(vert[1]->r,vert[1]->theta,horiz[1]->r,horiz[1]->theta,&bj);

    

    vec2f_subtract(&aj,&ai,&av);
    vec2f_subtract(&bj,&bi,&bv);
    vec2f_subtract(&bi,&ai,&iv);
    vec2f_subtract(&bj,&aj,&jv);

    if (fabs(jv.x) < 0.4f *l_frame.width ){
        return false;
    }
    vec2f_normalise(&av);
    vec2f_normalise(&bv);
    vec2f_normalise(&iv);
    vec2f_normalise(&jv);
    struct screen_obstruction si,sj,sa,sb;
    si.directionx = iv.x;si.directiony = iv.y;
    sj.directionx = jv.x;sj.directiony = jv.y;
    sa.directionx = av.x;sa.directiony = av.y;
    sb.directionx = bv.x;sb.directiony = bv.y;
    vec2f_mul(&av,boam);
    vec2f_mul(&bv,boam);
    vec2f_mul(&iv,boam);
    vec2f_mul(&jv,boam);
   // printf("(%i %i) ---------------- (%i,%i)\n",(int)ai.x,(int)ai.y,(int)bi.x,(int)bi.y);
    //printf("|------------------------------|\n");
    //printf("(%i %i) ---------------- (%i,%i)\n",(int)aj.x,(int)aj.y,(int)bj.x,(int)bj.y);

    //memset(scr_ob,0,sizeof(struct screen_obstruction));
    bool tol_out = true;
    int ij_out=0;
    int ab_out=0;
    int ab_out_thresh=10;
    int ij_out_thresh=10;

    bool I = border_check(ai.x -av.x,ai.y - av.y,bi.x - bv.x,bi.y - bv.y,&si,true);
    if (!I){
        return false;
    }
    if (si.N_out > ij_out_thresh)
        ij_out+=1;

    bool J = border_check(aj.x + av.x,aj.y + av.y,bj.x + bv.x,bj.y + bv.y,&sj,true);
    if (!J){
        return false;
    }
    if (sj.N_out > ij_out_thresh){
        ij_out+=1;
        if (ij_out == 2)
            return false;
    }


    bool A = border_check(ai.x -iv.x,ai.y-iv.y,aj.x-jv.x,aj.y-jv.y,&sa,true);
    if (!A){
        return false;
    }
    if (sa.N_out > ab_out_thresh){
        ab_out+=1;
    }

    bool B = border_check(bi.x+ iv.x,bi.y+ iv.y,bj.x+jv.x,bj.y+jv.y,&sb,true);
    if (!B){
        return false;
    }
    if (sb.N_out > ab_out_thresh){
        ab_out+=1;
        if (ab_out == 2)
            return false;
    }

    if ((ij_out + ab_out ) >= 1){
      //  printf("Hand Detected %i\n",sj.N_out);
        struct screen_obstruction * sp;
        sp = &sa;
        if (sb.N_out > sp->N_out)
            sp=&sb;
        if (si.N_out > sp->N_out)
            sp=&si;
        if (sj.N_out > sp->N_out)
            sp=&sj;
        scr_ob->N_out = sp->N_out;
        scr_ob->centre_x = sp->centre_x;
        scr_ob->centre_y = sp->centre_y;
        scr_ob->directionx =sp->directionx;
        scr_ob->directiony = sp->directiony;

        //printf("SCROB out = %i\n",scr_ob->N_out);
    }
    else{
        scr_ob->N_out=0;

    }


    return true;

//    return borderva && bordervb && borderhi && borderhj;
}
int m_rec::validate_hand(struct screen_obstruction * sob, int *fx,int *fy){
    int cx = sob->centre_x;
    int cy = sob->centre_y;
    float dx = sob->directionx;
    float dy = sob->directiony;
    for (int i=0;i < 80;i++){
        int ofx = cx + (int) ((float)i*dx);
        int ofy = cy + (int) ((float)i*dy);
        if (!in_border_bounds(ofx,ofy)){
            continue;
        }
        int val = l_frame.get_y(ofx,ofy);
        int edge_val = l_frame.get_normalised_edge(ofx,ofy);
        if (val > l_frame.y_thresh && edge_val < 15){
            *fx = ofx;
            *fy=ofy;
            return 0;
            //continue;
        }
        ofx = cx - (int) ((float)i*dx);
        ofy = cy - (int) ((float)i*dy);
        if (!in_border_bounds(ofx,ofy)){
            continue;
        }
        val = l_frame.get_y(ofx,ofy);
        edge_val = l_frame.get_normalised_edge(ofx,ofy);
        if (val > l_frame.y_thresh && edge_val < 15){
            *fx = ofx;
            *fy=ofy;
            return 0;
            //continue;
        }
    }
    return -1;
}
void m_rec::set_found(struct line * a,struct line * b, struct line * i,struct line * j){

    old_a.r=a->r;
    old_a.theta=a->theta;
    old_b.r=b->r;
    old_b.theta=b->theta;
    old_i.r=i->r;
    old_i.theta=i->theta;
    old_j.r=j->r;
    old_j.theta=j->theta;

    //printf("RDIFFS: %f %f\n",a->r-b->r,i->r-j->r);

    lcorner_ai.x = corner_ai.x;
    lcorner_aj.x = corner_aj.x;
    lcorner_bi.x = corner_bi.x;
    lcorner_bj.x = corner_bj.x;

    lcorner_ai.y = corner_ai.y;
    lcorner_aj.y = corner_aj.y;
    lcorner_bi.y = corner_bi.y;
    lcorner_bj.y = corner_bj.y;

    vec2f ai,aj,bi,bj;

    rt_intersection_vec2f(a->r,a->theta,i->r,i->theta,&corner_ai);
    rt_intersection_vec2f(a->r,a->theta,j->r,j->theta,&corner_aj);
    rt_intersection_vec2f(b->r,b->theta,i->r,i->theta,&corner_bi);
    rt_intersection_vec2f(b->r,b->theta,j->r,j->theta,&corner_bj);

    rt_intersection_vec2f(a->r,a->theta,i->r,i->theta,&ai);
    rt_intersection_vec2f(a->r,a->theta,j->r,j->theta,&aj);
    rt_intersection_vec2f(b->r,b->theta,i->r,i->theta,&bi);
    rt_intersection_vec2f(b->r,b->theta,j->r,j->theta,&bj);
    vec2f idiff;
    vec2f_subtract(&ai,&bi,&idiff);

    // XXX this is bad
    if (fabs(idiff.x) > fabs(idiff.y)){ // i is horizontal , j is horizontal
        if (ai.y < aj.y){ // i is on top
            if (ai.x < bi.x) {// a is left
                vec2f_set(&ptl,&ai);//
                vec2f_set(&ptr,&bi);//
                vec2f_set(&pbl,&aj);//
                vec2f_set(&pbr,&bj);//
            }
            else{ // b is left
                vec2f_set(&ptl,&bi);//
                vec2f_set(&ptr,&ai);//
                vec2f_set(&pbl,&bj);//
                vec2f_set(&pbr,&aj);//
                
            }
        }
        else{ // j is on top
            if (ai.x < bi.x) {// a is left
                vec2f_set(&ptl,&aj);//
                vec2f_set(&ptr,&bj);//
                vec2f_set(&pbl,&ai);//
                vec2f_set(&pbr,&bi);//
            }
            else{ // b is left
                vec2f_set(&ptl,&bj);//
                vec2f_set(&ptr,&aj);//
                vec2f_set(&pbl,&bi);//
                vec2f_set(&pbr,&ai);//
                
            }
        }

    }
    else{ // a is hoizontal , b is hoizontal
        if (aj.y <bj.y){// a is on top
            if (aj.x < bi.x){// j is left
                vec2f_set(&ptl,&aj);//
                vec2f_set(&ptr,&ai);//
                vec2f_set(&pbl,&bj);//
                vec2f_set(&pbr,&bi);//
            }
            else{ // i is left
                vec2f_set(&ptl,&ai);//
                vec2f_set(&ptr,&aj);//
                vec2f_set(&pbl,&bi);//
                vec2f_set(&pbr,&bj);//
            }
        }
        else{ // b is on top
            if (aj.x < bi.x){// j is left
                vec2f_set(&ptl,&bj);//
                vec2f_set(&ptr,&bi);//
                vec2f_set(&pbl,&aj);//
                vec2f_set(&pbr,&ai);//
            }
            else{ // i is left
                vec2f_set(&ptl,&bi);//
                vec2f_set(&ptr,&bj);//
                vec2f_set(&pbl,&ai);//
                vec2f_set(&pbr,&aj);//
            }
        }
    }

    //}double image_corners[4][2] = {{(double)monitor_rec.ptl.x,(double)monitor_rec.ptl.y},{(double)monitor_rec.ptr.x,(double)monitor_rec.ptr.y},{(double)monitor_rec.pbr.x,(double)monitor_rec.pbr.y},{(double)monitor_rec.pbl.x,(double)monitor_rec.pbl.y}};
    image_corners[0][0] = ptl.x;
    image_corners[0][1] = ptl.y;

    image_corners[1][0] = ptr.x;
    image_corners[1][1] = ptr.y;

    image_corners[2][0] = pbr.x;
    image_corners[2][1] = pbr.y;

    image_corners[3][0] = pbl.x;
    image_corners[3][1] = pbl.y;

    int result = cvFindHomography(&_src_points,&_display_points,&_h,CV_RANSAC,5);
    //printf("Result = %i\n",result);
    //printf("%f %f %f\n%f %f %f\n%f %f %f\n",homography_matrix[0],homography_matrix[1],homography_matrix[2],homography_matrix[3],homography_matrix[4],homography_matrix[5],homography_matrix[6],homography_matrix[7],homography_matrix[8]);


}

int m_rec::transform_xy(int x,int y,int * ox,int * oy){
    float hx=(float)x;
    float hy=(float)y;
    float coef = homography_matrix[6]*hx + homography_matrix[7]*hy + homography_matrix[8];
    int realx =(int) ((homography_matrix[0]*hx + homography_matrix[1]*hy + homography_matrix[2])/coef);
    int realy =(int) ((homography_matrix[3]*hx + homography_matrix[4]*hy+homography_matrix[5])/coef);
    //printf("Realxy %i %i\n",realx,realy);
    *ox = realx;
    *oy = realy;
}
bool m_rec::in_bounds(int x,int y){
    int ox,oy;
    transform_xy(x,y,&ox,&oy);
    if (ox < 0 || ox >= 1920|| oy <0 || oy >= 1200)
        return false;
    return true;
}
bool m_rec::in_border_bounds(int x,int y){
    int ox,oy;
    transform_xy(x,y,&ox,&oy);
    int border_amount = 90;
    if (ox < -border_amount || ox >= 1920 + border_amount|| oy <-border_amount || oy >= 1200 + border_amount)
        return false;
    return true;
}

bool m_rec::border_check(int x0,int y0,int x1,int y1,struct screen_obstruction * so, bool tolerate_out){
    //return false;
    //return true;

    so->N_out=0;
    so->total_N=0;
    so->centre_x=0;
    so->centre_y=0;

    
  /*  if (!l_frame.in_bounds(x0,y0) || !l_frame.in_bounds(x1,y1)){
        printf("Out of bounds %i %i  ---   %i %i\n",x0,y0,x1,y1);
        return false;
            }*/
    if (x0 < -2000 || y0 < -2000 || x1 < -2000 || y1 < -2000){
        //printf("Bad intersection points\n");
        return false;
    }
    //printf("%i %i - %i %i\n",x0,y0,x1,y1);
     bool steep = abs(y1 - y0) > abs(x1 - x0);
     if (steep){
         swap(&x0, &y0);
         swap(&x1, &y1);
     }
     if (x0 > x1){
         swap(&x0, &x1);
         swap(&y0, &y1);
     }
     int deltax = x1 - x0;
     int deltay = abs(y1 - y0);
     float error = 0.0f;
     float deltaerr =(float) deltay / (float)deltax;
     int ystep;
     int y = y0;
     int d_thresh = 6;
     int y_sum=0;
     int N=0;
     int out_count=0;


     // XXX acceptable out_pixels still affect mean - can cause error;
     //
     if (y0 < y1) { ystep = 1;} else {ystep = -1;}
     for (int x =x0;x<=x1;x++){
         if (steep){ // y x
             if (!l_frame.in_bounds(y,x)){
                 continue;
             }else{
             int y_val = l_frame.get_y(y,x);
             if (y_val > l_frame.y_thresh){
                 out_count++;
                 so->centre_x+=y;
                 so->centre_y+=x;
                 so->N_out++;
                 if (out_count > 20 && !tolerate_out){
                //    printf("Out count failed\n");
                    return false;
                 }
             }
             //l_frame.mark_r(y,x);
             y_sum+=y_val;
             N++;
             }
	 }
	 else {// x y
             if (!l_frame.in_bounds(x,y)){
                 continue;
             }else{
             int y_val = l_frame.get_y(x,y);
             if (y_val > l_frame.y_thresh){
                 out_count++;
                 so->centre_x+=x;
                 so->centre_y+=y;
                 so->N_out++;
                 if (out_count > 20 && !tolerate_out){
                  //   printf("Out count failed\n");
                    return false;
                 }
             }
             //l_frame.mark_r(x,y);
             y_sum+=y_val;
             N++;
             }
	 }
	 
         error = error + deltaerr;
         if (error >= 0.5){
             y = y + ystep;
             error = error - 1.0;
	 }
     }
     if (N==0){
         return true;
     }
     y_sum/=N;
     //if (so->N_out >0){
     if (so->N_out ==0){
         so->N_out=1;
     }
     
     if (((float)so->N_out / (float) N) < 0.5f){
     so->centre_x/=so->N_out;
     so->centre_y/=so->N_out;
     }

     so->total_N=N;
     //printf("Sum is %i\n",(r_sum + b_sum + g_sum)/3);
     if ((y_sum) < l_frame.y_thresh){
         return true;

     }
     else{
         //printf("FAILED DUE TO AVG! %i\n",r_sum+g_sum+b_sum);
         return false;
     }

}

float m_rec::dist(point & p1,point & p2){
    float dx = (p1.x-p2.x);
    float dy = (p1.y- p2.y);
    float dz = (p1.z- p2.z);
    return sqrtf(dx*dx + dy*dy + dz*dz);
}
bool m_rec::orient(){


}
