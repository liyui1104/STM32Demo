/* Includes ------------------------------------------------------------------*/

#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "oled_data.h"
#include "oled.h"

/* Macros --------------------------------------------------------------------*/

#define SCL_Pin GPIO_PIN_8  // SCL --> PB8
#define SDA_Pin GPIO_PIN_9  // SDA --> PB9

#define OLED_W_SCL(x) HAL_GPIO_WritePin(GPIOB, SCL_Pin, (GPIO_PinState)(x))
#define OLED_W_SDA(x) HAL_GPIO_WritePin(GPIOB, SDA_Pin, (GPIO_PinState)(x))

/* Global Variables ----------------------------------------------------------*/

/**
 * @brief  OLED display memory array
 * 
 * @note All display functions only read from or write to this display memory array.
 * 			 Subsequently, calling the OLED_Update function or the OLED_UpdateArea function
 * 			 will send the data in the display memory array to the OLED hardware for display.
 */
uint8_t OLED_DisplayBuf[8][128];

/* Software-emulated I2C Communication Functions -----------------------------*/

/**
 * @brief  I2C pins initialization
 * @param  None
 * @retval None
 */
void OLED_I2C_Init(void)
{
  OLED_W_SCL(GPIO_PIN_SET);
  OLED_W_SDA(GPIO_PIN_SET);
}

/**
 * @brief  I2C start condition
 * @param  None
 * @retval None
 */
void OLED_I2C_Start(void)
{
  OLED_W_SDA(GPIO_PIN_SET);
  OLED_W_SCL(GPIO_PIN_SET);
  OLED_W_SDA(GPIO_PIN_RESET);
  OLED_W_SCL(GPIO_PIN_RESET);
}

/**
 * @brief  I2C stop condition
 * @param  None
 * @retval None
 */
void OLED_I2C_Stop(void)
{
  OLED_W_SDA(GPIO_PIN_RESET);
  OLED_W_SCL(GPIO_PIN_SET);
  OLED_W_SDA(GPIO_PIN_SET);
}

/**
 * @brief  I2C send a byte of data
 * @param  byte The byte to send
 * @retval None
 */
void OLED_I2C_SendByte(uint8_t byte)
{
  uint8_t i;
  for (i = 0; i < 8; i++)
  {
    OLED_W_SDA(byte & (0x80 >> i));
    OLED_W_SCL(GPIO_PIN_SET);
    OLED_W_SCL(GPIO_PIN_RESET);
  }
  OLED_W_SCL(GPIO_PIN_SET);  // Extra clock for acknowledgment, not used here
  OLED_W_SCL(GPIO_PIN_RESET);
}

/**
 * @brief  Write a command to the OLED
 * @param  command The command to write
 * @retval None
 */
void OLED_WriteCommand(uint8_t command)
{
  OLED_I2C_Start();
  OLED_I2C_SendByte(0x78);  // Slave address
  OLED_I2C_SendByte(0x00);  // Command mode
  OLED_I2C_SendByte(command);
  OLED_I2C_Stop();
}

/**
 * @brief  Write data to the OLED
 * @param  data The start address of the data to write
 * @param  count The number of bytes to write
 * @retval None
 */
void OLED_WriteData(uint8_t *data, uint8_t count)
{
  uint8_t i;

  OLED_I2C_Start();
  OLED_I2C_SendByte(0x78);  // Slave address
  OLED_I2C_SendByte(0x40);  // Data mode
  for (i = 0; i < count; i++)
  {
    OLED_I2C_SendByte(data[i]);
  }
  OLED_I2C_Stop();
}

/* OLED Screen Tool Functions ------------------------------------------------*/

/**
 * @brief  Power function
 * @param  x The base number
 * @param  y The exponent
 * @retval The result of x raised to the power of y
 */
uint32_t OLED_Pow(uint32_t x, uint32_t y)
{
	uint32_t result = 1;  // Default result is 1
	while (y--)  // Multiply x by itself y times
	{
		result *= x;  // Multiply x to the result each time
	}
	return result;
}

/**
 * @brief  Determine if a specified point is inside a specified polygon
 * @param  nvert The number of vertices of the polygon
 * @param  vertx An array containing the x-coordinates of the polygon's vertices
 * @param  verty An array containing the y-coordinates of the polygon's vertices
 * @param  testx The x-coordinate of the test point
 * @param  testy The y-coordinate of the test point
 * @retval Whether the specified point is inside the specified polygon, 1: inside, 0: outside
 */
uint8_t OLED_Pnpoly(uint8_t nvert, int16_t *vertx, int16_t *verty, int16_t testx, int16_t testy)
{
	int16_t i, j, c = 0;
	
	/* The algorithm was proposed by W. Randolph Franklin (https://wrfranklin.org/) */
	for (i = 0, j = nvert - 1; i < nvert; j = i++)
	{
		if (((verty[i] > testy) != (verty[j] > testy)) &&
			  (testx < (vertx[j] - vertx[i]) * (testy - verty[i]) / (verty[j] - verty[i]) + vertx[i]))
		{
			c = !c;
		}
	}
	return c;
}

/**
 * @brief  Determine if a specified point is within a specified angle range
 * @param  x The x-coordinate of the specified point
 * @param  y The y-coordinate of the specified point
 * @param  start_angle The starting angle, range: [-180,180]
 * @param  end_angle The ending angle, range: [-180,180]
 * @retval Whether the specified point is within the specified angle range, 1: inside, 0: outside
 * @note 0 degrees is to the right horizontally, 180 or -180 degrees is to the left horizontally. 
 *       Positive angles are below the horizontal line, negative angles are above, and the rotation is clockwise.
 */
uint8_t OLED_IsInAngle(int16_t x, int16_t y, int16_t start_angle, int16_t end_angle)
{
	int16_t point_angle;

	// Calculate the angle of the specified point in radians and convert it to degrees
	point_angle = atan2(y, x) / 3.14 * 180;
  
	if (start_angle < end_angle)	// The starting angle is less than the ending angle
	{
		// If the point's angle is between the start and end angles, the point is within the angle range
		if (point_angle >= start_angle && point_angle <= end_angle)
		{
			return 1;
		}
	}
	else  // The starting angle is greater than the ending angle
	{
		// If the point's angle is greater than the start angle or less than the end angle, the point is within the angle range
		if (point_angle >= start_angle || point_angle <= end_angle)
		{
			return 1;
		}
	}
	return 0;		// If the above conditions are not met, the point is outside the angle range
}

/* OLED Screen Hardware Configuration Functions ------------------------------*/

/**
 * @brief  Initialize the OLED screen
 * @param  None
 * @retval None
 */
void OLED_Init(void)
{
  HAL_Delay(100);  // Power-up delay

  OLED_I2C_Init();  // Initialize I2C pins

  OLED_WriteCommand(0xAE);  // Turn off display

  OLED_WriteCommand(0xD5);  // Set display clock divide ratio/oscillator frequency
  OLED_WriteCommand(0x80);

  OLED_WriteCommand(0xA8);  // Set multiplex ratio
  OLED_WriteCommand(0x3F);

  OLED_WriteCommand(0xD3);  // Set display offset
  OLED_WriteCommand(0x00);

  OLED_WriteCommand(0x40);  // Set display start line

  OLED_WriteCommand(0xA1);  // Set segment re-map (normal)

  OLED_WriteCommand(0xC8);  // Set COM output scan direction (normal)

  OLED_WriteCommand(0xDA);  // Set COM pins hardware configuration
  OLED_WriteCommand(0x12);

  OLED_WriteCommand(0x81);  // Set contrast control
  OLED_WriteCommand(0xCF);

  OLED_WriteCommand(0xD9);  // Set pre-charge period
  OLED_WriteCommand(0xF1);

  OLED_WriteCommand(0xDB);  // Set VCOMH deselect level
  OLED_WriteCommand(0x30);

  OLED_WriteCommand(0xA4);  // Entire display on/off (resume to RAM content)

  OLED_WriteCommand(0xA6);  // Set normal display

  OLED_WriteCommand(0x8D);  // Enable charge pump
  OLED_WriteCommand(0x14);

  OLED_WriteCommand(0xAF);  // Turn on OLED panel

  OLED_Clear();  // Clear the OLED screen
  OLED_Update();
}

/**
 * @brief  Set the display cursor position on the OLED
 * @param  page The page where the cursor is located, range: [0,7]
 * @param  x The x-axis coordinate where the cursor is located, range: [0,127]
 * @retval None
 * @note   The default y-axis of the OLED screen can only be written in groups of 8 bits.
 *         1 page equals 8 y-axis coordinates.
 */
void OLED_SetCursor(uint8_t page, uint8_t x)
{
	OLED_WriteCommand(0xB0 | page);					      // Set the page position
	OLED_WriteCommand(0x10 | ((x & 0xF0) >> 4));  // Set the high 4 bits of the x position
	OLED_WriteCommand(0x00 | (x & 0x0F));			    // Set the low 4 bits of the x position
}

/* OLED Screen Display Functions ---------------------------------------------*/

/**
 * @brief  Update the OLED screen with the display memory array
 * @param  None
 * @retval None
 * @note   All display functions only read from or write to the OLED display memory array.
 *         Subsequently, calling the OLED_Update function or the OLED_UpdateArea function
 *         will send the data in the display memory array to the OLED hardware for display.
 *         Therefore, after calling a display function, must call an update function to actually display the content on the screen.
 */
void OLED_Update(void)
{
	uint8_t j;

	/* Iterate through each page */
	for (j = 0; j < 8; j++)
	{
		// Set the cursor position to the first column of each page
		OLED_SetCursor(j, 0);

		// Transfer the display memory array data to the OLED hardware by continuously writing 128 data bytes
		OLED_WriteData(OLED_DisplayBuf[j], 128);
	}
}

/**
 * @brief  Partially update the OLED screen with the display memory array
 * @param  x The x-coordinate of the top-left corner of the specified area, range: [-32768,32767], screen area: [0,127]
 * @param  y The y-coordinate of the top-left corner of the specified area, range: [-32768,32767], screen area: [0,63]
 * @param  width The width of the specified area, range: [0,128]
 * @param  height The height of the specified area, range: [0,64]
 * @retval None
 * @note   This function will update at least the area specified by the parameters.
 *         If the y-axis of the update area only includes part of a page, the remaining part of the same page will be updated as well.
 */
void OLED_UpdateArea(int16_t x, int16_t y, uint8_t width, uint8_t height)
{
	int16_t j;
	int16_t page, page1;
	
	/* A negative coordinate needs an offset when calculating the page address */
	page = y / 8;
	page1 = (y + height - 1) / 8 + 1;  // It's equal to (y + height) / 8 rounded up
	if (y < 0)
	{
		page -= 1;
		page1 -= 1;
	}
	
	/* Iterate through the pages involved in the specified area */
	for (j = page; j < page1; j++)
	{
		if (x >= 0 && x <= 127 && j >= 0 && j <= 7)		// Content outside the screen will not be displayed
		{
			// Set the cursor position to the specified column of the relevant page
			OLED_SetCursor(j, x);

			// Transfer the display memory array data to the OLED hardware by continuously writing data bytes
			OLED_WriteData(&OLED_DisplayBuf[j][x], width);
		}
	}
}

/**
 * @brief  Clear the entire OLED display memory array
 * @param  None
 * @retval None
 */
void OLED_Clear(void)
{
	uint8_t i, j;

	/* Iterate through 8 pages */
	for (j = 0; j < 8; j++)				
	{
		/* Iterate through 128 columns */
		for (i = 0; i < 128; i++)			
		{
			// Clear all data in the display memory array
			OLED_DisplayBuf[j][i] = 0x00;	
		}
	}
}

/**
 * @brief  Partially clear the OLED display memory array
 * @param  x The x-coordinate of the top-left corner of the specified area, range: [-32768,32767], screen area: [0,127]
 * @param  y The y-coordinate of the top-left corner of the specified area, range: [-32768,32767], screen area: [0,63]
 * @param  width The width of the specified area, range: [0,128]
 * @param  height The height of the specified area, range: [0,64]
 * @retval None
 */
void OLED_ClearArea(int16_t x, int16_t y, uint8_t width, uint8_t height)
{
	int16_t i, j;
	
	/* Iterate through the specified pages */
	for (j = y; j < y + height; j++)		
	{
		/* Iterate through the specified columns */
		for (i = x; i < x + width; i++)	
		{
			// Content outside the screen will not be displayed
			if (i >= 0 && i <= 127 && j >=0 && j <= 63)				
			{
				// Clear the specified data in the display memory array
				OLED_DisplayBuf[j / 8][i] &= ~(0x01 << (j % 8));	
			}
		}
	}
}

/**
 * @brief  Invert all data in the OLED display memory array
 * @param  None
 * @retval None
 */
void OLED_Reverse(void)
{
	uint8_t i, j;

	/* Iterate through 8 pages */
	for (j = 0; j < 8; j++)				
	{
		/* Iterate through 128 columns */
		for (i = 0; i < 128; i++)			
		{
			// Invert all data in the display memory array
			OLED_DisplayBuf[j][i] ^= 0xFF;	
		}
	}
}

/**
 * @brief  Partially invert the data in the OLED display memory array
 * @param  x The x-coordinate of the top-left corner of the specified area, range: [-32768,32767], screen area: [0,127]
 * @param  y The y-coordinate of the top-left corner of the specified area, range: [-32768,32767], screen area: [0,63]
 * @param  width The width of the specified area, range: [0,128]
 * @param  height The height of the specified area, range: [0,64]
 * @retval None
 */
void OLED_ReverseArea(int16_t x, int16_t y, uint8_t width, uint8_t height)
{
	int16_t i, j;
	
	/* Iterate through the specified pages */
	for (j = y; j < y + height; j++)		
	{
		/* Iterate through the specified columns */
		for (i = x; i < x + width; i++)	
		{
			// Content outside the screen will not be displayed
			if (i >= 0 && i <= 127 && j >=0 && j <= 63)				
			{
				// Invert the specified data in the display memory array
				OLED_DisplayBuf[j / 8][i] ^= 0x01 << (j % 8);	
			}
		}
	}
}

/**
 * @brief  Display a character on the OLED
 * @param  x The x-coordinate of the top-left corner of the character, range: [-32768,32767], screen area: [0,127]
 * @param  y The y-coordinate of the top-left corner of the character, range: [-32768,32767], screen area: [0,63]
 * @param  character The character to display, range: visible ASCII characters
 * @param  font_size The font size, range: OLED_8X16 (8 pixels wide, 16 pixels high) or OLED_6X8 (6 pixels wide, 8 pixels high)
 * @retval None
 */
void OLED_ShowChar(int16_t x, int16_t y, char character, uint8_t font_size)
{
	if (font_size == OLED_8X16)		// Font size is 8 pixels wide and 16 pixels high
	{
		// Display the specified data from the ASCII font library OLED_F8x16 in an 8*16 image format
		OLED_ShowImage(x, y, 8, 16, OLED_F8x16[character - ' ']);
	}
	else if(font_size == OLED_6X8)	// Font size is 6 pixels wide and 8 pixels high
	{
		// Display the specified data from the ASCII font library OLED_F6x8 in a 6*8 image format
		OLED_ShowImage(x, y, 6, 8, OLED_F6x8[character - ' ']);
	}
}

/**
 * @brief  Display a string on the OLED
 * @param  x The x-coordinate of the top-left corner of the string, range: [-32768,32767], screen area: [0,127]
 * @param  y The y-coordinate of the top-left corner of the string, range: [-32768,32767], screen area: [0,63]
 * @param  str The string to display, consisting of visible ASCII characters or Chinese characters
 * @param  font_size The font size, range: OLED_8X16 (8 pixels wide, 16 pixels high) or OLED_6X8 (6 pixels wide, 8 pixels high)
 * @retval None
 * @note   This function supports mixed input of ASCII and Chinese characters.
 *         Chinese characters to be displayed need to be defined in the OLED_CF16x16 array in oled_data.c file.
 *         If a specified Chinese character is not found, a default graphic (a box with a question mark inside) will be displayed.
 *         When the font size is OLED_8X16, Chinese characters are displayed normally in a 16*16 dot matrix.
 *         When the font size is OLED_6X8, Chinese characters are displayed as '?' in a 6*8 dot matrix.
 */
void OLED_ShowString(int16_t x, int16_t y, char *str, uint8_t font_size)
{
	uint16_t i = 0;
	char single_char[5];
	uint8_t char_length = 0;
	uint16_t x_offset = 0;
	uint16_t p_index;
	
	/* Iterate through the string */
	while (str[i] != '\0')	
	{
#ifdef OLED_CHARSET_UTF8  // Define the character set as UTF-8
		/* The purpose of this code is to extract a character from the UTF-8 string and store it in the single_char substring */
		/* Check the flag bit of the first byte of the UTF-8 encoding */
		if ((str[i] & 0x80) == 0x00)  // The first byte is 0xxxxxxx
		{
			char_length = 1;						// The character is 1 byte long
			single_char[0] = str[i++];  // Write the first byte to the 0th position of single_char, then move i to the next byte
			single_char[1] = '\0';			// Add a string termination flag to single_char
		}
		else if ((str[i] & 0xE0) == 0xC0)  // The first byte is 110xxxxx
		{
			char_length = 2;						     // The character is 2 bytes long
			single_char[0] = str[i++];		   // Write the first byte to the 0th position of single_char, then move i to the next byte
			if (str[i] == '\0') {break;}		 // Unexpected situation, break the loop and end the display
			single_char[1] = str[i++];		   // Write the second byte to the 1st position of single_char, then move i to the next byte
			single_char[2] = '\0';				   // Add a string termination flag to single_char
		}
		else if ((str[i] & 0xF0) == 0xE0)	 // The first byte is 1110xxxx
		{
			char_length = 3;						     // The character is 3 bytes long
			single_char[0] = str[i++];
			if (str[i] == '\0') {break;}
			single_char[1] = str[i++];
			if (str[i] == '\0') {break;}
			single_char[2] = str[i++];
			single_char[3] = '\0';
		}
		else if ((str[i] & 0xF8) == 0xF0)	// The first byte is 11110xxx
		{
			char_length = 4;						    // The character is 4 bytes long
			single_char[0] = str[i++];
			if (str[i] == '\0') {break;}
			single_char[1] = str[i++];
			if (str[i] == '\0') {break;}
			single_char[2] = str[i++];
			if (str[i] == '\0') {break;}
			single_char[3] = str[i++];
			single_char[4] = '\0';
		}
		else
		{
			i++;  // Unexpected situation, move i to the next byte, ignore this byte and continue to check the next byte
			continue;
		}
#endif
#ifdef OLED_CHARSET_GB2312  // Define the character set as GB2312
		/* The purpose of this code is to extract a character from the GB2312 string and store it in the single_char substring */
		/* Check the highest bit flag of the GB2312 byte */
		if ((str[i] & 0x80) == 0x00)  // The highest bit is 0
		{
			char_length = 1;						// The character is 1 byte long
			single_char[0] = str[i++];	// Write the first byte to the 0th position of single_char, then move i to the next byte
			single_char[1] = '\0';			// Add a string termination flag to single_char
		}
		else  // The highest bit is 1
		{
			char_length = 2;						  // The character is 2 bytes long
			single_char[0] = str[i++];		// Write the first byte to the 0th position of single_char, then move i to the next byte
			if (str[i] == '\0') {break;}  // Unexpected situation, break the loop and end the display
			single_char[1] = str[i++];		// Write the second byte to the 1st position of single_char, then move i to the next byte
			single_char[2] = '\0';				// Add a string termination flag to single_char
		}
#endif
		/* Display the single_char extracted by the above code */
		if (char_length == 1)  // If it is a single-byte character
		{
			// Use OLED_ShowChar to display this character
			OLED_ShowChar(x + x_offset, y, single_char[0], font_size);
			x_offset += font_size;
		}
		else // Otherwise, it is a multi-byte character
		{
			// Iterate through the entire font library to find the data of this character
			// If the last character (defined as an empty string) is found, it means the character is not defined in the font library, stop searching
			for (p_index = 0; strcmp(OLED_CF16x16[p_index].index, "") != 0; p_index++)
			{
				// Find a matching character
				if (strcmp(OLED_CF16x16[p_index].index, single_char) == 0)
				{
					break;  // Break the loop, at this time the value of p_index is the index of the specified character
				}
			}
			if (font_size == OLED_8X16)		// The given font is 8*16 dots
			{
				// Display the specified data from the font library OLED_CF16x16 in a 16*16 image format
				OLED_ShowImage(x + x_offset, y, 16, 16, OLED_CF16x16[p_index].data);
				x_offset += 16;
			}
			else if (font_size == OLED_6X8)	 // The given font is 6*8 dots
			{
				// Insufficient space, display '?' at this position
				OLED_ShowChar(x + x_offset, y, '?', OLED_6X8);
				x_offset += OLED_6X8;
			}
		}
	}
}

/**
 * @brief  Display a positive decimal integer on the OLED
 * @param  x The x-coordinate of the top-left corner of the number, range: [-32768,32767], screen area: [0,127]
 * @param  y The y-coordinate of the top-left corner of the number, range: [-32768,32767], screen area: [0,63]
 * @param  number The number to display, range: [0,4294967295]
 * @param  length The length of the number, range: [0,10]
 * @param  font_size The font size, range: OLED_8X16 (8 pixels wide, 16 pixels high) or OLED_6X8 (6 pixels wide, 8 pixels high)
 * @retval None
 */
void OLED_ShowNum(int16_t x, int16_t y, uint32_t number, uint8_t length, uint8_t font_size)
{
	uint8_t i;
	
  /* Iterate through each digit of the number */
	for (i = 0; i < length; i++)		
	{
		// number / OLED_Pow(10, length - i - 1) % 10 extracts each decimal digit of the number
		// Adding '0' converts the number to a character format
		OLED_ShowChar(x + i * font_size, y, number / OLED_Pow(10, length - i - 1) % 10 + '0', font_size);
	}
}

/**
 * @brief  Display a signed decimal integer on the OLED
 * @param  x The x-coordinate of the top-left corner of the number, range: [-32768,32767], screen area: [0,127]
 * @param  y The y-coordinate of the top-left corner of the number, range: [-32768,32767], screen area: [0,63]
 * @param  number The number to display, range: [-2147483648,2147483647]
 * @param  length The length of the number, range: [0,10]
 * @param  font_size The font size, range: OLED_8X16 (8 pixels wide, 16 pixels high) or OLED_6X8 (6 pixels wide, 8 pixels high)
 * @retval None
 */
void OLED_ShowSignedNum(int16_t x, int16_t y, int32_t number, uint8_t length, uint8_t font_size)
{
	uint8_t i;
	uint32_t number1;
	
	if (number >= 0)  // If the number is greater than or equal to 0
	{
		OLED_ShowChar(x, y, '+', font_size);	 // Display the '+' sign
		number1 = number;  // number1 is directly equal to number
	}
	else  // If the number is less than 0
	{
		OLED_ShowChar(x, y, '-', font_size);	 // Display the '-' sign
		number1 = -number;  // number1 is the absolute value of number
	}
	
	/* Iterate through each digit of the number */
	for (i = 0; i < length; i++)			
	{
		OLED_ShowChar(x + (i + 1) * font_size, y, number1 / OLED_Pow(10, length - i - 1) % 10 + '0', font_size);
	}
}

/**
 * @brief  Display a positive hexadecimal integer on the OLED
 * @param  x The x-coordinate of the top-left corner of the number, range: [-32768,32767], screen area: [0,127]
 * @param  y The y-coordinate of the top-left corner of the number, range: [-32768,32767], screen area: [0,63]
 * @param  number The number to display, range: [0x00000000,0xFFFFFFFF]
 * @param  length The length of the number, range: [0,8]
 * @param  font_size The font size, range: OLED_8X16 (8 pixels wide, 16 pixels high) or OLED_6X8 (6 pixels wide, 8 pixels high)
 * @retval None
 */
void OLED_ShowHexNum(int16_t x, int16_t y, uint32_t number, uint8_t length, uint8_t font_size)
{
	uint8_t i, single_number;

	/* Iterate through each digit of the number */
	for (i = 0; i < length; i++)		
	{
		// Extract each hexadecimal digit of the number
		single_number = number / OLED_Pow(16, length - i - 1) % 16;
		
		if (single_number < 10)  // If the single digit is less than 10
		{
			// Call the OLED_ShowChar function to display this digit
			OLED_ShowChar(x + i * font_size, y, single_number + '0', font_size);
		}
		else  // If the single digit is greater than or equal to 10
		{
			// Adding 'A' converts the number to a hexadecimal character starting from 'A'
			OLED_ShowChar(x + i * font_size, y, single_number - 10 + 'A', font_size);
		}
	}
}

/**
 * @brief  Display a positive binary integer on the OLED
 * @param  x The x-coordinate of the top-left corner of the number, range: [-32768,32767], screen area: [0,127]
 * @param  y The y-coordinate of the top-left corner of the number, range: [-32768,32767], screen area: [0,63]
 * @param  number The number to display, range: [0x00000000,0xFFFFFFFF]
 * @param  length The length of the number, range: [0,16]
 * @param  font_size The font size, range: OLED_8X16 (8 pixels wide, 16 pixels high) or OLED_6X8 (6 pixels wide, 8 pixels high)
 * @retval None
 */
void OLED_ShowBinNum(int16_t x, int16_t y, uint32_t number, uint8_t length, uint8_t font_size)
{
	uint8_t i;
	
  /* Iterate through each digit of the number */
	for (i = 0; i < length; i++)		
	{
		// number / OLED_Pow(2, length - i - 1) % 2 extracts each binary digit of the number
		// Adding '0' converts the number to a character format
		OLED_ShowChar(x + i * font_size, y, number / OLED_Pow(2, length - i - 1) % 2 + '0', font_size);
	}
}

/**
 * @brief  Display a floating-point number on the OLED
 * @param  x The x-coordinate of the top-left corner of the number, range: [-32768,32767], screen area: [0,127]
 * @param  y The y-coordinate of the top-left corner of the number, range: [-32768,32767], screen area: [0,63]
 * @param  number The number to display, range: [-4294967295.0,4294967295.0]
 * @param  int_length The length of the integer part of the number, range: [0,10]
 * @param  fra_length The length of the fractional part of the number, range: [0,9]
 * @param  font_size The font size, range: OLED_8X16 (8 pixels wide, 16 pixels high) or OLED_6X8 (6 pixels wide, 8 pixels high)
 * @retval None
 * @note   The fraction is rounded for display.
 */
void OLED_ShowFloatNum(int16_t x, int16_t y, double number, uint8_t int_length, uint8_t fra_length, uint8_t font_size)
{
	uint32_t pow_num, int_num, fra_num;
	
	if (number >= 0)  // If the number is greater than or equal to 0
	{
		OLED_ShowChar(x, y, '+', font_size);  // Display the '+' sign
	}
	else  // If the number is less than 0
	{
		OLED_ShowChar(x, y, '-', font_size);  // Display the '-' sign
		number = -number;  // Take the absolute value of number
	}
	
	/* Extract the integer and fractional parts */
	int_num = number;                    // Directly assign to an integer variable to extract the integer part
	number -= int_num;                   // Subtract the integer part from number to prevent errors when multiplying the fraction into an integer
	pow_num = OLED_Pow(10, fra_length);  // Determine the multiplier based on the specified number of decimal places
	fra_num = round(number * pow_num);   // Multiply the fraction into an integer and round it to avoid display errors
	int_num += fra_num / pow_num;				 // If rounding causes a carry, add it to the integer part
	
	// Display the integer part
	OLED_ShowNum(x + font_size, y, int_num, int_length, font_size);
	
	// Display the decimal point
	OLED_ShowChar(x + (int_length + 1) * font_size, y, '.', font_size);
	
	// Display the fractional part
	OLED_ShowNum(x + (int_length + 2) * font_size, y, fra_num, fra_length, font_size);
}

/**
 * @brief  Display an image on the OLED
 * @param  x The x-coordinate of the top-left corner of the image, range: [-32768,32767], screen area: [0,127]
 * @param  y The y-coordinate of the top-left corner of the image, range: [-32768,32767], screen area: [0,63]
 * @param  width The width of the image, range: [0,128]
 * @param  height The height of the image, range: [0,64]
 * @param  image The image to display
 * @retval None
 */
void OLED_ShowImage(int16_t x, int16_t y, uint8_t width, uint8_t height, const uint8_t *image)
{
	uint8_t i = 0, j = 0;
	int16_t page, shift;

	/* A negative coordinate needs an offset when calculating the page address and shift */
	page = y / 8;
	shift = y % 8;
	if (y < 0)
	{
		page -= 1;
		shift += 8;
	}

	// Clear the area where the image will be displayed
	OLED_ClearArea(x, y, width, height);
	
	/* Iterate through the relevant pages involved in the specified image */
	for (j = 0; j < (height - 1) / 8 + 1; j++)  // (height - 1) / 8 + 1 is used to round up height / 8
	{
		/* Iterate through the relevant columns involved in the specified image */
		for (i = 0; i < width; i++)
		{
			if (x + i >= 0 && x + i <= 127)  // Content outside the screen will not be displayed
			{
				if (page + j >= 0 && page + j <= 7)  // Content outside the screen will not be displayed
				{
					// Display the content of the image on the current page
					OLED_DisplayBuf[page + j][x + i] |= image[j * width + i] << (shift);
				}
				
				if (page + j + 1 >= 0 && page + j + 1 <= 7)  // Content outside the screen will not be displayed
				{					
					// Display the content of the image on the next page
					OLED_DisplayBuf[page + j + 1][x + i] |= image[j * width + i] >> (8 - shift);
				}
			}
		}
	}
}

/**
 * @brief  Use the printf function to print a formatted string on the OLED, supporting mixed input of ASCII and Chinese characters
 * @param  x The x-coordinate of the top-left corner of the formatted string, range: [-32768,32767], screen area: [0,127]
 * @param  y The y-coordinate of the top-left corner of the formatted string, range: [-32768,32767], screen area: [0,63]
 * @param  font_size The font size, range: OLED_8X16 (8 pixels wide, 16 pixels high) or OLED_6X8 (6 pixels wide, 8 pixels high)
 * @param  format The formatted string to display, consisting of visible ASCII characters or Chinese characters
 * @param  ... Variable argument list for the formatted string
 * @retval None
 * @note   Chinese characters to be displayed need to be defined in the OLED_CF16x16 array in OLED_Data.c file.
 *         If a specified Chinese character is not found, a default graphic (a box with a question mark inside) will be displayed.
 *         When the font size is OLED_8X16, Chinese characters are displayed normally in a 16*16 dot matrix.
 *         When the font size is OLED_6X8, Chinese characters are displayed as '?' in a 6*8 dot matrix.
 */
void OLED_Printf(int16_t x, int16_t y, uint8_t font_size, char *format, ...)
{
	char str[256];						   // Define a character array
	va_list arg;							   // Define a variable of variable argument list data type
	va_start(arg, format);			 // Start receiving the argument list from format into the arg variable
	vsprintf(str, format, arg);  // Use vsprintf to print the formatted string and argument list into the character array
	va_end(arg);							   // End the variable arg
	OLED_ShowString(x, y, str, font_size);	 // Display the character array (string) on the OLED
}

/* OLED Screen Draw Geometry Functions ---------------------------------------*/

/**
 * @brief  Draw a point on the OLED at the specified position
 * @param  x The x-coordinate of the point, range: [-32768,32767], screen area: [0,127]
 * @param  y The y-coordinate of the point, range: [-32768,32767], screen area: [0,63]
 * @retval None
 */
void OLED_DrawPoint(int16_t x, int16_t y)
{
	if (x >= 0 && x <= 127 && y >=0 && y <= 63)		// Content outside the screen will not be displayed
	{
		// Set the bit data at the specified position in the display buffer array to 1
		OLED_DisplayBuf[y / 8][x] |= 0x01 << (y % 8);
	}
}

/**
 * @brief  Get the value of a point at the specified position on the OLED
 * @param  x The x-coordinate of the point, range: [-32768,32767], screen area: [0,127]
 * @param  y The y-coordinate of the point, range: [-32768,32767], screen area: [0,63]
 * @retval Whether the point at the specified position is lit, 1: lit, 0: off
 */
uint8_t OLED_GetPoint(int16_t x, int16_t y)
{
	if (x >= 0 && x <= 127 && y >=0 && y <= 63)		// Content outside the screen will not be read
	{
		// Check the data at the specified position
		if (OLED_DisplayBuf[y / 8][x] & 0x01 << (y % 8))
		{
			return 1;	 // If it's 1, return 1
		}
	}
	
	return 0;  // Otherwise, return 0
}

/**
 * @brief  Draw a line on the OLED
 * @param  x0 The x-coordinate of one endpoint, range: [-32768,32767], screen area: [0,127]
 * @param  y0 The y-coordinate of one endpoint, range: [-32768,32767], screen area: [0,63]
 * @param  x1 The x-coordinate of the other endpoint, range: [-32768,32767], screen area: [0,127]
 * @param  y1 The y-coordinate of the other endpoint, range: [-32768,32767], screen area: [0,63]
 * @retval None
 */
void OLED_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
	int16_t x, y, dx, dy, d, incrE, incrNE, temp;
	int16_t x0_ = x0, y0_ = y0, x1_ = x1, y1_ = y1;
	uint8_t yflag = 0, xyflag = 0;
	
	if (y0_ == y1_)  // Handle horizontal lines separately
	{
		// If the x-coordinate of point 0 is greater than that of point 1, swap their x-coordinates
		if (x0_ > x1_) {temp = x0_; x0_ = x1_; x1_ = temp;}
		
		/* Iterate through the x-coordinates */
		for (x = x0_; x <= x1_; x++)
		{
			OLED_DrawPoint(x, y0_);	 // Draw points sequentially
		}
	}
	else if (x0_ == x1_)	 // Handle vertical lines separately
	{
		// If the y-coordinate of point 0 is greater than that of point 1, swap their y-coordinates
		if (y0_ > y1_) {temp = y0_; y0_ = y1_; y1_ = temp;}
		
		/* Iterate through the y-coordinates */
		for (y = y0_; y <= y1_; y++)
		{
			OLED_DrawPoint(x0_, y);	 // Draw points sequentially
		}
	}
	else  // Handle diagonal lines
	{
		/* Use the Bresenham's algorithm to draw a line, which avoids time-consuming floating-point operations and is more efficient */
		/* Reference document: https://www.cs.montana.edu/courses/spring2009/425/dslectures/Bresenham.pdf */
		/* Reference tutorial: https://www.bilibili.com/video/BV1364y1d7Lo */

		if (x0_ > x1_)  // If the x-coordinate of point 0 is greater than that of point 1
		{
      // Swap the coordinates of the two points
			// After swapping, it does not affect the line drawing, but the drawing direction changes from the first, second, third, and fourth quadrants to the first and fourth quadrants
			temp = x0_; x0_ = x1_; x1_ = temp;
			temp = y0_; y0_ = y1_; y1_ = temp;
		}
		
		if (y0_ > y1_)  // If the y-coordinate of point 0 is greater than that of point 1
		{
			// Negate the y-coordinates
			// Negating affects the line drawing, but the drawing direction changes from the first and fourth quadrants to the first quadrant
			y0_ = -y0_;
			y1_ = -y1_;
			
			// Set the flag bit yflag to remember the current transformation, and swap the coordinates back when actually drawing the line later
			yflag = 1;
		}
		
		if (y1_ - y0_ > x1_ - x0_)	// If the slope of the line is greater than 1
		{
			// Swap the x and y coordinates
			// Swapping affects the line drawing, but the drawing direction changes from the 0~90 degrees range in the first quadrant to the 0~45 degrees range in the first quadrant
			temp = x0_; x0_ = y0_; y0_ = temp;
			temp = x1_; x1_ = y1_; y1_ = temp;
			
			// Set the flag bit xyflag to remember the current transformation, and swap the coordinates back when actually drawing the line later
			xyflag = 1;
		}
		
		// The following is the Bresenham's algorithm for drawing a line
		// The algorithm requires that the drawing direction must be in the 0~45 degrees range in the first quadrant
		dx = x1_ - x0_;
		dy = y1_ - y0_;
		incrE = 2 * dy;
		incrNE = 2 * (dy - dx);
		d = 2 * dy - dx;
		x = x0_;
		y = y0_;
		
		/* Draw the starting point and check the flag bits to swap the coordinates back */
		if (yflag && xyflag)  {OLED_DrawPoint(y, -x);}
		else if (yflag)		    {OLED_DrawPoint(x, -y);}
		else if (xyflag)	    {OLED_DrawPoint(y, x);}
		else				          {OLED_DrawPoint(x, y);}
		
		while (x < x1_)  // Iterate through each point on the x-axis
		{
			x++;
			if (d < 0)		// The next point is to the east of the current point
			{
				d += incrE;
			}
			else  // The next point is to the northeast of the current point
			{
				y++;
				d += incrNE;
			}
			
			/* Draw each point and check the flag bits to swap the coordinates back */
			if (yflag && xyflag){OLED_DrawPoint(y, -x);}
			else if (yflag)		  {OLED_DrawPoint(x, -y);}
			else if (xyflag)	  {OLED_DrawPoint(y, x);}
			else				        {OLED_DrawPoint(x, y);}
		}	
	}
}

/**
 * @brief  Draw a rectangle on the OLED
 * @param  x The x-coordinate of the top-left corner of the rectangle, range: [-32768,32767], screen area: [0,127]
 * @param  y The y-coordinate of the top-left corner of the rectangle, range: [-32768,32767], screen area: [0,63]
 * @param  width The width of the rectangle, range: [0,128]
 * @param  height The height of the rectangle, range: [0,64]
 * @param  is_filled Whether the rectangle is filled, range: OLED_UNFILLED (not filled) or OLED_FILLED (filled)
 * @retval None
 */
void OLED_DrawRectangle(int16_t x, int16_t y, uint8_t width, uint8_t height, uint8_t is_filled)
{
	int16_t i, j;
	if (!is_filled)  // If the rectangle is not filled
	{
		/* Iterate through the x-coordinates of the top and bottom edges, draw the top and bottom lines of the rectangle */
		for (i = x; i < x + width; i++)
		{
			OLED_DrawPoint(i, y);
			OLED_DrawPoint(i, y + height - 1);
		}
		/* Iterate through the y-coordinates of the left and right edges, draw the left and right lines of the rectangle */
		for (i = y; i < y + height; i++)
		{
			OLED_DrawPoint(x, i);
			OLED_DrawPoint(x + width - 1, i);
		}
	}
	else  // If the rectangle is filled
	{
		/* Iterate through the x-coordinates */
		for (i = x; i < x + width; i++)
		{
			/* Iterate through the y-coordinates */
			for (j = y; j < y + height; j++)
			{
				// Draw points in the specified area to fill the rectangle
				OLED_DrawPoint(i, j);
			}
		}
	}
}

/**
 * @brief  Draw a triangle on the OLED
 * @param  x0 The x-coordinate of the first endpoint, range: [-32768,32767], screen area: [0,127]
 * @param  y0 The y-coordinate of the first endpoint, range: [-32768,32767], screen area: [0,63]
 * @param  x1 The x-coordinate of the second endpoint, range: [-32768,32767], screen area: [0,127]
 * @param  y1 The y-coordinate of the second endpoint, range: [-32768,32767], screen area: [0,63]
 * @param  x2 The x-coordinate of the third endpoint, range: [-32768,32767], screen area: [0,127]
 * @param  y2 The y-coordinate of the third endpoint, range: [-32768,32767], screen area: [0,63]
 * @param  is_filled Whether the rectangle is filled, range: OLED_UNFILLED (not filled) or OLED_FILLED (filled)
 * @retval None
 */
void OLED_DrawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t is_filled)
{
	int16_t minx = x0, miny = y0, maxx = x0, maxy = y0;
	int16_t i, j;
	int16_t vx[] = {x0, x1, x2};
	int16_t vy[] = {y0, y1, y2};
	
	if (!is_filled)  // If the triangle is not filled
	{
		/* Call the line drawing function to connect the three points with straight lines */
		OLED_DrawLine(x0, y0, x1, y1);
		OLED_DrawLine(x0, y0, x2, y2);
		OLED_DrawLine(x1, y1, x2, y2);
	}
	else  // If the triangle is filled
	{
		/* Find the minimum x and y coordinates of the three points */
		if (x1 < minx) {minx = x1;}
		if (x2 < minx) {minx = x2;}
		if (y1 < miny) {miny = y1;}
		if (y2 < miny) {miny = y2;}
		
		/* Find the maximum x and y coordinates of the three points */
		if (x1 > maxx) {maxx = x1;}
		if (x2 > maxx) {maxx = x2;}
		if (y1 > maxy) {maxy = y1;}
		if (y2 > maxy) {maxy = y2;}
		
		/* The rectangle between the minimum and maximum coordinates is the area that may need to be filled */
		for (i = minx; i <= maxx; i++)  // Iterate through the x-coordinates
		{
			for (j = miny; j <= maxy; j++)  // Iterate through the y-coordinates
			{
				// Call OLED_Pnpoly to determine if the specified point is within the specified triangle
				// If it is, draw a point; if not, do nothing
				if (OLED_Pnpoly(3, vx, vy, i, j)) {OLED_DrawPoint(i, j);}
			}
		}
	}
}

/**
 * @brief  Draw a circle on the OLED
 * @param  center_x The x-coordinate of the circle's center, range: [-32768,32767], screen area: [0,127]
 * @param  center_y The y-coordinate of the circle's center, range: [-32768,32767], screen area: [0,63]
 * @param  radius The radius of the circle, range: [0,255]
 * @param  is_filled Whether the rectangle is filled, range: OLED_UNFILLED (not filled) or OLED_FILLED (filled)
 * @retval None
 */
void OLED_DrawCircle(int16_t center_x, int16_t center_y, uint8_t radius, uint8_t is_filled)
{
	int16_t x, y, d, j;
	
	/* Use the Bresenham's algorithm to draw a circle, which avoids time-consuming floating-point operations and is more efficient */
	/* Reference document: https://www.cs.montana.edu/courses/spring2009/425/dslectures/Bresenham.pdf */
	/* Reference tutorial: https://www.bilibili.com/video/BV1VM4y1u7wJ */
	
	d = 1 - radius;
	x = 0;
	y = radius;
	
	/* Draw the starting point of each eighth of the arc */
	OLED_DrawPoint(center_x + x, center_y + y);
	OLED_DrawPoint(center_x - x, center_y - y);
	OLED_DrawPoint(center_x + y, center_y + x);
	OLED_DrawPoint(center_x - y, center_y - x);
	
	if (is_filled)  // If the circle is filled
	{
		/* Iterate through the starting point's y-coordinate */
		for (j = -y; j < y; j++)
		{
			// Draw points in the specified area to fill part of the circle
			OLED_DrawPoint(center_x, center_y + j);
		}
	}
	
	while (x < y)  // Iterate through each point on the x-axis
	{
		x++;
		if (d < 0)  // The next point is to the east of the current point
		{
			d += 2 * x + 1;
		}
		else  // The next point is to the southeast of the current point
		{
			y--;
			d += 2 * (x - y) + 1;
		}
		
		/* Draw points for each eighth of the arc */
		OLED_DrawPoint(center_x + x, center_y + y);
		OLED_DrawPoint(center_x + y, center_y + x);
		OLED_DrawPoint(center_x - x, center_y - y);
		OLED_DrawPoint(center_x - y, center_y - x);
		OLED_DrawPoint(center_x + x, center_y - y);
		OLED_DrawPoint(center_x + y, center_y - x);
		OLED_DrawPoint(center_x - x, center_y + y);
		OLED_DrawPoint(center_x - y, center_y + x);
		
		if (is_filled)  // If the circle is filled
		{
			/* Iterate through the middle part */
			for (j = -y; j < y; j++)
			{
				/* Draw points in the specified area to fill part of the circle */
				OLED_DrawPoint(center_x + x, center_y + j);
				OLED_DrawPoint(center_x - x, center_y + j);
			}
			
			/* Iterate through the two side parts */
			for (j = -x; j < x; j++)
			{
				/* Draw points in the specified area to fill part of the circle */
				OLED_DrawPoint(center_x - y, center_y + j);
				OLED_DrawPoint(center_x + y, center_y + j);
			}
		}
	}
}

/**
 * @brief  Draw an ellipse on the OLED
 * @param  center_x The x-coordinate of the ellipse's center, range: [-32768,32767], screen area: [0,127]
 * @param  center_y The y-coordinate of the ellipse's center, range: [-32768,32767], screen area: [0,63]
 * @param  a The length of the horizontal semiaxis of the ellipse, range: [0,255]
 * @param  b The length of the vertical semiaxis of the ellipse, range: [0,255]
 * @param  is_filled Whether the rectangle is filled, range: OLED_UNFILLED (not filled) or OLED_FILLED (filled)
 * @retval None
 */
void OLED_DrawEllipse(int16_t center_x, int16_t center_y, uint8_t a, uint8_t b, uint8_t is_filled)
{
	int16_t x, y, j;
	int16_t a_ = a, b_ = b;
	float d1, d2;
	
	/* Use the Bresenham's algorithm to draw an ellipse, which can avoid some time consuming of floating point operations and is more efficient */
	/* Reference link: https://blog.csdn.net/myf_666/article/details/128167392 */
	
	x = 0;
	y = b_;
	d1 = b_ * b_ + a_ * a_ * (-b_ + 0.5);
	
	if (is_filled)	 // If the ellipse is filled
	{
		/* Iterate through the starting point's y-coordinate */
		for (j = -y; j < y; j++)
		{
			/* Draw points in the specified area to fill part of the ellipse */
			OLED_DrawPoint(center_x, center_y + j);
			OLED_DrawPoint(center_x, center_y - j);
		}
	}
	
	/* Draw the starting point of the elliptical arc */
	OLED_DrawPoint(center_x + x, center_y + y);
	OLED_DrawPoint(center_x - x, center_y - y);
	OLED_DrawPoint(center_x - x, center_y + y);
	OLED_DrawPoint(center_x + x, center_y - y);
	
	/* Draw the middle part of the ellipse */
	while (b_ * b_ * (x + 1) < a_ * a_ * (y - 0.5))
	{
		if (d1 <= 0)  // The next point is to the east of the current point
		{
			d1 += b_ * b_ * (2 * x + 3);
		}
		else  // The next point is to the southeast of the current point
		{
			d1 += b_ * b_ * (2 * x + 3) + a_ * a_ * (-2 * y + 2);
			y--;
		}
		x++;
		
		if (is_filled)	 // If the ellipse is filled
		{
			/* Iterate through the middle part */
			for (j = -y; j < y; j++)
			{
				/* Draw points in the specified area to fill part of the ellipse */
				OLED_DrawPoint(center_x + x, center_y + j);
				OLED_DrawPoint(center_x - x, center_y + j);
			}
		}
		
		/* Draw the arc of the middle part of the ellipse */
		OLED_DrawPoint(center_x + x, center_y + y);
		OLED_DrawPoint(center_x - x, center_y - y);
		OLED_DrawPoint(center_x - x, center_y + y);
		OLED_DrawPoint(center_x + x, center_y - y);
	}
	
	// Draw the two side parts of the ellipse
	d2 = b_ * b_ * (x + 0.5) * (x + 0.5) + a_ * a_ * (y - 1) * (y - 1) - a_ * a_ * b_ * b_;
	
	while (y > 0)
	{
		if (d2 <= 0)  // The next point is to the east of the current point
		{
			d2 += b_ * b_ * (2 * x + 2) + a_ * a_ * (-2 * y + 3);
			x++;
		}
		else  // The next point is to the southeast of the current point
		{
			d2 += a_ * a_ * (-2 * y + 3);
		}
		y--;
		
		if (is_filled)	 // If the ellipse is filled
		{
			/* Iterate through the two side parts */
			for (j = -y; j < y; j++)
			{
				/* Draw points in the specified area to fill part of the ellipse */
				OLED_DrawPoint(center_x + x, center_y + j);
				OLED_DrawPoint(center_x - x, center_y + j);
			}
		}
		
		/* Draw the arc of the two side parts of the ellipse */
		OLED_DrawPoint(center_x + x, center_y + y);
		OLED_DrawPoint(center_x - x, center_y - y);
		OLED_DrawPoint(center_x - x, center_y + y);
		OLED_DrawPoint(center_x + x, center_y - y);
	}
}

/**
 * @brief  Draw an arc on the OLED
 * @param  center_x The x-coordinate of the arc's center, range: [-32768,32767], screen area: [0,127]
 * @param  center_y The y-coordinate of the arc's center, range: [-32768,32767], screen area: [0,63]
 * @param  radius The radius of the arc, range: [0,255]
 * @param  start_angle The starting angle of the arc, range: -180~180
 * @param  end_angle The ending angle of the arc, range: -180~180
 * @param  is_filled Whether the rectangle is filled, range: OLED_UNFILLED (not filled) or OLED_FILLED (filled)
 * @retval None
 * @note   0 degree is to the right horizontally, 180 or -180 degree is to the left horizontally.
 *         Positive angles are below the horizontal line, negative angles are above, and the rotation is clockwise.
 */
void OLED_DrawArc(int16_t center_x, int16_t center_y, uint8_t radius, int16_t start_angle, int16_t end_angle, uint8_t is_filled)
{
	int16_t x, y, d, j;
	
	/* This function borrows the circle drawing method of the Bresenham's algorithm */
	
	d = 1 - radius;
	x = 0;
	y = radius;
	
	/* When drawing each point of the circle, check if the specified point is within the specified angle range */
	/* If it is, draw the point; if not, do nothing */
	if (OLED_IsInAngle(x, y, start_angle, end_angle))	  {OLED_DrawPoint(center_x + x, center_y + y);}
	if (OLED_IsInAngle(-x, -y, start_angle, end_angle)) {OLED_DrawPoint(center_x - x, center_y - y);}
	if (OLED_IsInAngle(y, x, start_angle, end_angle))   {OLED_DrawPoint(center_x + y, center_y + x);}
	if (OLED_IsInAngle(-y, -x, start_angle, end_angle)) {OLED_DrawPoint(center_x - y, center_y - x);}
	
	if (is_filled)	 // If the arc is filled
	{
		/* Iterate through the starting point's y-coordinate */
		for (j = -y; j < y; j++)
		{
			if (OLED_IsInAngle(0, j, start_angle, end_angle)) {OLED_DrawPoint(center_x, center_y + j);}
		}
	}
	
	while (x < y)		// Iterate through each point on the x-axis
	{
		x++;
		if (d < 0)  // The next point is to the east of the current point
		{
			d += 2 * x + 1;
		}
		else  // The next point is to the southeast of the current point
		{
			y--;
			d += 2 * (x - y) + 1;
		}
		
		if (OLED_IsInAngle(x, y, start_angle, end_angle))   {OLED_DrawPoint(center_x + x, center_y + y);}
		if (OLED_IsInAngle(y, x, start_angle, end_angle))   {OLED_DrawPoint(center_x + y, center_y + x);}
		if (OLED_IsInAngle(-x, -y, start_angle, end_angle)) {OLED_DrawPoint(center_x - x, center_y - y);}
		if (OLED_IsInAngle(-y, -x, start_angle, end_angle)) {OLED_DrawPoint(center_x - y, center_y - x);}
		if (OLED_IsInAngle(x, -y, start_angle, end_angle))  {OLED_DrawPoint(center_x + x, center_y - y);}
		if (OLED_IsInAngle(y, -x, start_angle, end_angle))  {OLED_DrawPoint(center_x + y, center_y - x);}
		if (OLED_IsInAngle(-x, y, start_angle, end_angle))  {OLED_DrawPoint(center_x - x, center_y + y);}
		if (OLED_IsInAngle(-y, x, start_angle, end_angle))  {OLED_DrawPoint(center_x - y, center_y + x);}
		
		if (is_filled)	 // If the arc is filled
		{
			/* Iterate through the middle part */
			for (j = -y; j < y; j++)
			{
				if (OLED_IsInAngle(x, j, start_angle, end_angle))  {OLED_DrawPoint(center_x + x, center_y + j);}
				if (OLED_IsInAngle(-x, j, start_angle, end_angle)) {OLED_DrawPoint(center_x - x, center_y + j);}
			}
			
			/* Iterate through the two side parts */
			for (j = -x; j < x; j++)
			{
				if (OLED_IsInAngle(-y, j, start_angle, end_angle)) {OLED_DrawPoint(center_x - y, center_y + j);}
				if (OLED_IsInAngle(y, j, start_angle, end_angle))  {OLED_DrawPoint(center_x + y, center_y + j);}
			}
		}
	}
}
