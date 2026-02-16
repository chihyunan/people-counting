#include "stdbool.h"
#include "math.h"
#include "stdio.h"
#include "stdlib.h"
#include "grideye_api_common.h"
#include "assert.h"
#include <stdint.h>
#include "macro.h"

typedef struct Filter Filter;
typedef int (*pool_function_t)(short[], int);

int blob_detection(short *raw, uint8_t *result,short temp_min_thres);

/*API functions*/
void image_copy(short *src, short *dst, int size);
void mask_copy(uint8_t *src, uint8_t *dst, int size);
int labeling8(uint8_t *mask, int width, int height);
bool average_filter(short *image,int width, int height, int side);
bool binary_fill_holes(uint8_t *mask,int width, int height);
bool discrete_convolution_2d_seperable(short *image,int width,int height,Filter *fx, Filter *fy);

/*low level functions*/
struct Filter *gkern_1d(double sigma);
struct Filter *avg_kern1d(int side);

int max_of_array(short *array, int size);
int min_of_array(short *array, int size);
int avg_of_array(short *array, int size);
int std_of_array(short *array, int size);

void interpolation71x71(short *input8x8, short *output71x71);
void convolution_x(short *image, short *output, int image_width, int image_height, struct Filter *f1d);
void convolution_y(short *image, short *output, int image_width, int image_height, struct Filter *f1d);
void grayscale_thresholding(short *image, short *output, int image_size, short *threshold, _Bool broadcast);
void binary_thresholding(short *image, uint8_t *output, int image_size, short *threshold, bool broadcast);
void summed_area_table(short *image, unsigned int *output, int image_width, int image_height);
void average_of_area(unsigned int *sum_table, short *output, int image_width,int image_height,int side);
void binary_extract_holes(uint8_t *mask, uint8_t *outmask, int width, int height, uint8_t *holder, uint16_t *bg_search_list);
UCHAR ucAMG_PUB_ODT_CalcDataLabeling8( UCHAR ucWidth, UCHAR ucHeight, UCHAR ucMark, USHORT usArea, UCHAR* pucImg, USHORT* pusSearchList);
