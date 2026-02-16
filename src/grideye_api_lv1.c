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

/*******************************************************************************
	include file
*******************************************************************************/
#include "grideye_api_lv1.h"
#include <stdbool.h>
#include <stdio.h>
// Add these at the top of the file, after the includes
typedef int esp_err_t;
#define ESP_OK 0
#define ESP_FAIL -1

/*******************************************************************************
	public method
 ******************************************************************************/

/*------------------------------------------------------------------------------
	Read data form I2C bus.
------------------------------------------------------------------------------*/
esp_err_t bAMG_PUB_I2C_Read(UCHAR ucI2cAddr, UCHAR ucRegAddr, UCHAR ucSize, UCHAR *ucDstAddr)
{
	
    return (ESP_OK); 
}
/*
#if defined(MCU_TEST)
	{
		extern esp_err_t i2c_master_read_slave(UCHAR, UCHAR, UCHAR *, UCHAR);
		return (i2c_master_read_slave(ucI2cAddr, ucRegAddr, ucDstAddr, ucSize));
	}
#else  /* !defined(MCU_TEST) */
	//{
	//	return (1);
	//}
//#endif /*  defined(MCU_TEST) */
//} */

/*------------------------------------------------------------------------------
	Convert thermistor register value.
------------------------------------------------------------------------------*/
short shAMG_PUB_TMP_ConvThermistor(UCHAR aucRegVal[2])
{
	short shVal = ((short)(aucRegVal[1] & 0x07) << 8) | aucRegVal[0];

	if (0 != (0x08 & aucRegVal[1]))
	{
		shVal *= -1;
	}

	shVal *= 16;

	return (shVal);
}

/*------------------------------------------------------------------------------
	Convert temperature register value for 1 pixel.
------------------------------------------------------------------------------*/
short shAMG_PUB_TMP_ConvTemperature(UCHAR aucRegVal[2])
{
	short shVal = ((short)(aucRegVal[1] & 0x07) << 8) | aucRegVal[0];

	if (0 != (0x08 & aucRegVal[1]))
	{
		shVal -= 2048;
	}

	shVal *= 64;

	return (shVal);
}

/*------------------------------------------------------------------------------
	Convert temperature register value for 64 pixel.
------------------------------------------------------------------------------*/
void vAMG_PUB_TMP_ConvTemperature64(UCHAR *pucRegVal, short *pshVal)
{
	UCHAR ucCnt = 0;

	for (ucCnt = 0; ucCnt < SNR_SZ; ucCnt++)
	{
		pshVal[ucCnt] = shAMG_PUB_TMP_ConvTemperature(pucRegVal + ucCnt * 2);
	}
}

/*------------------------------------------------------------------------------
	Convert value.
------------------------------------------------------------------------------*/
short shAMG_PUB_CMN_ConvFtoS(float fVal)
{
	return ((fVal > 0) ? (short)(fVal * 256 + 0.5) : (short)(fVal * 256 - 0.5));
}

/*------------------------------------------------------------------------------
	Convert value.
------------------------------------------------------------------------------*/
float fAMG_PUB_CMN_ConvStoF(short shVal)
{
	return ((float)shVal / 256);
}
/**
 * @brief read 128 register value, and convert to 64 pixel values, the temperature
 * value is .C degree multipled by 256 
 */
esp_err_t read_pixels(short *raw_temp)
{

	UCHAR aucTmpBuf[GRIDEYE_REGSZ_TMP];

	/* Get temperature register value. */
	if (ESP_FAIL == bAMG_PUB_I2C_Read(GRIDEYE_ADR, GRIDEYE_REG_TMP00, GRIDEYE_REGSZ_TMP, aucTmpBuf))
	{
		return (ESP_FAIL);
	}

	/* Convert temperature register value. */
	vAMG_PUB_TMP_ConvTemperature64(aucTmpBuf, raw_temp);

	return (ESP_OK);
}
/**
 * @brief return on-board thermistor temperature reading, multiplied by 256
 * @param short *thermistor 
 */ 
esp_err_t read_thermistor(short *thermistor_temp)
{
	UCHAR aucThsBuf[GRIDEYE_REGSZ_THS];
	/* Get thermistor register value. */
	if (ESP_FAIL == bAMG_PUB_I2C_Read(GRIDEYE_ADR, GRIDEYE_REG_THS00, GRIDEYE_REGSZ_THS, aucThsBuf))
	{
		return (ESP_FAIL);
	}

	/* Convert thermistor register value. */
	*thermistor_temp = shAMG_PUB_TMP_ConvThermistor(aucThsBuf);

	return (ESP_OK);
}

void print_pixels_to_serial(short *raw_temp, bool print_float)
{
	printf("[\n");
	for (int i = 1; i <= SNR_SZ; i++)
	{
		if (print_float)
		{
			printf("%.2f", ((float)raw_temp[i - 1] / CONVERT_FACTOR));
		}
		else
		{
			printf("%d", raw_temp[i - 1]);
		}
		if (i != SNR_SZ)
			printf(", ");
		if (i % 8 == 0 && i != SNR_SZ)
			printf("\n");
		if (i % 8 == 0 && i == SNR_SZ)
			printf("\n]");
	}
	printf("\n");
}