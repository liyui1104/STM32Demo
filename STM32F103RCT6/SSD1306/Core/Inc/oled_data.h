/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __OLED_DATA_H__
#define __OLED_DATA_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include "main.h"

/* Macros --------------------------------------------------------------------*/

#define OLED_CHARSET  0

#if OLED_CHARSET == 0
  #define OLED_CHARSET_UTF8    // Define the character set as UTF-8
#elif OLED_CHARSET == 1
  #define OLED_CHARSET_GB2312  // Define the character set as GB2312
#endif

/* Data Type Definitions -----------------------------------------------------*/

/* Basic unit of Chinese character font*/
typedef struct 
{
#ifdef OLED_CHARSET_UTF8
    char index[5];  // Chinese character index, with a space of 5 bytes
#endif
#ifdef OLED_CHARSET_GB2312
    char index[3];  // Chinese character index, with a space of 3 bytes
#endif
    uint8_t data[32];  // Character font data
} ChineseCell_t;

/* Font Library Declarations -------------------------------------------------*/

extern const uint8_t OLED_F8x16[][16];      // ASCII font library (8x16 font size)
extern const uint8_t OLED_F6x8[][6];        // ASCII font library (6x8 font size)

extern const ChineseCell_t OLED_CF16x16[];  // Chinese font library (16x16 font size)

/* Image Library Declarations ------------------------------------------------*/

extern const uint8_t Diode[];

/* Following the format above, add the new image declaration at here */
// ...

#ifdef __cplusplus
}
#endif
#endif /* __OLED_DATA_H__ */
