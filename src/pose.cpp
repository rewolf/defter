#include "pose.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>


pose::pose(){
    local_image_points = (float*) malloc(4*2*sizeof(float)  );
    local_image_vectors = (float*) malloc(4*2*sizeof(float)  );
    local_object_points = (float*) malloc(4*3*sizeof(float));
    local_object_vectors = (float*) malloc(4*3*sizeof(float));
    focal_length=480;
    float op[] = {0,0,0,16,0,0,0,9,0,16,9,0};
    //float op[] = {-8.f,-4.5f,0,8.f,-4.5f,0,-8.f,4.5f,0,8.f,4.5f,0};
    for (int i =0;i < 12;i++){
        local_object_points[i] = op[i];
    }
    compute_object_vectors(local_object_vectors,local_object_points);

}

void pose::pos(float * image_points,float * object_points,float * image_vectors,float * rotation,float * translation){
    float    I0[3], J0[3], row1[3], row2[3], row3[3];
    float    I0I0, J0J0;
    float scale, scale1, scale2;

    for (int i=0;i < 3;i++){
        I0[i]=0;
        J0[i]=0;
        for (int j=0;j < 4;j++){
            I0[i]+=object_points[j*3 + i] * image_vectors[2*j];
            J0[i]+=object_points[j*3 + i] * image_vectors[2*j + 1];
            //printf ("I[%i] J[%i] %f %f\n",i,i,I0[i],J0[i]);
        }
    }
    I0I0=I0[0]*I0[0] + I0[1]*I0[1] + I0[2]*I0[2];
    J0J0=J0[0]*J0[0] + J0[1]*J0[1] + J0[2]*J0[2];

    scale1 = sqrtf(I0I0);
    scale2 = sqrtf(J0J0);
    scale = (scale1 + scale2) / 2.0;


    translation[0] = image_points[0]/scale;
    translation[1] = image_points[1]/scale;
    translation[2] = focal_length/scale;

    for (int i=0;i<3;i++){
       row1[i]=I0[i]/scale1;
       row2[i]=J0[i]/scale2;
    }
    row3[0]=row1[1]*row2[2]-row1[2]*row2[1];/* Cross-product to obtain third row */
    row3[1]=row1[2]*row2[0]-row1[0]*row2[2];
    row3[2]=row1[0]*row2[1]-row1[1]*row2[0];
    for (int i=0;i<3;i++){ // XXX this may be wrong
       rotation[0 + i]=row1[i];
       rotation[3 + i]=row2[i];
       rotation[6 + i]=row3[i];
    }

    //printf("Translation %f %f %f\n",1000*translation[0],1000*translation[1],1000*translation[2]);
   //printf("Rotation:\n%f %f %f\n%f %f %f\n%f %f %f\n",rotation[0],rotation[1],rotation[2],rotation[3],rotation[4],rotation[5],rotation[6],rotation[7],rotation[8]);
    

    
}
void pose::compute_pose(float * image_points){
    float rotation[3*3];
    float translation[3];
    posit(image_points,local_object_points,local_image_vectors,local_object_vectors,rotation,translation);
}
void pose::compute_object_vectors(float *ov,float *op){

    for (int pn=0, off=0;pn < 4;pn++){
        ov[off++]=op[3*pn] - op[0];
        ov[off++]=op[3*pn + 1] - op[1] ;
        ov[off++]=op[3*pn + 2] - op[2] ;
        printf("OBJECT VECTOR: %f %f %f\n",ov[3*pn],ov[3*pn+1],ov[3*pn+2]);
    }
}
void pose::posit(float * image_points,float * object_points,float * image_vectors,float * object_vectors,float * rotation,float * translation){
    float  epsilon[4];
    float old_image_vectors[40]; // XXX fix size
    for (int iter=0;iter < 5;iter++){
        if (iter==0){
            for (int pn=0,off=0;pn < 4;pn++){
                image_vectors[off++]=image_points[2*pn] - image_points[0];
                image_vectors[off++]=image_points[2*pn + 1] - image_points[1];
            }
        }
        else{ // new image vectors XXX must compute object vectors
            for (int pn=0;pn < 4;pn++){
                epsilon[pn] = 0;
                for (int j=0;j < 3;j++){
                    epsilon[pn]+=object_vectors[3*pn + j] * rotation[6+j];
               //     printf("it is : %f %f\n",object_vectors[3*pn + j] , rotation[6+j]);
                }
                epsilon[pn]/=translation[2];

            }
            for (int pn=0,off=0;pn < 4;pn++){
                image_vectors[off++]=image_points[2*pn]*(1 + epsilon[pn]) - image_points[0];
                //printf("eps: %f\n",epsilon[pn]);
                image_vectors[off++]=image_points[2*pn + 1]*(1+ epsilon[pn]) - image_points[1];
            }

            //printf("Image diff!\n");

        }
        memcpy(old_image_vectors,image_vectors,sizeof(float)*2*4);
        pos(image_points,object_points,image_vectors,rotation,translation);

    }
}
