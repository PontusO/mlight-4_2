/*
 * Copyright (c) 2008, Invector Embedded Technologies
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
 * Author:    Pontus Oldberg
 *
 */

//#define PRINT_B

#include "Flash_02x.h"
#include "parameters.h"
#include "system.h"
#include "iet_debug.h"
#include "util.h"

static int flash_chksum(u8_t command);
static void write_new_verification_word(int ver);

static void enter_flash_write(void)
{
   ENTER_CRITICAL_SECTION;

   FLSCL |= 0x01;  /* set FLWE(FLSCL.0) bit*/
   PSCTL |= 0x01;  /* set PSWE(PSCTL.0) bit*/
   PSCTL |= 0x04;  /* Point out the scratch pad area */
}

static void exit_flash_write(void)
{
   PSCTL &= ~0x04;  /* Point out the scratch pad area */
   PSCTL &= ~0x01; /* clear PSWE(PSCTL.0) bit*/
   FLSCL &= ~0x01;  /* clear FLWE(FLSCL.0) bit*/

   EXIT_CRITICAL_SECTION;
}

static void erase_config_area(void)
{
   char __xdata *touch_address = (char __xdata *)SCRATCH_PAGE_ADDRESS;

   ENTER_CRITICAL_SECTION;

   FLSCL |= 0x01;    /* set FLWE(FLSCL.0) bit*/
   PSCTL |= 0x03;    /* set PSWE(PSCTL.0)  and PSEE(PSCTL.1) bit*/
   PSCTL |= 0x04;  /* Point out the scratch pad area */

   *touch_address = 0x00;   /* Touch the desired bank */

   PSCTL &= ~0x04;  /* Point out the scratch pad area */
   PSCTL &= ~0x03;    /* clear PSWE(PSCTL.0)  and PSEE(PSCTL.1) bit*/
   FLSCL &= ~0x01;    /* clear FLWE(FLSCL.0) bit*/

   EXIT_CRITICAL_SECTION;
}

u8_t read_flash(u8_t n)
{
  char __code * __idata pread;
  u8_t byte;

  ENTER_CRITICAL_SECTION;

  pread = (char __code *)n;

  PSCTL |= 0x04;      /* Select scratch pad */
  byte = *pread;      /* Read the byte */
  PSCTL &= ~0x04;     /* Diselect scratch pad */

  EXIT_CRITICAL_SECTION;

  return byte;
}

/* save config to Flash */
void write_config_to_flash(void)
{
  __idata u16_t i;
  char __xdata *dest = (char __xdata *)SCRATCH_PAGE_ADDRESS;
  char __xdata *src = (char __xdata *)&sys_cfg;
  int chksum;

#if 0
  if (CONFIG_MEM_SIZE > SCRATCH_PAGE_ADDRESS)
  {
    A_(printf("The config struct is to large (%d bytes)\r\n", CONFIG_MEM_SIZE);)
    return;
  }
#endif

  /* You'd better have your contents backed up somewhere */
  erase_config_area();
  enter_flash_write();

  /* 4-byte block alignment */
  for(i=0; i<CONFIG_MEM_SIZE ; i++)
  {
    *dest++=*src++;
    RESET_WDT(WDT_RST);
  }

  exit_flash_write();

  // Get new check sum of entire block minus last word
  chksum = flash_chksum(FLASH_WO_VER);
  B_(printf("New FLASH checksum %d, verfication word %d\r\n", (int)chksum, (int)(-1 - chksum));)
  write_new_verification_word(-1 - chksum);
}

static void write_new_verification_word(int ver)
{
  char __xdata *dest = (char __xdata *)(SCRATCH_PAGE_ADDRESS + 0x7e);

  enter_flash_write();

  *dest++ = ver & 0xff;
  *dest = ver >> 8;

  exit_flash_write();
}

/**
 * This method calculates the checksum of the configuration block.
 *
 * @return the sum of the entire block minus the verification word.
 */
static int flash_chksum(u8_t command)
{
  int i;
  int chksum = 0;
  int loop;

  switch(command)
  {
    case FLASH_WO_VER:
      loop = SCRATCH_PAGE_SIZE-2;
      break;

    case ENTIRE_FLASH:
      loop = SCRATCH_PAGE_SIZE;
      break;

    default:
      loop = SCRATCH_PAGE_SIZE;
      break;
  }

  B_(printf("Iterating through %d bytes.\r\n", (int)loop);)
  for (i=0 ; i<loop ; i+=2)
  {
    chksum += read_flash(i) + (read_flash(i+1) << 8);
  }

  return chksum;
}

/**
 * This method will check that the checksum of the configuration flash block is
 * valid. If it is not valis it will replace the content with a valid default
 * content to ensure proper operation. It will return an error code to report that
 * it had to do the update.
 *
 * Ths checksum is calculated by adding every byte in the config block together.
 * This should return -1 as a result if all is OK. Anything else is incorrect.
 *
 * @return ERR_CONFIG_FLASH_NOT_VALID If the config  block was invalid and needed
 *         an update.
 **/

u8_t validate_config_flash(void)
{
  int chksum = 0;

  B_(printf("FLASH config structure is (%d bytes) !\n\r", CONFIG_MEM_SIZE);)

  chksum = flash_chksum(ENTIRE_FLASH);
  RESET_WDT(WDT_RST);
  B_(printf("FLASH checksum %d\r\n", (int)chksum);)

  /* Check if the content is valid */
  if (chksum != -1)
  {
    B_(printf("Checksum not -1, writing new checksum\r\n");)

    // Go and get the default values from flash
    load_default_config();
    RESET_WDT(WDT_RST);

    // Write it to flash
    write_config_to_flash();
    RESET_WDT(WDT_RST);
  }

  return 0;
}

// EOF
