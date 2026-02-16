#include "helper.h"

void print_array_sh(short *array, int width, int height)
{
    for (int r = 0; r < height; r++)
    {
        for (int c = 0; c < width; c++)
        {
            int index = r * width + c;
            printf("%d ", array[index]);
        }
        printf("\n");
    }
}
void print_array_c(uint8_t *array, int width, int height)
{
    for (int r = 0; r < height; r++)
    {
        for (int c = 0; c < width; c++)
        {
            int index = r * width + c;
            if (array[index])
            {
                printf("%d ", array[index]);
            }
            else
            {
                printf("- ");
            }
        }
        printf("\n");
    }
}
/*0 is start, 1 is end, return eclipsed time in ms*/
double performance_evaluation(int start_or_end)
{
    static struct timeval tik, tok;
    if (start_or_end)
    {
        gettimeofday(&tok, NULL);
        double eclipsed_time_ms = (tok.tv_sec - tik.tv_sec) * 1000 + (tok.tv_usec - tik.tv_usec) / 1000.0;
        return eclipsed_time_ms;
    }
    else
    {
        gettimeofday(&tik, NULL);
        return -1;
    }
}

/* print a short array to a csv style string*/
void sh_array_to_string(short *raw_temp, char *buf, int array_len)
{
    int index = 0;
    for (int i = 0; i < array_len; i++)
    {
        index += sprintf(&buf[index], "%d", raw_temp[i]);
        if ((i + 1) % array_len == 0)
        {
            ;
        }
        else
        {
            index += sprintf(&buf[index], ",");
        }
    }
}

/* print a char array to a csv style string*/
void c_array_to_string(unsigned char *raw_temp, char *buf, int array_len)
{
    int index = 0;
    for (int i = 0; i < array_len; i++)
    {
        index += sprintf(&buf[index], "%d", raw_temp[i]);
        if ((i + 1) % array_len == 0)
        {
            ;
        }
        else
        {
            index += sprintf(&buf[index], ",");
        }
    }
}
