/*
 * Copyright (c) 2010, Pontus Oldberg.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include <system.h>
#include <adc.h>
#include <angle.h>
#include <math.h>
#include <stdio.h>

#define CENTER_POSITION_X     2736
#define CENTER_POSITION_Y     2767
#define CENTER_POSITION_Z     3352
#define CENTER_OFFSET_Z       2781
#define QUARTER_COUNT          580

#define ADC_CHANNEL_X         0
#define ADC_CHANNEL_Y         1
#define ADC_CHANNEL_Z         2

int get_angle(unsigned char channel)
{
  int center_pos = 0;
  float vec;
  float z;
  float result;
  float factor = 100;

  switch (channel)
  {
    case 0:
      center_pos = CENTER_POSITION_X;
      break;

    case 1:
      center_pos = CENTER_POSITION_Y;
      break;

    case 2:
      center_pos = CENTER_POSITION_Z;
      break;

    default:
      return 0;
  }

  /* Z axis is used as the reference vector when calculating the angle */
  z = (float)((int)adc_get_average(ADC_CHANNEL_Z)-CENTER_OFFSET_Z);
  /* Prepare the wanted first vector */
  vec = (float)((int)adc_get_average(channel) - center_pos);
  /* calculate the angle of the summed vector */
  result = atanf (vec / z) * 180 / PI;

  /* This is just for presentation */
  if (vec < z) {
    factor = -100;
  }

  return (int)(result*factor);
}
