/*
 * Copyright (c) 2009, Pontus Oldberg.
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
#include "dm9000.h"
#include "pt.h"
#include "led.h"
#include "swtimers.h"
#include "iet_debug.h"

#include <string.h>

struct led led;

/**
 * Initialize LED settings
 */
void init_led(void)
{
  memset(&led, 0, sizeof(struct led));
  PT_INIT(&led.pt);
}

/**
 * This function will set the indicator led according to the supplied
 * status.
 */
void set_led(u8_t status, u8_t option) __reentrant
{
  A_(printf("Setting LED status to ");)
  switch (status)
  {
    case LED_ON:
      A_(printf("ON\r\n");)
      break;

    case LED_OFF:
      A_(printf("OFF\r\n");)
      led.command = 0;
      break;

    case LED_BLINK:
      A_(printf("BLINK\r\n");)
      led.command = 0x01;
      led.blink_rate = option;
      break;
  }
}

/**
 * Here's the led super duper task.
 * It will take care of blinking the high quality super duper front
 * indicator LED
 */
PT_THREAD(handle_led(struct led *ld) __reentrant)
{
  PT_BEGIN(&ld->pt);

  ld->timer = alloc_timer();
  A_(printf("LED Mgr: Using timer %d !\r\n", ld->timer);)

  while (1)
  {
    PT_WAIT_WHILE(&ld->pt, (ld->command == 0));
    A_(printf("LED command 0x%02x\r\n", ld->command);)

    set_timer(ld->timer, (counter_width)ld->blink_rate, NULL);
    PT_WAIT_UNTIL(&ld->pt, (get_timer(ld->timer) == 0x00));
    if (ld->command != 0)

    if (ld->command != 0)
    {
      set_timer(ld->timer, (counter_width)ld->blink_rate, NULL);
      PT_WAIT_UNTIL(&ld->pt, (get_timer(ld->timer) == 0x00));
    }
  }

  PT_END(&ld->pt);
}

/* End of file */
