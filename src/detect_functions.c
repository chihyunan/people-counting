#include "detect.h"
#include "grideye_api_common.h"
static const char *TAG = "CV functions";

struct Filter
{
    int *kernel;
    int side;
    int *weight;
};

struct Filter *gkern_1d(double sigma)
{
    struct Filter *f = malloc(sizeof(struct Filter));
    if (f == NULL)
    {
        return NULL;
    }
    int l = round(2 * 3 * sigma);
    if (l % 2 == 0)
    {
        l = l - 1;
    }
    int radius = (l - 1) / 2;
    int *kernel = malloc(sizeof(int) * l);
    if (kernel == NULL)
    {
        free(f);
        return NULL;
    }
    int *weight = malloc(sizeof(int) * l);
    if (weight == NULL)
    {
        free(kernel);
        free(f);
        return NULL;
    }
    double sigma2 = sigma * sigma;
    int double_to_int_factor = 4 * radius;
    for (int x = -radius; x <= radius; x++)
    {
        int ref = x + radius;
        double value = exp(-0.5 * (x * x) / sigma2);
        value *= double_to_int_factor;
        kernel[ref] = round(value);
    }
    for (int i = 0; i <= radius; i++)
    {
        int weight_sum = 0;
        for (int x = 0; x <= radius + i; x++)
        {
            weight_sum += kernel[x];
        }
        weight[i] = weight_sum;
        weight[2 * radius - i] = weight_sum;
    }
    f->side = l;
    f->kernel = kernel;
    f->weight = weight;
    return f;
}

struct Filter *avg_kern1d(int side)
{
    if (side % 2 == 0)
    {
        side = side - 1;
        printf("update avg kernel side to %d\n", side);
    }
    struct Filter *f = malloc(sizeof(struct Filter));
    if (f == NULL)
    {
        return NULL;
    }
    int *vec = malloc(sizeof(int) * side);
    if (vec == NULL)
    {
        free(f);
        return NULL;
    }
    int *weight = malloc(sizeof(int) * side);
    if (weight == NULL)
    {
        free(f);
        free(vec);
        return NULL;
    }
    int radius = (side - 1) / 2;
    for (int i = 0; i <= radius; i++)
    {
        vec[i] = 1;
        vec[2 * radius - i] = 1;
        weight[i] = radius + i + 1;
        weight[2 * radius - i] = radius + i + 1;
    }
    f->side = side;
    f->kernel = vec;
    f->weight = weight;
    return f;
}

int max_of_array(short *array, int size)
{
    int loc = 0;
    for (int index = 1; index < size; index++)
    {
        if (array[index] > array[loc])
        {
            loc = index;
        }
    }
    return array[loc];
}

/*return the smallest non-zero value*/
int min_of_array(short *array, int size)
{
    int loc = 0;
    for (int index = loc; index < size; index++)
    {
        if (array[index] != 0)
        {
            loc = index;
            break;
        }
    }

    for (int index = loc; index < size; index++)
    {
        if (array[index] == 0)
            continue;
        if (array[index] < array[loc])
        {
            loc = index;
        }
    }
    return array[loc];
}

/*average of all non-zero values in an array*/
int avg_of_array(short *array, int size)
{
    int sum = 0;
    int count = 0;
    for (int index = 0; index < size; index++)
    {
        if (array[index] == 0)
            continue;
        sum += array[index];
        count++;
    }
    return sum / count;
}

/*std of all non-zero values in an array*/
int std_of_array(short *array, int size)
{
    int avg = avg_of_array(array, size);
    int var = 0;
    int count = 0;
    for (int index = 0; index < size; index++)
    {
        if (array[index] == 0)
            continue;
        // do not use pow in loop, too slow
        var += (array[index] - avg) * (array[index] - avg);
        count++;
    }
    var /= count;
    return (int)sqrt(var);
}

/*insert 9 pixels between each pixel*/
void interpolation71x71(short *input8x8, short *output71x71)
{
    uint8_t w = 8;
    uint8_t h = 8;

    uint16_t index;
    uint8_t col = 0;
    uint8_t row = 0;
    for (row = 0; row < h; row++)
    {
        for (col = 0; col < w; col++)
        {
            index = 10 * row * 71 + 10 * col;
            output71x71[index] = input8x8[row * w + col];
        }
        for (col = 0; col < w - 1; col++)
        {
            short left = input8x8[row * w + col];
            short right = input8x8[row * w + col + 1];
            short diff = right - left;
            for (int newcol = 1; newcol <= 9; newcol++)
            {
                index = 10 * row * 71 + 10 * col + newcol;
                output71x71[index] = left + (diff * newcol / 10);
            }
        }
    }
    for (row = 0; row < h - 1; row++)
    {
        for (col = 0; col < 71; col++)
        {
            short up = output71x71[10 * row * 71 + col];
            short down = output71x71[10 * (row + 1) * 71 + col];
            short diff = down - up;
            for (int newrow = 1; newrow <= 9; newrow++)
            {
                index = (10 * row + newrow) * 71 + col;
                output71x71[index] = up + (diff * newrow / 10);
            }
        }
    }
}

void convolution_x(short *image, short *output, int image_width, int image_height, struct Filter *f1d)
{
    int radius = (f1d->side - 1) / 2;
    for (int row = 0; row < image_height; row++)
    {
        for (int col = 0; col < image_width; col++)
        {
            int index = row * image_width + col;
            unsigned int result = 0;
            for (int x = -radius; x <= radius; x++)
            {
                bool out_of_boundary = ((col + x) < 0) || ((col + x) >= image_width);
                if (out_of_boundary)
                    continue;
                result += image[index + x] * f1d->kernel[x + radius];
            }
            if (col < radius)
            {
                result /= f1d->weight[col];
            }
            else if (col > (image_width - radius - 1))
            {
                result /= f1d->weight[image_width - col - 1];
            }
            else
            {
                result /= f1d->weight[radius];
            }

            output[index] = result;
        }
    }
}

void convolution_y(short *image, short *output, int image_width, int image_height, struct Filter *f1d)
{
    int radius = (f1d->side - 1) / 2;
    for (int row = 0; row < image_height; row++)
    {
        for (int col = 0; col < image_width; col++)
        {
            int index = row * image_width + col;
            unsigned int result = 0;
            for (int x = -radius; x <= radius; x++)
            {
                bool out_of_boundary = ((row + x) < 0) || ((row + x) >= image_height);
                if (out_of_boundary)
                    continue;
                result += image[index + x * image_width] * f1d->kernel[x + radius];
            }
            if (row < radius)
            {
                result /= f1d->weight[row];
            }
            else if (row > (image_height - radius - 1))
            {
                result /= f1d->weight[image_height - row - 1];
            }
            else
            {
                result /= f1d->weight[radius];
            }

            output[index] = result;
        }
    }
}

void grayscale_thresholding(short *image, short *output, int image_size, short *threshold, bool broadcast)
{
    for (int index = 0; index < image_size; index++)
    {
        output[index] = 0;
    }
    if (broadcast)
    {
        short th = threshold[0];
        for (int index = 0; index < image_size; index++)
        {
            if (image[index] < th)
                continue;
            output[index] = image[index];
        }
    }
    else
    {
        for (int index = 0; index < image_size; index++)
        {
            if (image[index] < threshold[index])
                continue;
            output[index] = image[index];
        }
    }
}

void binary_thresholding(short *image, uint8_t *output, int image_size, short *threshold, bool broadcast)
{

    if (broadcast)
    {
        short th = threshold[0];
        for (int index = 0; index < image_size; index++)
        {
            output[index] = (bool)(image[index] >= th);
        }
    }
    else
    {
        for (int index = 0; index < image_size; index++)
        {
            output[index] = (bool)(image[index] >= threshold[index]);
        }
    }
}

void summed_area_table(short *image, unsigned int *output, int image_width, int image_height)
{
    output[0] = image[0];
    for (int col = 1; col < image_width; col++)
    {
        output[col] = image[col] + output[col - 1];
    }
    for (int row = 1; row < image_height; row++)
    {
        output[row * image_width] = image[row * image_width] + output[(row - 1) * image_width];
    }
    for (int row = 1; row < image_height; row++)
    {
        for (int col = 1; col < image_width; col++)
        {
            int index = row * image_width + col;
            int left = index - 1;
            int up = index - image_width;
            int upleft = up - 1;
            output[index] = image[index] + output[up] + output[left] - output[upleft];
        }
    }
}

void average_of_area(unsigned int *sum_table, short *output, int image_width, int image_height, int side)
{
    if (side % 2 == 0)
    {
        side = side - 1;
        printf("avg from sum table, update side to %d\n", side);
    }
    int radius = (side - 1) / 2;
    for (int row = 0; row < image_height; row++)
    {
        int north = (row - radius - 1) < 0 ? -1 : (row - radius - 1);
        int south = (row + radius) >= image_height ? (image_height - 1) : (row + radius);
        for (int col = 0; col < image_width; col++)
        {
            int west = (col - radius - 1) < 0 ? -1 : (col - radius - 1);
            int east = (col + radius) >= image_width ? (image_width - 1) : (col + radius);

            unsigned int nw = ((north < 0) || (west < 0)) ? 0 : sum_table[north * image_width + west];
            unsigned int ne = (north < 0) ? 0 : sum_table[north * image_width + east];
            unsigned int sw = (west < 0) ? 0 : sum_table[south * image_width + west];
            unsigned int se = sum_table[south * image_width + east];
            unsigned int avg = se - sw - ne + nw;
            avg /= (south - north) * (east - west);
            output[row * image_width + col] = avg;
        }
    }
}

void binary_extract_holes(uint8_t *mask, uint8_t *outmask, int width, int height, uint8_t *holder, uint16_t *bg_search_list)
{
    int im_addborder_length = (width + 2) * (height + 2);
    for (int i = 0; i < im_addborder_length; i++)
    {
        holder[i] = 0;
    }
    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            int index_in_holder = (row + 1) * (width + 2) + (col + 1);
            holder[index_in_holder] = (mask[row * width + col] != 0x00);
        }
    }
    {
        bg_search_list[0] = 0; //set flood seed to upper left corner in padded mask
        /* we are sure that the most upper left pixel is 0, 
        so count it as the first bg pixel 
        and invert it so that it will not be counted again*/
        holder[0] = 1;
        int bg_counted = 0;
        int bg_tocount = 1;
        while (bg_counted < bg_tocount)
        {
            int index = bg_search_list[bg_counted];
            int row = index / (width + 2);
            int col = index % (width + 2);
            if ((row - 1) >= 0)
            {
                int index_up = index - (width + 2);
                if (holder[index_up] == 0)
                {
                    holder[index_up] = 1;
                    bg_search_list[bg_tocount] = index_up;
                    bg_tocount++;
                }
            }
            if ((row + 1) < (height + 2))
            {
                int index_down = index + (width + 2);
                if (holder[index_down] == 0)
                {
                    holder[index_down] = 1;
                    bg_search_list[bg_tocount] = index_down;
                    bg_tocount++;
                }
            }
            if ((col - 1) >= 0)
            {
                int index_left = index - 1;
                if (holder[index_left] == 0)
                {
                    holder[index_left] = 1;
                    bg_search_list[bg_tocount] = index_left;
                    bg_tocount++;
                }
            }
            if ((col + 1) < (width + 2))
            {
                int index_right = index + 1;
                if (holder[index_right] == 0)
                {
                    holder[index_right] = 1;
                    bg_search_list[bg_tocount] = index_right;
                    bg_tocount++;
                }
            }
            bg_counted++;
        }
    }
    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            int index = row * width + col;
            int index_in_holder = (row + 1) * (width + 2) + (col + 1);
            outmask[index] = (holder[index_in_holder] ^ 0x01);
        }
    }
}

/*******************************************************************************
 Copyright (C) <2015>, <Panasonic Corporation>
All rights reserved.
Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1.	Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2.	Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3.	The name of copyright holders may not be used to endorse or promote products derived from this software without specific prior written permission.
4.	This software code may only be redistributed and used in connection with a grid-eye product.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS �AS IS� AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR POFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND OR ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY; OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those of the authors and should not be interpreted as representing official policies, either expressed or implied, of the FreeBSD Project. 

 ******************************************************************************/
UCHAR ucAMG_PUB_ODT_CalcDataLabeling8(UCHAR ucWidth, UCHAR ucHeight, UCHAR ucMark, USHORT usArea, UCHAR *pucImg, USHORT *pusSearchList)
{
	USHORT usImg = 0;
	USHORT usSize = ucWidth * ucHeight;
	UCHAR ucDetectNum = 0;

	for (usImg = 0; usImg < usSize; usImg++)
	{
		UCHAR ucLabelNum = 0;
		USHORT usIndex = 0;
		USHORT usIndexAdd = 0;

		if (ucMark == pucImg[usImg])
		{
			pucImg[usImg] = 0;
			pusSearchList[usIndex] = usImg;
			usIndexAdd = 1;
		}

		while (usIndex < usIndexAdd)
		{
			UCHAR ucX = pusSearchList[usIndex] % ucWidth;
			UCHAR ucY = pusSearchList[usIndex] / ucWidth;
			{
				if (0 <= (ucY - 1)) // 不是第一行
				{
					USHORT usCheckIndex = (ucX) + (ucY - 1) * ucWidth;
					if (ucMark == pucImg[usCheckIndex]) // 看上面一个像素是不是物体
					{
						pucImg[usCheckIndex] = 0;
						pusSearchList[usIndexAdd++] = usCheckIndex;
					}
				}
				if (ucHeight > (ucY + 1)) //不是最后一行
				{
					USHORT usCheckIndex = (ucX) + (ucY + 1) * ucWidth;
					if (ucMark == pucImg[usCheckIndex]) // 看下面一个像素是不是物体
					{
						pucImg[usCheckIndex] = 0;
						pusSearchList[usIndexAdd++] = usCheckIndex;
					}
				}
			}
			if (0 <= (ucX - 1)) //不是最左
			{
				{
					USHORT usCheckIndex = (ucX - 1) + (ucY)*ucWidth; //正左边
					if (ucMark == pucImg[usCheckIndex])
					{
						pucImg[usCheckIndex] = 0;
						pusSearchList[usIndexAdd++] = usCheckIndex;
					}
				}
				if (0 <= (ucY - 1))
				{
					USHORT usCheckIndex = (ucX - 1) + (ucY - 1) * ucWidth; //左上
					if (ucMark == pucImg[usCheckIndex])
					{
						pucImg[usCheckIndex] = 0;
						pusSearchList[usIndexAdd++] = usCheckIndex;
					}
				}
				if (ucHeight > (ucY + 1)) //左下
				{
					USHORT usCheckIndex = (ucX - 1) + (ucY + 1) * ucWidth;
					if (ucMark == pucImg[usCheckIndex])
					{
						pucImg[usCheckIndex] = 0;
						pusSearchList[usIndexAdd++] = usCheckIndex;
					}
				}
			}
			if (ucWidth > (ucX + 1)) //不是最右
			{
				{
					USHORT usCheckIndex = (ucX + 1) + (ucY)*ucWidth; //正右边
					if (ucMark == pucImg[usCheckIndex])
					{
						pucImg[usCheckIndex] = 0;
						pusSearchList[usIndexAdd++] = usCheckIndex;
					}
				}
				if (0 <= (ucY - 1))
				{
					USHORT usCheckIndex = (ucX + 1) + (ucY - 1) * ucWidth; //右上
					if (ucMark == pucImg[usCheckIndex])
					{
						pucImg[usCheckIndex] = 0;
						pusSearchList[usIndexAdd++] = usCheckIndex;
					}
				}
				if (ucHeight > (ucY + 1))
				{
					USHORT usCheckIndex = (ucX + 1) + (ucY + 1) * ucWidth; //右下
					if (ucMark == pucImg[usCheckIndex])
					{
						pucImg[usCheckIndex] = 0;
						pusSearchList[usIndexAdd++] = usCheckIndex;
					}
				}
			}
			usIndex++;
		}
		
		if (usIndex <= usArea) //小于三个扩容后像素则认为是没有检测到物体
		{
			ucLabelNum = 0;
		}
		else
		{
			ucDetectNum++;
			ucLabelNum = ucDetectNum;
		}

		{
			USHORT usImg2 = 0;
			for (usImg2 = 0; usImg2 < usIndex; usImg2++)
			{
				pucImg[pusSearchList[usImg2]] = ucLabelNum;
			}
		}
	}

	return (ucDetectNum);
}