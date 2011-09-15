/*
*********************************************************************************************************
* Copyright (c) 2006, Eric Shufro <eric@shufro.com>
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
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                      uIP Generic Applicaiton Interface
*
* File  : uip_app_registration.h
* By    : Eric Shufro
* Date  : 05.11.2006
*********************************************************************************************************
*/

#ifndef uIP_APP_REG_H
#define uIP_APP_REG_H

/*
*********************************************************************************************************
*                                              DATATYPES
*********************************************************************************************************
*/

typedef            void      CPU_VOID;
typedef  unsigned  char      CPU_CHAR;                                  /*  8-bit character                                         */
typedef  unsigned  char      CPU_BOOLEAN;                               /*  8-bit boolean or logical                                */
typedef  unsigned  char      CPU_INT08U;                                /*  8-bit unsigned integer                                  */
typedef    signed  char      CPU_INT08S;                                /*  8-bit   signed integer                                  */
typedef  unsigned  int       CPU_INT16U;                                /* 16-bit unsigned integer                                  */
typedef    signed  int       CPU_INT16S;                                /* 16-bit   signed integer                                  */
typedef  unsigned  long      CPU_INT32U;                                /* 32-bit unsigned integer                                  */
typedef    signed  long      CPU_INT32S;                                /* 32-bit   signed integer                                  */

typedef            CPU_VOID    (*UIP_APP_FNCT_PTR)(CPU_VOID);           /* Function pointer type to a function in the form of       */
                                                                        /* void myFunc (void) which is used within uip_app_register */

/*
*********************************************************************************************************
*                                     uIP REGISTRATION SETUP MACROS
*********************************************************************************************************
*/

#define  UIP_APP_LIST_SIZE                    2                         /* Hold data for up to 10 registered uIP applications       */

#ifdef   UIP_APPCALL                                                    /* If UIP_APPCALL is already defined from another uIP App   */
#undef   UIP_APPCALL                                                    /* then undefine it and force uIP apps to register          */
#endif                                                                  /* See uip_app.c for a compelte description                 */

#define  UIP_APPCALL                    uip_tcp_app_exec                /* Use the generic application handler for TCP app dispatch */

#ifdef   UIP_UDP_APPCALL                                                /* If UIP_UDP_APPCALL is already defined from an uIP App    */
#undef   UIP_UDP_APPCALL                                                /* then undefine it and redefine it as specified below      */
#endif

#define  UIP_UDP_APPCALL                uip_udp_app_exec                /* Use the generic application handler for UDP app dispatch */

/*
*********************************************************************************************************
*                                           API DEFINES
*********************************************************************************************************
*/

#define  REG_NO_ERR        0
#define  REG_DUP_ENTRY 	   1
#define  REG_NULL_PNTR	   2
#define  REG_INV_PROTO	   3
#define  REG_INV_TYPE      4
#define  REG_FUNCT_EXISTS  5
#define  REG_LIST_FULL 	   6

#define  REG_TCP           0
#define  REG_UDP           1

#define  UNREG_NO_ERR      0
#define  UNREG_FUNCT_NR	   1
#define  UNREG_NULL_PNTR   2

#define  REG_ACTIVE        0
#define  REG_PASSIVE       1

#define  ENTRY_FREE        0
#define  ENTRY_OCCUPIED    1

/*
*********************************************************************************************************
*                                              PROTOTYPES
*********************************************************************************************************
*/

CPU_VOID   uip_app_list_init(CPU_VOID);
CPU_INT08U uip_app_register(UIP_APP_FNCT_PTR function, CPU_INT08U protocol, CPU_INT16U port, CPU_INT08U type);
CPU_INT08U uip_app_unregister(UIP_APP_FNCT_PTR function);

CPU_VOID   uip_tcp_app_exec(CPU_VOID);
CPU_VOID   uip_udp_app_exec(CPU_VOID);




#endif
