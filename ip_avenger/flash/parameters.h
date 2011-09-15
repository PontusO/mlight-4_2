#ifndef PARAMETERS_H_INCLUDED
#define PARAMETERS_H_INCLUDED

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

#include "system.h"

struct sys_config
{
  /* TCP/IP Settings */
  struct uip_eth_addr mac_addr;   /* MAC address of the node */
  u8_t  ip_addr[4];               /* IP address of the node */
  u8_t  netmask[4];               /* network mask of the node */
  u8_t  gw_addr[4];               /* the default gateway of the node */
  u16_t http_port;                /* Web server port */
  u8_t  node_name[12];            /* the name of the node */
#ifdef USE_WEB_AUTHETICATION
  char  username[30];             /* Username and password */
  char  password[10];
  char  authenabled;              /* Boolean indicating if authentication is used */
#endif
};

/* sys_cfg is the global parameter structure */
extern struct sys_config sys_cfg;

#define CONFIG_MEM_SIZE       (sizeof(struct sys_config))

/* This defines how many bytes there are in a flash page */
#define FLASH_PAGE_SIZE       512
#define LAST_PAGE_ADDRESS     (0x10000 - FLASH_PAGE_SIZE)

#define SCRATCH_PAGE_SIZE     128
#define SCRATCH_PAGE_ADDRESS  0x0000

#define ENTIRE_FLASH          1
#define FLASH_WO_VER          0

#endif // PARAMETERS_H_INCLUDED
