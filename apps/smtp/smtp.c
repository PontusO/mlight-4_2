/**
 * \addtogroup apps
 * @{
 */

/**
 * \defgroup smtp SMTP E-mail sender
 * @{
 *
 * The Simple Mail Transfer Protocol (SMTP) as defined by RFC821 is
 * the standard way of sending and transfering e-mail on the
 * Internet. This simple example implementation is intended as an
 * example of how to implement protocols in uIP, and is able to send
 * out e-mail but has not been extensively tested.
 */

/**
 * \file
 * SMTP example implementation
 * \author Adam Dunkels <adam@dunkels.com>
 */

/*
 * Copyright (c) 2004, Adam Dunkels.
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
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: smtp.c,v 1.4 2006/06/11 21:46:37 adam Exp $
 */
//#define PRINT_A

#include "uip.h"
#include "psock.h"
#include "smtp.h"
#include "parameters.h"
#include "iet_debug.h"
#include "smtp-strings.h"
#include "uip_app_registration.h"

#include <string.h>

struct smtp_state {
  struct psock psock;
  u8_t connected;
  u8_t state;
  char *to;
  char *cc;
  char *from;
  char *subject;
  char *msg;
  u16_t msglen;
  u16_t sentlen, textlen;
  u16_t sendptr;
  char inputbuffer[10];
};

struct smtp_state s;

static char *localhostname;
static uip_ipaddr_t smtpserver;

void encode64( const char *instr, char *outstr);

#define ISO_nl 0x0a
#define ISO_cr 0x0d

#define ISO_period 0x2e
#define ISO_space  0x20

#define ISO_2  0x32
#define ISO_3  0x33
#define ISO_4  0x34
#define ISO_5  0x35

static char b64[32];

void smtp_done(unsigned char error)
{
  IDENTIFIER_NOT_USED(error);
}

/*---------------------------------------------------------------------------*/
static
PT_THREAD(smtp_thread(void))
{
  PSOCK_BEGIN(&s.psock);

  PSOCK_READTO(&s.psock, ISO_nl);

  if(strncmp(s.inputbuffer, smtp_220, 3) != 0) {
    PSOCK_CLOSE(&s.psock);
    smtp_done(SMTP_UNEXPECTED_RESPONSE);
    PSOCK_EXIT(&s.psock);
  }

  if (!sys_cfg.authenabled) {
    PSOCK_SEND_STR(&s.psock, (char *)smtp_helo);
    PSOCK_SEND_STR(&s.psock, localhostname);
    PSOCK_SEND_STR(&s.psock, (char *)smtp_crnl);
  } else {
    PSOCK_SEND_STR(&s.psock, (char *)smtp_ehlo);
    PSOCK_SEND_STR(&s.psock, localhostname);
    PSOCK_SEND_STR(&s.psock, (char *)smtp_crnl);

    /* Read all 250 headers */
    do {
      PSOCK_READTO(&s.psock, ISO_nl);
    } while (strncmp(s.inputbuffer, smtp_250_dsn, 7) != 0);

    PSOCK_SEND_STR(&s.psock, smtp_auth);
    PSOCK_SEND_STR(&s.psock, (char *)smtp_crnl);

    PSOCK_READTO(&s.psock, ISO_nl);

    /* Continue with checking server response for auth string */
    if (strncmp(s.inputbuffer, smtp_334, 3) != 0) {
      s.inputbuffer[3] = 0;
      PSOCK_CLOSE(&s.psock);
      smtp_done(SMTP_UNEXPECTED_RESPONSE);
      PSOCK_EXIT(&s.psock);
    }

    /* Base 64 encode the username */
    encode64(sys_cfg.username, b64);
    PSOCK_SEND_STR(&s.psock, b64);
    PSOCK_SEND_STR(&s.psock, (char *)smtp_crnl);

    PSOCK_READTO(&s.psock, ISO_nl);

    /* Now wait for the password identifier */
    if (strncmp(s.inputbuffer, smtp_334, 3) != 0) {
      s.inputbuffer[3] = 0;
      PSOCK_CLOSE(&s.psock);
      smtp_done(SMTP_UNEXPECTED_RESPONSE);
      PSOCK_EXIT(&s.psock);
    }

    /* Base 64 encode the password */
    encode64(sys_cfg.password, b64);
    PSOCK_SEND_STR(&s.psock, b64);
    PSOCK_SEND_STR(&s.psock, (char *)smtp_crnl);

    /* Now wait for the authentication okay from the server */
    PSOCK_READTO(&s.psock, ISO_nl);

    if (strncmp(s.inputbuffer, smtp_235, 3) != 0) {
      s.inputbuffer[3] = 0;
      PSOCK_CLOSE(&s.psock);
      smtp_done(SMTP_AUTHENTICATION_FAILED);
      PSOCK_EXIT(&s.psock);
    }
  }

  /* Now take care of the actual mail */
  PSOCK_SEND_STR(&s.psock, (char *)smtp_mail_from);
  PSOCK_SEND_STR(&s.psock, s.from);
  /* We're doing an ugly duckling here, asuming that 8-bit mime is allowed */
//  PSOCK_SEND_STR(&s.psock, (char *)smtp_body);
  PSOCK_SEND_STR(&s.psock, (char *)smtp_crnl);

  PSOCK_READTO(&s.psock, ISO_nl);

  if(s.inputbuffer[0] != ISO_2) {
    PSOCK_CLOSE(&s.psock);
    smtp_done(SMTP_UNEXPECTED_RESPONSE);
    PSOCK_EXIT(&s.psock);
  }

  PSOCK_SEND_STR(&s.psock, (char *)smtp_rcpt_to);
  PSOCK_SEND_STR(&s.psock, s.to);
  PSOCK_SEND_STR(&s.psock, (char *)smtp_crnl);

  PSOCK_READTO(&s.psock, ISO_nl);

  if(s.inputbuffer[0] != ISO_2) {
    PSOCK_CLOSE(&s.psock);
    smtp_done(SMTP_UNEXPECTED_RESPONSE);
    PSOCK_EXIT(&s.psock);
  }

  if(s.cc != NULL) {
    PSOCK_SEND_STR(&s.psock, (char *)smtp_rcpt_to);
    PSOCK_SEND_STR(&s.psock, s.cc);
    PSOCK_SEND_STR(&s.psock, (char *)smtp_crnl);

    PSOCK_READTO(&s.psock, ISO_nl);

    if(s.inputbuffer[0] != ISO_2) {
      PSOCK_CLOSE(&s.psock);
      smtp_done(SMTP_UNEXPECTED_RESPONSE);
      PSOCK_EXIT(&s.psock);
    }
  }

  PSOCK_SEND_STR(&s.psock, (char *)smtp_data);

  PSOCK_READTO(&s.psock, ISO_nl);

  if(s.inputbuffer[0] != ISO_3) {
    PSOCK_CLOSE(&s.psock);
    smtp_done(SMTP_UNEXPECTED_RESPONSE);
    PSOCK_EXIT(&s.psock);
  }

  PSOCK_SEND_STR(&s.psock, (char *)smtp_to);
  PSOCK_SEND_STR(&s.psock, s.to);
  PSOCK_SEND_STR(&s.psock, (char *)smtp_crnl);

  if(s.cc != 0) {
    PSOCK_SEND_STR(&s.psock, (char *)smtp_cc);
    PSOCK_SEND_STR(&s.psock, s.cc);
    PSOCK_SEND_STR(&s.psock, (char *)smtp_crnl);
  }

  PSOCK_SEND_STR(&s.psock, (char *)smtp_from);
  PSOCK_SEND_STR(&s.psock, s.from);
  PSOCK_SEND_STR(&s.psock, (char *)smtp_crnl);

  PSOCK_SEND_STR(&s.psock, (char *)smtp_subject);
  PSOCK_SEND_STR(&s.psock, s.subject);
  PSOCK_SEND_STR(&s.psock, (char *)smtp_crnl);

  PSOCK_SEND_STR(&s.psock, (char *)smtp_content_type_text);
  PSOCK_SEND_STR(&s.psock, (char *)smtp_crnl);
  PSOCK_SEND_STR(&s.psock, (char *)smtp_crnl);

  PSOCK_SEND(&s.psock, s.msg, s.msglen);

  PSOCK_SEND_STR(&s.psock, (char *)smtp_crnlperiodcrnl);

  PSOCK_READTO(&s.psock, ISO_nl);

  if(s.inputbuffer[0] != ISO_2) {
    PSOCK_CLOSE(&s.psock);
    smtp_done(SMTP_UNEXPECTED_RESPONSE);
    PSOCK_EXIT(&s.psock);
  }

  PSOCK_SEND_STR(&s.psock, (char *)smtp_quit);
  smtp_done(SMTP_ERR_OK);

exit:
  PSOCK_END(&s.psock);
}
/*---------------------------------------------------------------------------*/
void
smtp_appcall(void)
{
  if(uip_closed()) {
    s.connected = 0;
    return;
  }
  if(uip_aborted() || uip_timedout()) {
    s.connected = 0;
    smtp_done(SMTP_FAILED_TO_CONNECT);
    return;
  }
  smtp_thread();
}
/*---------------------------------------------------------------------------*/
/**
 * Specificy an SMTP server and hostname.
 *
 * This function is used to configure the SMTP module with an SMTP
 * server and the hostname of the host.
 *
 * \param lhostname The hostname of the uIP host.
 *
 * \param server A pointer to a 4-byte array representing the IP
 * address of the SMTP server to be configured.
 */
void
smtp_configure(char *lhostname, void *server)
{
  localhostname = lhostname;
  uip_ipaddr_copy(smtpserver, server);
}
/*---------------------------------------------------------------------------*/
/**
 * Send an e-mail.
 *
 * \param to The e-mail address of the receiver of the e-mail.
 * \param cc The e-mail address of the CC: receivers of the e-mail.
 * \param from The e-mail address of the sender of the e-mail.
 * \param subject The subject of the e-mail.
 * \param msg The actual e-mail message.
 * \param msglen The length of the e-mail message.
 */
unsigned char
smtp_send(char *to, char *cc, char *from,
	  char *subject, char *msg, u16_t msglen)
{
  struct uip_conn *conn;

  B_(printf("Sending mejl\r\n");)
  conn = uip_connect(&smtpserver, htons(sys_cfg.smtp_port));
  if(conn == NULL) {
    B_(printf("Could not connect !\r\n");)
    return 0;
  }

  s.connected = 1;
  s.to = to;
  s.cc = cc;
  s.from = from;
  s.subject = subject;
  s.msg = msg;
  s.msglen = msglen;

  PSOCK_INIT(&s.psock, s.inputbuffer, sizeof(s.inputbuffer));

  return 1;
}
/*---------------------------------------------------------------------------*/
void
smtp_init(void)
{
  CPU_INT08U result;

  result = uip_app_register (smtp_appcall,
    REG_TCP,
    htons(sys_cfg.smtp_port),
    REG_ACTIVE);

  if (result != REG_NO_ERR)
  {
    A_(printf("Could not register the SMTP deamon, error %d\r\n", result);)
  }
  else
  {
    A_(printf("SMTP deamon registered\r\n");)
    s.connected = 0;
  }
}
/*---------------------------------------------------------------------------*/
/** @} */
/** @} */
