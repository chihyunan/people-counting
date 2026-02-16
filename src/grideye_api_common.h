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
#ifndef __GRIDEYE_API_COMMON_H
#define __GRIDEYE_API_COMMON_H

/*******************************************************************************
	macro definition
*******************************************************************************/
#define MCU_TEST
/* Grid-EYE's I2C slave address */
#define GRIDEYE_ADR (0x69) /* AD_SELECT pin connect to VDD		*/

/* Grid-EYE's register address */
#define GRIDEYE_REG_THS00 (0x0E) /* head address of thermistor  resister	*/
#define GRIDEYE_REG_TMP00 (0x80) /* head address of temperature resister	*/

/* Grid-EYE's register size */
#define GRIDEYE_REGSZ_THS (0x02) /* size of thermistor  resister		*/
#define GRIDEYE_REGSZ_TMP (0x80) /* size of temperature resister		*/

/* Grid-EYE's number of pixels */
#define SNR_SZ_X (8)
#define SNR_SZ_Y (8)
#define SNR_SZ (SNR_SZ_X * SNR_SZ_Y)

/*data precision*/
#define GRIDEYE_PRECISION_THS .0625
#define GRIDEYE_PRECISION_TMP .25
#define CONVERT_FACTOR 256

/*******************************************************************************
	type definition
*******************************************************************************/
typedef unsigned char BOOL;
typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef unsigned long ULONG;
typedef signed char CHAR;

#endif /* __GRIDEYE_API_COMMON_H */
/*******************************************************************************
 * Copyright(C) by 2015 Panasonic Corporation.
 ******************************************************************************/
