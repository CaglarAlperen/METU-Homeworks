/********************************************************
 * Kernels to be optimized for the Metu Ceng Performance Lab
 ********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "defs.h"

/* 
 * Please fill in the following team struct 
 */
team_t team = {
    "xKralTR",                  /* Team name */

    "Çağlar Alperen Tosun",     /* First member full name */
    "e2310530",                 /* First member id */

    "",                         /* Second member full name (leave blank if none) */
    "",                         /* Second member id (leave blank if none) */

    "",                         /* Third member full name (leave blank if none) */
    ""                          /* Third member id (leave blank if none) */
};

/****************
 * EXPOSURE FUSION KERNEL *
 ****************/

/*******************************************************
 * Your different versions of the exposure fusion kernel go here 
 *******************************************************/

/* 
 * naive_fusion - The naive baseline version of exposure fusion
 */
char naive_fusion_descr[] = "naive_fusion: Naive baseline exposure fusion";
void naive_fusion(int dim, int *img, int *w, int *dst) {
  
    int i, j, k; 
    for(k = 0; k < 4; k++){
        for(i = 0; i < dim; i++) {
            for(j = 0; j < dim; j++) {
                    dst[i*dim+j] += w[k*dim*dim+i*dim+j] * img[k*dim*dim+i*dim+j];
            }
        }
    }
}

/* 
 * fusion - Your current working version of fusion
 * IMPORTANT: This is the version you will be graded on
 */
char fusion_descr[] = "fusion: Current working version";
void fusion(int dim, int *img, int *w, int *dst) 
{
    int i, j, k, ki, idim; 
    for(k = 0; k < 4; k++){
        for(i = 0; i < dim; i++) {
	    idim = i*dim;
	    ki = k*dim*dim + idim;
            for(j = 0; j < dim-7; j+=8) {
                    dst[idim+j] += w[ki+j] * img[ki+j];
                    dst[idim+j+1] += w[ki+j+1] * img[ki+j+1];
                    dst[idim+j+2] += w[ki+j+2] * img[ki+j+2];
                    dst[idim+j+3] += w[ki+j+3] * img[ki+j+3];
                    dst[idim+j+4] += w[ki+j+4] * img[ki+j+4];
                    dst[idim+j+5] += w[ki+j+5] * img[ki+j+5];
                    dst[idim+j+6] += w[ki+j+6] * img[ki+j+6];
                    dst[idim+j+7] += w[ki+j+7] * img[ki+j+7];
            }
	    for (; j < dim; j++)
	    {
		dst[idim+j] += w[ki+j] * img[ki+j];
	    }
        }
    }
}

/*********************************************************************
 * register_fusion_functions - Register all of your different versions
 *     of the fusion kernel with the driver by calling the
 *     add_fusion_function() for each test function. When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.  
 *********************************************************************/

void register_fusion_functions() 
{
    add_fusion_function(&naive_fusion, naive_fusion_descr);  
    add_fusion_function(&fusion, fusion_descr);  
    /* ... Register additional test functions here */
}

/***************************
 * GAUSSIAN BLUR KERNEL *
 ***************************/

/******************************************************
 * Your different versions of the Gaussian blur functions go here
 ******************************************************/

/* 
 * naive_blur - The naive baseline version of Gussian blur
 */
char naive_blur_descr[] = "naive_blur The naive baseline version of Gaussian blur";
void naive_blur(int dim, float *img, float *flt, float *dst) {
  
    int i, j, k, l;

    for(i = 0; i < dim-5+1; i++){
        for(j = 0; j < dim-5+1; j++) {
            for(k = 0; k < 5; k++){
                for(l = 0; l < 5; l++) {
                    dst[i*dim+j] = dst[i*dim+j] + img[(i+k)*dim+j+l] * flt[k*dim+l];
                }
            }
        }
    }
}

/* 
 * blur - Your current working version of Gaussian blur
 * IMPORTANT: This is the version you will be graded on
 */
char blur_descr[] = "blur: Current working version";
void blur(int dim, float *img, float *flt, float *dst) 
{
    int i, j, idim, idim2, idim3, idim4, idim5, row;
    float flt11,flt12,flt13,flt14,flt15;
    float flt21,flt22,flt23,flt24,flt25;
    float flt31,flt32,flt33,flt34,flt35;
    float flt41,flt42,flt43,flt44,flt45;
    float flt51,flt52,flt53,flt54,flt55;

    flt11 = *flt; flt12 = *(flt+1); flt13 = *(flt+2); flt14 = *(flt+3); flt15 = *(flt+4);
    flt21 = *(flt+dim); flt22 = *(flt+dim+1); flt23 = *(flt+dim+2); flt24 = *(flt+dim+3); flt25 = *(flt+dim+4);
    flt31 = *(flt+2*dim); flt32 = *(flt+2*dim+1); flt33 = *(flt+2*dim+2); flt34 = *(flt+2*dim+3); flt35 = *(flt+2*dim+4);
    flt41 = *(flt+3*dim); flt42 = *(flt+3*dim+1); flt43 = *(flt+3*dim+2); flt44 = *(flt+3*dim+3); flt45 = *(flt+3*dim+4);
    flt51 = *(flt+4*dim); flt52 = *(flt+4*dim+1); flt53 = *(flt+4*dim+2); flt54 = *(flt+4*dim+3); flt55 = *(flt+4*dim+4);

    // First Row
    for (i = 0; i < dim-4; i++){
    	dst[i] += img[i] * flt11 + img[i+1] * flt12 + img[i+2] * flt13 + img[i+3] * flt14 + img[i+4] * flt15;
    }
    idim = dim;
    // Second Row
    for (i = 0; i < dim-4; i++){
	dst[i] += img[idim+i] * flt21 + img[idim+i+1] * flt22 + img[idim+i+2] * flt23 + img[idim+i+3] * flt24 + img[idim+i+4] * flt25;
	dst[idim+i] += img[idim+i] * flt11 + img[idim+i+1] * flt12 + img[idim+i+2] * flt13 + img[idim+i+3] * flt14 + img[idim+i+4] * flt15;
    }
    idim2 = idim+dim;
    // Third Row
    for (i = 0; i < dim-4; i++){
	dst[i] += img[idim2+i] * flt31 + img[idim2+i+1] * flt32 + img[idim2+i+2] * flt33 + img[idim2+i+3] * flt34 + img[idim2+i+4] * flt35;
	dst[idim+i] += img[idim2+i] * flt21 + img[idim2+i+1] * flt22 + img[idim2+i+2] * flt23 + img[idim2+i+3] * flt24 + img[idim2+i+4] * flt25;
	dst[idim2+i] += img[idim2+i] * flt11 + img[idim2+i+1] * flt12 + img[idim2+i+2] * flt13 + img[idim2+i+3] * flt14 + img[idim2+i+4] * flt15;
    }
    idim3 = idim2+dim;
    // Fourth Row
    for (i = 0; i < dim-4; i++){
	dst[i] += img[idim3+i] * flt41 + img[idim3+i+1] * flt42 + img[idim3+i+2] * flt43 + img[idim3+i+3] * flt44 + img[idim3+i+4] * flt45;
	dst[idim+i] += img[idim3+i] * flt31 + img[idim3+i+1] * flt32 + img[idim3+i+2] * flt33 + img[idim3+i+3] * flt34 + img[idim3+i+4] * flt35;
	dst[idim2+i] += img[idim3+i] * flt21 + img[idim3+i+1] * flt22 + img[idim3+i+2] * flt23 + img[idim3+i+3] * flt24 + img[idim3+i+4] * flt25;
	dst[idim3+i] += img[idim3+i] * flt11 + img[idim3+i+1] * flt12 + img[idim3+i+2] * flt13 + img[idim3+i+3] * flt14 + img[idim3+i+4] * flt15;
    }
    idim = 0;
    idim2 = dim;
    idim3 = idim2+dim;
    idim4 = idim3+dim;
    idim5 = idim4+dim;
    row = 4*dim;
    // Mid Rows
    for (j = 4; j < dim-4; j++){
	for (i = 0; i < dim-4; i++){
	    dst[idim+i] += img[row+i] * flt51 + img[row+i+1] * flt52 + img[row+i+2] * flt53 + img[row+i+3] * flt54 + img[row+i+4] * flt55;
	    dst[idim2+i] += img[row+i] * flt41 + img[row+i+1] * flt42 + img[row+i+2] * flt43 + img[row+i+3] * flt44 + img[row+i+4] * flt45;
	    dst[idim3+i] += img[row+i] * flt31 + img[row+i+1] * flt32 + img[row+i+2] * flt33 + img[row+i+3] * flt34 + img[row+i+4] * flt35;
	    dst[idim4+i] += img[row+i] * flt21 + img[row+i+1] * flt22 + img[row+i+2] * flt23 + img[row+i+3] * flt24 + img[row+i+4] * flt25;
	    dst[idim5+i] += img[row+i] * flt11 + img[row+i+1] * flt12 + img[row+i+2] * flt13 + img[row+i+3] * flt14 + img[row+i+4] * flt15;
	}
	row += dim;
	idim += dim;
	idim2 += dim;
	idim3 += dim;
	idim4 += dim;
	idim5 += dim;
    }

    // Fourth To Last
    for (i = 0; i < dim-4; i++){
	dst[idim+i] += img[row+i] * flt51 + img[row+i+1] * flt52 + img[row+i+2] * flt53 + img[row+i+3] * flt54 + img[row+i+4] * flt55;
	dst[idim2+i] += img[row+i] * flt41 + img[row+i+1] * flt42 + img[row+i+2] * flt43 + img[row+i+3] * flt44 + img[row+i+4] * flt45;
	dst[idim3+i] += img[row+i] * flt31 + img[row+i+1] * flt32 + img[row+i+2] * flt33 + img[row+i+3] * flt34 + img[row+i+4] * flt35;
	dst[idim4+i] += img[row+i] * flt21 + img[row+i+1] * flt22 + img[row+i+2] * flt23 + img[row+i+3] * flt24 + img[row+i+4] * flt25;
    }
    row += dim;

    // Third To Last
    for (i = 0; i < dim-4; i++){
	dst[idim2+i] += img[row+i] * flt51 + img[row+i+1] * flt52 + img[row+i+2] * flt53 + img[row+i+3] * flt54 + img[row+i+4] * flt55;
	dst[idim3+i] += img[row+i] * flt41 + img[row+i+1] * flt42 + img[row+i+2] * flt43 + img[row+i+3] * flt44 + img[row+i+4] * flt45;
	dst[idim4+i] += img[row+i] * flt31 + img[row+i+1] * flt32 + img[row+i+2] * flt33 + img[row+i+3] * flt34 + img[row+i+4] * flt35;
    }
    row += dim;

    // Second To Last
    for (i = 0; i < dim-4; i++){
	dst[idim3+i] += img[row+i] * flt51 + img[row+i+1] * flt52 + img[row+i+2] * flt53 + img[row+i+3] * flt54 + img[row+i+4] * flt55;
	dst[idim4+i] += img[row+i] * flt41 + img[row+i+1] * flt42 + img[row+i+2] * flt43 + img[row+i+3] * flt44 + img[row+i+4] * flt45;
    }
    row += dim;

    // Last Row
    for (i = 0; i < dim-4; i++){
	dst[idim4+i] += img[row+i] * flt51 + img[row+i+1] * flt52 + img[row+i+2] * flt53 + img[row+i+3] * flt54 + img[row+i+4] * flt55;
    }
}

/*********************************************************************
 * register_blur_functions - Register all of your different versions
 *     of the gaussian blur kernel with the driver by calling the
 *     add_blur_function() for each test function. When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.  
 *********************************************************************/

void register_blur_functions() 
{
    add_blur_function(&naive_blur, naive_blur_descr); 
    add_blur_function(&blur, blur_descr);
    /* ... Register additional test functions here */
}

