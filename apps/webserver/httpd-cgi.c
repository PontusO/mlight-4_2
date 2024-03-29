/**
 * \addtogroup httpd
 * @{
 */

/**
 * \file
 *         Web server script interface
 * \author
 *         Adam Dunkels <adam@sics.se>
 *
 */

/*
 * Copyright (c) 2001-2006, Adam Dunkels.
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
 * This file is part of the uIP TCP/IP stack.
 *
 * $Id: httpd-cgi.c,v 1.2 2006/06/11 21:46:37 adam Exp $
 *
 */

#include "uip.h"
#include "psock.h"
#include "httpd.h"
#include "httpd-cgi.h"
#include "httpd-fs.h"
#include "adc.h"
#include "parameters.h"
#include "light/lightlib.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <angle.h>

HTTPD_CGI_CALL(f_get_ip_num, "get-ip-num", get_ip_num);
HTTPD_CGI_CALL(f_get_temp, "get-temp", get_temp);
HTTPD_CGI_CALL(f_get_check_box, "get-check", get_check_box);
HTTPD_CGI_CALL(f_get_string, "get-string", get_string);
HTTPD_CGI_CALL(f_get_int, "get-int", get_int);
HTTPD_CGI_CALL(f_set_level, "set-level", set_level);
HTTPD_CGI_CALL(f_get_level, "get-level", get_level);
HTTPD_CGI_CALL(f_start_ramp, "start-ramp", start_ramp);
HTTPD_CGI_CALL(f_stop_ramp, "stop-ramp", stop_ramp);

static const struct httpd_cgi_call *calls[] = {
  &f_get_ip_num,
  &f_get_temp,
  &f_get_check_box,
  &f_get_string,
  &f_get_int,
  &f_set_level,
  &f_get_level,
  &f_start_ramp,
  &f_stop_ramp,
  NULL };

static char *ip_format = "%d.%d.%d.%d";
static char *error_string = "<ERROR ";

struct cgi_parameters cgi_parms_ctrl;

/*---------------------------------------------------------------------------*/
static
PT_THREAD(set_level(struct httpd_state *s, char *ptr) __reentrant)
{
  IDENTIFIER_NOT_USED(ptr);

  PSOCK_BEGIN(&s->sout);
  /* Make sure only 2 parameters were passed to this cgi */
  if (cgi_parms_ctrl.num_parms == 2) {
    /* Make sure only the correct parameters were passed */
    if (cgi_parms_ctrl.channel_updated && cgi_parms_ctrl.level_updated) {
      /* Validate parameters */
      if (cgi_parms_ctrl.channel < CFG_NUM_LIGHT_DRIVERS &&
          cgi_parms_ctrl.level <= 100) {
        struct led_lights *ll = ledlib_get_current();
        ledlib_set_light_percentage (ll, cgi_parms_ctrl.channel, cgi_parms_ctrl.level);
        sprintf((char *)uip_appdata, "<OK>");
      } else {
        sprintf((char *)uip_appdata, "%s01>", error_string);
      }
    } else {
      sprintf((char *)uip_appdata, "%s02>", error_string);
    }
  } else {
      sprintf((char *)uip_appdata, "%s03>", error_string);
  }
  PSOCK_SEND_STR(&s->sout, uip_appdata);
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(get_level(struct httpd_state *s, char *ptr) __reentrant)
{
  IDENTIFIER_NOT_USED(ptr);

  PSOCK_BEGIN(&s->sout);
  /* Make sure only 1 parameter was passed to this cgi */
  if (cgi_parms_ctrl.num_parms == 1) {
    /* Make sure only the correct parameters were passed */
    if (cgi_parms_ctrl.channel_updated) {
      /* Validate parameters */
      if (cgi_parms_ctrl.channel < CFG_NUM_LIGHT_DRIVERS) {
        struct led_lights *ll = ledlib_get_current();
        sprintf((char *)uip_appdata, "<%d>",
                ledlib_get_light_percentage (ll, cgi_parms_ctrl.channel));
      } else {
        sprintf((char *)uip_appdata, "%s01>", error_string);
      }
    } else {
      sprintf((char *)uip_appdata, "%s02>", error_string);
    }
  } else {
      sprintf((char *)uip_appdata, "%s03>", error_string);
  }
  PSOCK_SEND_STR(&s->sout, uip_appdata);
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(start_ramp(struct httpd_state *s, char *ptr) __reentrant)
{
  IDENTIFIER_NOT_USED(ptr);

  PSOCK_BEGIN(&s->sout);
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(stop_ramp(struct httpd_state *s, char *ptr) __reentrant)
{
  IDENTIFIER_NOT_USED(ptr);

  PSOCK_BEGIN(&s->sout);
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(nullfunction(struct httpd_state *s, char *ptr) __reentrant)
{
  IDENTIFIER_NOT_USED(ptr);

  PSOCK_BEGIN(&s->sout);
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
httpd_cgifunction
httpd_cgi(char *name) __reentrant
{
  const struct httpd_cgi_call **f;

  /* Find the matching name in the table, return the function. */
  for(f = calls; *f != NULL; ++f) {
    if(strncmp((*f)->name, name, strlen((*f)->name)) == 0) {
      return (*f)->function;
    }
  }
  return nullfunction;
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(get_temp(struct httpd_state *s, char *ptr) __reentrant)
{
  int temp = 0;
  int channel;

  PSOCK_BEGIN(&s->sout);

  while (*ptr != ' ')
    ptr++;
  ptr++;
  channel = atoi(ptr);

  switch (channel)
  {
    case 0:
    case 1:
    case 2:
//      temp = get_angle(channel);
      break;
  }

  sprintf((char *)uip_appdata, "%d.%d", temp / 100, abs(temp % 100));

  PSOCK_SEND_STR(&s->sout, uip_appdata);
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(get_ip_num(struct httpd_state *s, char *ptr) __reentrant)
{
  char ip_group;

  PSOCK_BEGIN(&s->sout);

  while (*ptr != ' ')
    ptr++;
  ptr++;
  /* The index of the color is in the webpage, go get it */
  ip_group = atoi(ptr);

  switch(ip_group)
  {
    case 1:
      /* Host IP */
      sprintf((char *)uip_appdata, ip_format,
        sys_cfg.ip_addr[0], sys_cfg.ip_addr[1],
        sys_cfg.ip_addr[2], sys_cfg.ip_addr[3]);
      break;

    case 2:
      /* Netmask */
      sprintf((char *)uip_appdata, ip_format,
        sys_cfg.netmask[0], sys_cfg.netmask[1],
        sys_cfg.netmask[2], sys_cfg.netmask[3]);
      break;

    case 3:
      /* Deafult router */
      sprintf((char *)uip_appdata, ip_format,
        sys_cfg.gw_addr[0], sys_cfg.gw_addr[1],
        sys_cfg.gw_addr[2], sys_cfg.gw_addr[3]);
      break;

    case 5:
      /* HTTP Port */
      sprintf((char *)uip_appdata, "%u", sys_cfg.http_port);
      break;

    default:
      sprintf((char *)uip_appdata, "Invalid IP group !");
      break;
  }

  PSOCK_SEND_STR(&s->sout, uip_appdata);

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(get_check_box(struct httpd_state *s, char *ptr) __reentrant)
{
  char check_box;

  PSOCK_BEGIN(&s->sout);

  while (*ptr != ' ')
    ptr++;
  ptr++;
  /* The index of the color is in the webpage, go get it */
  check_box = atoi(ptr);

  switch(check_box)
  {
#if 0
    case 0:
      check_box = sys_cfg.authenabled;
      break;

    case 1:
      check_box = sys_cfg.enable_stop_larm;
      break;

    case 2:
      check_box = sys_cfg.enable_fall_tube_alarm;
      break;

    case 3:
      check_box = sys_cfg.enable_sonar_larm;
      break;
#endif
    default:
      check_box = 0;
      break;
  }

  if (check_box)
    sprintf((char *)uip_appdata, "checked");
  else
    sprintf((char *)uip_appdata, " ");

  PSOCK_SEND_STR(&s->sout, uip_appdata);
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(get_int(struct httpd_state *s, char *ptr) __reentrant)
{
  char intno;
  int myint = 0;

  PSOCK_BEGIN(&s->sout);

  while (*ptr != ' ')
    ptr++;
  ptr++;
  intno = atoi(ptr);

  switch(intno)
  {
    /* emergency stop pin status */
    case 1:
//      myint = EMERGENCY_STOP;
      break;

    /* Handles the larm level on the pellets level alarm */
    case 2:
//      myint = sys_cfg.sonar_larm;
      break;

    /* Handles the bottom level value for the pellet container */
    case 3:
//      myint = sys_cfg.sonar_bottom;
      break;
  }

  sprintf((char *)uip_appdata, "%d", myint);

  PSOCK_SEND_STR(&s->sout, uip_appdata);
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(get_string(struct httpd_state *s, char *ptr) __reentrant)
{
  char stringno;
  char *string = NULL;

  PSOCK_BEGIN(&s->sout);

  while (*ptr != ' ')
    ptr++;
  ptr++;
  /* The index of the color is in the webpage, go get it */
  stringno = atoi(ptr);

  switch(stringno)
  {
    /* Node Name */
    case 4:
      string = sys_cfg.node_name;
      break;
  }

  if (string)
    sprintf((char *)uip_appdata, "%s", string);

  PSOCK_SEND_STR(&s->sout, uip_appdata);
  PSOCK_END(&s->sout);
}
/** @} */
