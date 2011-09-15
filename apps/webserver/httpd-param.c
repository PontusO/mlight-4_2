/*
 * Copyright (c) 2008, Invector Embedded Technologies.
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack.
 *
 * Author: Pontus Oldberg <pontus@invector.se>
 *
 */

#include "system.h"
#include "uip.h"
#include "httpd-param.h"
#include "httpd-cgi.h"
#include "parameters.h"
#include "Flash_02x.h"
#include "iet_debug.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/*---------------------- Local data types -----------------------------------*/
struct parameter_table {
  const char *param;
  void (*function)(char *buffer) __reentrant;
};

/*---------------------- Local function prototypes --------------------------*/
static void set_ip(char *buffer) __reentrant;
static void set_netmask(char *buffer) __reentrant;
static void set_gateway(char *buffer) __reentrant;
static void set_smtp(char *buffer) __reentrant;
static void set_smtp_port(char *buffer) __reentrant;
static void set_email(char *buffer) __reentrant;
static void set_varmare(char *buffer) __reentrant;
static void set_min(char *buffer) __reentrant;
static void set_flakt(char *buffer) __reentrant;
static void set_max(char *buffer) __reentrant;
static void set_ejmin(char *buffer) __reentrant;
static void set_tidmin(char *buffer) __reentrant;
static void set_ejmax(char *buffer) __reentrant;
static void set_tidmax(char *buffer) __reentrant;
static void set_uname(char *buffer) __reentrant;
static void set_pword(char *buffer) __reentrant;
static void set_node(char *buffer) __reentrant;

/*---------------------- Local data ----------------------------------------*/
/* Form parameters that needs to be parsed */
static const struct parameter_table parmtab[] = {
    {
        "ip",
        set_ip
    },
    {
        "netmask",
        set_netmask
    },
    {
        "gateway",
        set_gateway
    },
    {
        "webport",
        set_webport
    },
    {
        "clear",
        set_clear
    },
    {
        "node",
        set_node
    },
    {
        "submit",
        set_submit
    },
    {
        "channel",
        cgi_set_channel
    },
    {
        "level",
        cgi_set_level
    },
    {
        "rampto",
        cgi_set_rampto
    },
    {
        "*",
        0
    }
};

__bit LSIG_Reset = 0;

/*---------------------- Local functions ------------------------------------*/
static char *skip_to_equal(char *buf)
{
  while (*buf != ISO_equal)
    buf++;
  buf++;

  return buf;
}
/*---------------------------------------------------------------------------*/
static char *skip_to_dot(char *buf)
{
  while (*buf != ISO_period)
    buf++;
  buf++;

  return buf;
}
/*---------------------------------------------------------------------------*/
static void parse_ip(char *buf, uip_ipaddr_t *ip)
{
  static u8_t octet[4];

  buf = skip_to_equal(buf);
  octet[0] = atoi(buf);
  buf = skip_to_dot(buf);
  octet[1] = atoi(buf);
  buf = skip_to_dot(buf);
  octet[2] = atoi(buf);
  buf = skip_to_dot(buf);
  octet[3] = atoi(buf);
  buf = skip_to_dot(buf);

  uip_ipaddr(ip, octet[0], octet[1], octet[2], octet[3]);
}
/*---------------------------------------------------------------------------*/
static int parse_temp(char *buf)
{
  int ret = 0;
  u8_t dp = 0;
  int sign = 1;

  /* Check if it's a negative number */
  if (*buf == '-') {
    sign = -1;
    buf++;
  }

  /* Loop through all digits, marking the dp if it's found */
  while(isdigit(*buf) || *buf == '.') {
    if (*buf == '.') {
      buf++;
      dp = 1;
      if (!isdigit(*buf)) {
        ret *= 10;
      }
    } else {
      ret = ret * 10 + (*buf - '0');
      buf++;
    }
  }

  if (dp)
    return ret * sign;
  else
    return ret * 10 * sign;
}
/*---------------------------------------------------------------------------*/
static void set_clear(char *buffer) __reentrant
{
  buffer = skip_to_equal(buffer);
  if (strncmp(buffer, "larmset", 7) == 0) {
//    sys_cfg.enable_stop_larm = 0;
//    sys_cfg.enable_fall_tube_alarm = 0;
//    sys_cfg.enable_sonar_larm = 0;
  } else if (strncmp(buffer, "mejlset", 7) == 0) {
//    sys_cfg.authenabled = 0;
  }
}
/*---------------------------------------------------------------------------*/
static void set_node(char *buffer) __reentrant
{
  buffer = skip_to_equal(buffer);

  memset(sys_cfg.node_name, 0, sizeof sys_cfg.node_name);
  strncpy(sys_cfg.node_name, buffer, sizeof sys_cfg.node_name);
}
/*---------------------------------------------------------------------------*/
static void set_ip(char *buffer) __reentrant
{
  uip_ipaddr_t ip;

  /* Parse IP address */
  parse_ip(buffer, &ip);

  /* Pack the result in the global parameter structure */
  sys_cfg.ip_addr[0] = htons(ip[0]) >> 8;
  sys_cfg.ip_addr[1] = htons(ip[0]) & 0xff;
  sys_cfg.ip_addr[2] = htons(ip[1]) >> 8;
  sys_cfg.ip_addr[3] = htons(ip[1]) & 0xff;

}
/*---------------------------------------------------------------------------*/
static void set_netmask(char *buffer) __reentrant
{
  uip_ipaddr_t ip;

  parse_ip(buffer, &ip);

  /* Pack the result in the global parameter structure */
  sys_cfg.netmask[0] = htons(ip[0]) >> 8;
  sys_cfg.netmask[1] = htons(ip[0]) & 0xff;
  sys_cfg.netmask[2] = htons(ip[1]) >> 8;
  sys_cfg.netmask[3] = htons(ip[1]) & 0xff;
}
/*---------------------------------------------------------------------------*/
static void set_gateway(char *buffer) __reentrant
{
  uip_ipaddr_t ip;

  parse_ip(buffer, &ip);

  /* Pack the result in the global parameter structure */
  sys_cfg.gw_addr[0] = htons(ip[0]) >> 8;
  sys_cfg.gw_addr[1] = htons(ip[0]) & 0xff;
  sys_cfg.gw_addr[2] = htons(ip[1]) >> 8;
  sys_cfg.gw_addr[3] = htons(ip[1]) & 0xff;

  /* Indicate that we want a reset after the parameters has
   * been written to flash */
  LSIG_Reset = 1;
}
/*---------------------------------------------------------------------------*/
static void set_webport(char *buffer) __reentrant
{
  unsigned int port;

  buffer = skip_to_equal(buffer);
  port = atoi(buffer);

  sys_cfg.http_port = port;
}
/*---------------------------------------------------------------------------*/
static void cgi_set_channel(char *buffer) __reentrant
{
  buffer = skip_to_equal(buffer);
  cgi_parms_ctrl.channel = atoi(buffer);
  cgi_parms_ctrl.channel_updated = 1;
  cgi_parms_ctrl.num_parms++;
}
/*---------------------------------------------------------------------------*/
static void cgi_set_level(char *buffer) __reentrant
{
  buffer = skip_to_equal(buffer);
  cgi_parms_ctrl.level = atoi(buffer);
  cgi_parms_ctrl.level_updated = 1;
  cgi_parms_ctrl.num_parms++;
}
/*---------------------------------------------------------------------------*/
static void cgi_set_rampto(char *buffer) __reentrant
{
  buffer = skip_to_equal(buffer);
  cgi_parms_ctrl.rampto = atoi(buffer);
  cgi_parms_ctrl.rampto_updated = 1;
  cgi_parms_ctrl.num_parms++;
}
/*---------------------------------------------------------------------------*/
static void set_submit(char *buffer) __reentrant
{
  IDENTIFIER_NOT_USED(buffer);

  B_(printf("Saving to parameters to FLASH\r\n");)
  write_config_to_flash();
  if (LSIG_Reset) {
    LSIG_Reset = 0;
    /* Reset the controller to reload network parameters */
    EA = 0;
    RSTSRC |= 0x10;
  }
}
/*---------------------------------------------------------------------------*/

static u8_t parse_expr(char *buf)
{
  struct parameter_table *tptr = (struct parameter_table *)parmtab;

  while (*tptr->param != '*')
  {
    if ((buf[strlen(tptr->param)] == '=') &&
       (strncmp(buf, tptr->param, strlen(tptr->param)) == 0))
    {
      /* Call the parameter set function */
      tptr->function(buf);
      return 1;
    }
    tptr++;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/

void parse_input(char *buf)
{
  static char token[64];
  char *tok;

  /* Search for query or end of line */
  while (*buf != ISO_query &&
         *buf != ISO_nl &&
         *buf != ISO_space)
    buf++;

  /* Return if no query is present */
  if (*buf == ISO_nl ||
      *buf == ISO_space)
    return;

  /* Clear the cgi control parameter structure */
  memset (&cgi_parms_ctrl, 0, sizeof cgi_parms_ctrl);

  while (*buf != ISO_space)
  {
    buf++;

    tok = token;
    while (*buf != ISO_space &&
           *buf != ISO_and)
    {
      *tok++ = *buf++;
    }
    *tok = 0;
    if (!parse_expr(token)) {
      B_(printf("Invalid parameter '%s'\r\n", token);)
    }
    RESET_WDT(WDT_RST);
  }
}

/* End of File */
