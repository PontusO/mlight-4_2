#ifndef _UTIL_H
#define _UTIL_H
#include "Uipopt.h"

/* some utilities used by the system */


/* convert two hex nibbles to 8-bit value */
u8_t hexstr2value(u8_t  *str);



/* convert a decimal string to one-byte value */
u8_t decimal2byte(u8_t  *str, u8_t *val);



/* convert a decimal string to 16-bit positive integer */
void decimal2word(u8_t  *str, u16_t *val);



/* search a value in an array */
u8_t search_value(u8_t *str, u8_t value, u8_t len);



/* parse ip address */
void parse_ip(u8_t *buf, u8_t *str);


/* parse mac address */
void parse_mac(u8_t * buf, u8_t * str);


/* URL string copy, additional \0 will be added to the destination string */
void url_str_cpy(u8_t *dest, u8_t *src, u16_t len);


/* delay ms*/
void wait_ms(u8_t count);

/* delay us */
void wait_us(u8_t count);

#define ISO_G        0x47
#define ISO_E        0x45
#define ISO_T        0x54
#define ISO_slash    0x2f
#define ISO_c        0x63
#define ISO_g        0x67
#define ISO_i        0x69
#define ISO_space    0x20
#define ISO_nl       0x0a
#define ISO_cr       0x0d
#define ISO_a        0x61
#define ISO_t        0x74
#define ISO_hash     0x23
#define ISO_period   0x2e
#define ISO_percent  0x25         /* % */
#define ISO_question 0x3f         /* ? */
#define ISO_equal    0x3d         /* = */
#define ISO_and      0x26         /* & */
#define ISO_minus    0x2d         /* - */
#define ISO_plus     0x2B         /* + */

#endif
