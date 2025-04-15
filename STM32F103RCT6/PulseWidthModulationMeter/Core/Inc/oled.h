/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __OLED_H__
#define __OLED_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include "main.h"

/* Function Prototypes -------------------------------------------------------*/

void OLED_Init(void);
void OLED_Clear(void);
void OLED_ShowChar(uint8_t line, uint8_t column, char character);
void OLED_ShowString(uint8_t line, uint8_t column, char *str);
void OLED_ShowNum(uint8_t line, uint8_t column, uint32_t number, uint8_t length);
void OLED_ShowSignedNum(uint8_t line, uint8_t column, int32_t number, uint8_t length);
void OLED_ShowHexNum(uint8_t line, uint8_t column, uint32_t number, uint8_t length);
void OLED_ShowBinNum(uint8_t line, uint8_t column, uint32_t number, uint8_t length);

#ifdef __cplusplus
}
#endif
#endif /* __OLED_H__ */
