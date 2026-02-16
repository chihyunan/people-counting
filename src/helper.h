#include <stdint.h>
#include <stdio.h>
#include "time.h"
#include <sys/time.h>
void print_array_sh(short *array, int width, int height);
void print_array_c(uint8_t *array, int width, int height);
double performance_evaluation(int start_or_end);
void sh_array_to_string(short *raw_temp, char *buf, int array_len);
void c_array_to_string(unsigned char *raw_temp, char *buf, int array_len);