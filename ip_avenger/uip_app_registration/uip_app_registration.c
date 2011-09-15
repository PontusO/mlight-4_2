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
* File  : uip_app_registration.c
* By    : Eric Shufro
* Date  : 05.11.2006
*
* Description : This file serves as an API extension to uIP. By registering your uIP
*               applications during runtime, you may avois having to hard code the call back
*               function associated with UIP_APPCALL(). This API also allows for a more
*               dynamic runtime environment with multiple uIP applications running together
*               on the same system.
*
* Notes       : 1) This file makes use of the uIP macro UIP_APPCALL and UIP_UDP_APPCALL
*                  (See uip_app_registration.h)
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                              INCLUDES
*********************************************************************************************************
*/
#include <stdio.h>
#include "system.h"
#include "uipopt.h"
#include "uip_app_registration.h"

/*
*********************************************************************************************************
*                                              GLOBALS
*********************************************************************************************************
*/

typedef struct uip_app_info {
    UIP_APP_FNCT_PTR  function;
    CPU_INT08U        protocol;
    CPU_INT16U        port;
    CPU_INT08U        type;
    CPU_INT08U        occupied;
};

static struct uip_app_info uipAppList[UIP_APP_LIST_SIZE];

/*
*********************************************************************************************************
*                                       uIP APP LIST INIT
*
* Description  : This function is calld by YOUR application when initializing uIP in order to
*                initialize the uIP Application List to a known, default state. (EMPTY). This
*                MUST be done before registering and initializing uIP applications such as HTTPd
*                and so forth.
*********************************************************************************************************
*/

CPU_VOID  uip_app_list_init (CPU_VOID)
{
    CPU_INT08U i;

    for (i = 0; i < UIP_APP_LIST_SIZE; i++) {
        uipAppList[i].occupied = ENTRY_FREE;
    }
}

/*
*********************************************************************************************************
*                                       uIP APP REGISTRATION
*
* Description :  This function is should be called in order to register new uIP connections
*                with the generic uIP application. This is necessary so that when data arrives
*                it can be redirected to the uIP work function for the application that
*                requires processing. For example, registering both the Netlogger and HTTPd
*                would allow both uIP applications to coexist in one large application.
*
* Arguments   :  function  :  A pointer to your uIP application's work function. Ex: httpd_appcall
*             :  protocol  :  0 = TCP, 1 = UDP, use the macros REG_TCP, and REG_UDP in uip_app.h
*                port      :  The local or remote port your application is using
*                type      :  For UDP and TCP, the connection type (Active or Passive). This is
*                             used to determine whether incomming data should have its
*                             lport or rport compared to the argument port.
*                             0 = REG_ACTIVE, 1 = REG_PASSIVE;
*
* Returns     :  0 = REG_NO_ERR       -  Registration successful
*                1 = REG_DUP_ENTRY    -  Registration failed, protocol and port already in use.
*                2 = REG_NULL_PNTR    -  Registration failed since the work function pointer is NULL
*                3 = REG_INV_PROTO    -  Registration failed since the protocol is not valid
*                4 = REG_INV_TYPE     -  Registration failed since the connection type is not valid
*                5 = REG_FUNCT_EXISTS -  Function is already registered! Try unregistering first.
*                6 = REG_LIST_FULL    -  The registration list is full and cannot accept another
*                                        entry. If you receive this, try increasing the size of
*                                        UIP_APP_LIST_SIZE from within app_cfg.h.
*
* Notes       :  1) The 'function' argument MUST be a pointer to a function of the form
*                   void myfunc (void) as specified in the uIP documentation for UIP_APPCALL.
*
*                2) The list is populated in reverse order, (UIP_APP_LIST_SIZE - 1) to 0. This is
*                   the result of using 1 loop to find an empty location, and record the last
*                   available location found. Since the loop goes from 0 to UIP_APP_LIST_SIZE,
*                   the freeLocation ends up being the last available location found.
*
*                3) Even though it is possible for applicatins to use the same rport (like 53 for resolv)
*                   it is not possible to distinguish between them since the lport is not known at the
*                   time of registration for an outbound (Active) connection. Therefore, ensure that only
*                    1 uIP app uses a single remote port at a time. You can register and unregister
*                   apps in order to ensure this.
*
*                4) If type is REG_ACTIVE,  then port represents the rport (remote port), See 3)
*                   If type is REG_PASSIVE, then port represents the lport (local / listening port)
*********************************************************************************************************
*/

CPU_INT08U  uip_app_register (UIP_APP_FNCT_PTR function, CPU_INT08U protocol, CPU_INT16U port, CPU_INT08U type)
{
    CPU_INT08U  i;
    CPU_INT08U  freeLocation;
    static char n = 0;

    freeLocation = UIP_APP_LIST_SIZE + 1;                               /* Assume there is no free location within the list         */

    if (function == NULL) {
        return (REG_NULL_PNTR);                                         /* If a NULL pointer was passed in, return an error         */
    }

    if ((protocol != REG_TCP) && (protocol != REG_UDP)) {
        return (REG_INV_PROTO);                                         /* Return an error if the protocol is invalid               */
    }

    if ((type != REG_TCP) && (type != REG_UDP)) {
        return (REG_INV_TYPE);                                          /* Return an error if the connection type is invalid        */
    }

    for (i = 0; i < UIP_APP_LIST_SIZE; i++) {                           /* Check list for function / protocol / port already in use */
        if (uipAppList[i].occupied == ENTRY_FREE) {
            freeLocation = i;                                           /* Store the index of an available location if found        */
        } else {            									                          /* Location is in use, check if protocol and port in use    */
            if (uipAppList[i].function == function) {
                return (REG_FUNCT_EXISTS);                              /* Return an error if the function already exists.          */
            }

            if ((uipAppList[i].protocol == protocol) &&
                (uipAppList[i].port == port)         &&
                (uipAppList[i].type == type)) {
                return (REG_DUP_ENTRY);                                 /* Return an error since the protocol and port are in use   */
            }
        }
    }

    if (freeLocation < UIP_APP_LIST_SIZE) {                             /* If we found an available location in the list            */
        uipAppList[freeLocation].function = function;                   /* Store this applicaitons function pointer in the list     */
        uipAppList[freeLocation].protocol = protocol;                   /* Store this applications protocol in the list             */
        uipAppList[freeLocation].port     = port;                       /* Store this applications distinguishing port in the list  */
        uipAppList[freeLocation].type     = type;                  		  /* Store this applications connection type                  */
        uipAppList[freeLocation].occupied = ENTRY_OCCUPIED;             /* Mark the entry as occupied */
        return (REG_NO_ERR);                                            /* Success. Return with no errors                           */
    } else {
        return (REG_LIST_FULL);                                         /* No free location found, return REG_LIST_FULL             */
    }
}

/*
*********************************************************************************************************
*                                       uIP APP UN-REGISTER
*
* Description  : This function is calld by YOUR application when either changing the listening port
*                for your  uIP application (followed by a new uip_app_register() call), or when
*                you no longer wish to receive data for a specific uIP application. (App Shut Down)
*
* Arguments    : function - A function pointer to your uIP application work function. This pointer must point
*                           to a function in the form of void myFund (void).
*
* Returns     :  0 = UNREG_NO_ERR     -  Unregistration successful
*                1 = UNREG_FUNCT_NR   -  Unregistration failed, function is not currently registered.
*                2 = UNREG_NULL_PNTR  -  Registration failed since the work function pointer is NULL
*********************************************************************************************************
*/

CPU_INT08U  uip_app_unregister (UIP_APP_FNCT_PTR function)
{
    CPU_INT08U i;

    if (function == NULL) {
        return (UNREG_NULL_PNTR);                                       /* Return an error if the function pointer is NULL          */
    }

    for (i = 0; i < UIP_APP_LIST_SIZE; i++) {
        if (uipAppList[i].function == function) {
            uipAppList[i].function  = NULL;                             /* If the function is found in the list, remove it          */
            return (UNREG_NO_ERR);                                      /* Unregistration successful, return no error               */
        }
    }

    return (UNREG_FUNCT_NR);                                            /* Unregistration failed, function is not registered        */
}

/*
*********************************************************************************************************
*                                       uIP TCP APP EXECUTE
*
* Description  : This function is calld by uIP via the UIP_APPCALL macro and is resonsible for
*                executing a registed uIP application work function when the destination port
*                number matches the registered local port number. In order for this to work, you
*                MUST register your uIP application by calling uip_app_register() after your uIP
*                application has been initialized and is ready to begin receiving data.
*                The UIP_APPCALL is defined in app_cfg.h.
*
* Notes        : This only processes TCP connections, see uip_udp_app_exec for UDP connection processing
*********************************************************************************************************
*/

CPU_VOID  uip_tcp_app_exec (CPU_VOID)
{
    CPU_INT16U i;


    for (i = 0; i < UIP_APP_LIST_SIZE; i++) {                           /* Check all slots in the uIP App List                      */
        if (uipAppList[i].type == REG_PASSIVE) {                        /* Check if we are looking for an lport (Passive Conn)      */
            if ((uipAppList[i].port == uip_conn->lport)     &&          /* If the port that data arrived on is in the uIP App List  */
                (uipAppList[i].protocol == REG_TCP)         &&          /* and the listed app is a TCP application, and the         */
                (uipAppList[i].function != NULL)) {                     /* function pointer is not NULL (App is registered)         */
                    uipAppList[i].function();                           /* Call the user's uIP App work function                    */
            }
        } else {
            if ((uipAppList[i].port == uip_conn->rport)     &&          /* else, we are looking for an rport match (Active Conn)    */
                (uipAppList[i].protocol == REG_TCP)         &&          /* and the listed app is a TCP application, and the         */
                (uipAppList[i].function != NULL)) {                     /* function pointer is not NULL (App is registered)         */
                    uipAppList[i].function();                           /* Call the user's uIP App work function                    */
            }
        }
    }
}

#if UIP_UDP != 0
/*
*********************************************************************************************************
*                                       uIP UDP APP EXECUTE
*
* Description  : This function is calld by uIP via the UIP_UDP_APPCALL macro and is resonsible for
*                executing a registed uIP application work function when the destination port
*                number matches the registered local port number. In order for this to work, you
*                MUST register your uIP application by calling uip_app_register() after your uIP
*                application has been initialized and is ready to begin receiving data.
*                The UIP_UDP_APPCALL is defined in app_cfg.h.
*
* Notes        : 1) This only processes UDP connections, see uip_tcp_app_exec for TCP connection processing
*********************************************************************************************************
*/

CPU_VOID  uip_udp_app_exec (CPU_VOID)
{
    CPU_INT16U i;

    for (i = 0; i < UIP_APP_LIST_SIZE; i++) {                           /* Check all slots in the uIP App List                      */
        if (uipAppList[i].type == REG_PASSIVE) {                        /* Check if we are looking for an lport (Passive Conn)      */
            if ((uipAppList[i].port == uip_udp_conn->lport) &&          /* If the port that data arrived on is in the uIP App List  */
                (uipAppList[i].protocol == REG_UDP)         &&          /* and the listed app is a UDP application, and the         */
                (uipAppList[i].function != NULL)) {                     /* function pointer is not NULL (App is registered)         */
                    uipAppList[i].function();                           /* Call the user's uIP App work function                    */
            }
        } else {
            if ((uipAppList[i].port == uip_udp_conn->rport) &&          /* else, we are looking for an rport match (Active Conn)    */
                (uipAppList[i].protocol == REG_UDP)         &&          /* and the listed app is a UDP application, and the         */
                (uipAppList[i].function != NULL)) {                     /* function pointer is not NULL (App is registered)         */
                    uipAppList[i].function();                           /* Call the user's uIP App work function                    */
            }
        }
    }
}
#endif

/* End of file */
