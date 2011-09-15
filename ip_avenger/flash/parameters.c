/*
 * Copyright (c) 2008, Pontus Oldberg.
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

//#define PRINT_B

#include <string.h>
#include <stdlib.h>

#include "flash_02x.h"
#include "parameters.h"
#include "iet_debug.h"

/* this is the default config */
const struct sys_config default_cfg = {
   /* offset=0, length=6, the MAC address of the node */
  { { 0x00, 0x1a, 0xb2, 0xc9, 0x0a, 0x00 } },
  {192, 168, 0, 11},  /* offset=6, length=4, the IP address of the node */
  {255, 255, 255, 0}, /* offset=10, length=4, the network mask of the node */
  {192, 168, 0, 1},   /* offset=14, length=4, the default gateway of the node */
  (u16_t)80,          /* Default web server port */
  { "LED_1",0,0,0,0,0,0,0}, /* The name of the node */
};

/* This is the config in the RAM */
struct sys_config sys_cfg;

/* this section does not apply when the scratch pad flash is used */
#ifndef USE_SCRATCH_PAD_FLASH
/* This is the config saved in the FLASH.
We put it in the last page of the 64K flash. (BANK0 + COBANK1).
According to the SDCC manual section 3.5, The compiler won't generate the code
for a variable or const declared using absolute addressing. */
static const __at(LAST_PAGE_ADDRESS) u8_t saved_cfg[FLASH_PAGE_SIZE];
#endif

/* load the system config from the flash when the system is powered on. */
void load_sys_config(void)
{
  u8_t i;

  B_(printf("Entering load_sys_config\r\n");)
  /* copy flash config from flash to xram*/
  for (i=0;i<CONFIG_MEM_SIZE;i++)
  {
    *((char*)sys_cfg+i) = read_flash(i);
  }

  uip_setethaddr(sys_cfg.mac_addr);
  /* A special setting that allows multiple units to be on the same network */
  uip_ethaddr.addr[5] = sys_cfg.ip_addr[3];

   uip_sethostaddr(sys_cfg.ip_addr);
   uip_setnetmask(sys_cfg.netmask);
   uip_setdraddr(sys_cfg.gw_addr);
}

/* this function simply loads the default configuration values
 * into the global configuration structure
 */
void load_default_config(void)
{
  memcpy(&sys_cfg, &default_cfg, CONFIG_MEM_SIZE);
}

void sys_getethaddr(struct uip_eth_addr *addr)
{
  // get the ethernet address
  memcpy(addr, &sys_cfg.mac_addr, sizeof(struct uip_eth_addr));
}

/* End of file */
