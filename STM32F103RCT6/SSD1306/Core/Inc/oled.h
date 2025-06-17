/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __OLED_H__
#define __OLED_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include "main.h"
#include "oled_data.h"

/* Macros --------------------------------------------------------------------*/

#define OLED_8X16				  8
#define OLED_6X8				  6

#define OLED_UNFILLED			0
#define OLED_FILLED				1

/* Function Prototypes -------------------------------------------------------*/

/* OLED Screen Tool Functions ------------------------------------------------*/

uint32_t OLED_Pow(uint32_t x, uint32_t y);
uint8_t OLED_Pnpoly(uint8_t nvert, int16_t *vertx, int16_t *verty, int16_t testx, int16_t testy);
uint8_t OLED_IsInAngle(int16_t x, int16_t y, int16_t start_angle, int16_t end_angle);

/* OLED Screen Hardware Configuration Functions ------------------------------*/

void OLED_Init(void);
void OLED_SetCursor(uint8_t page, uint8_t x);

/* OLED Screen Display Functions ----------------------------------------------*/

void OLED_Update(void);
void OLED_UpdateArea(int16_t x, int16_t y, uint8_t width, uint8_t height);
void OLED_Clear(void);
void OLED_ClearArea(int16_t x, int16_t y, uint8_t width, uint8_t height);
void OLED_Reverse(void);
void OLED_ReverseArea(int16_t x, int16_t y, uint8_t width, uint8_t height);
void OLED_ShowChar(int16_t x, int16_t y, char character, uint8_t font_size);
void OLED_ShowString(int16_t x, int16_t y, char *str, uint8_t font_size);
void OLED_ShowNum(int16_t x, int16_t y, uint32_t number, uint8_t length, uint8_t font_size);
void OLED_ShowSignedNum(int16_t x, int16_t y, int32_t number, uint8_t length, uint8_t font_size);
void OLED_ShowHexNum(int16_t x, int16_t y, uint32_t number, uint8_t length, uint8_t font_size);
void OLED_ShowBinNum(int16_t x, int16_t y, uint32_t number, uint8_t length, uint8_t font_size);
void OLED_ShowFloatNum(int16_t x, int16_t y, double number, uint8_t int_length, uint8_t fra_length, uint8_t font_size);
void OLED_ShowImage(int16_t x, int16_t y, uint8_t width, uint8_t height, const uint8_t *image);
void OLED_Printf(int16_t x, int16_t y, uint8_t font_size, char *format, ...);

/* OLED Screen Draw Geometry Functions ---------------------------------------*/

void OLED_DrawPoint(int16_t x, int16_t y);
uint8_t OLED_GetPoint(int16_t x, int16_t y);
void OLED_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
void OLED_DrawRectangle(int16_t x, int16_t y, uint8_t width, uint8_t height, uint8_t is_filled);
void OLED_DrawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t is_filled);
void OLED_DrawCircle(int16_t center_x, int16_t center_y, uint8_t radius, uint8_t is_filled);
void OLED_DrawEllipse(int16_t center_x, int16_t center_y, uint8_t a, uint8_t b, uint8_t is_filled);
void OLED_DrawArc(int16_t center_x, int16_t center_y, uint8_t radius, int16_t start_angle, int16_t end_angle, uint8_t is_filled);

#ifdef __cplusplus
}
#endif
#endif /* __OLED_H__ */
