/* Includes ------------------------------------------------------------------*/

#include "oled_font.h"
#include "oled.h"

/* Macros --------------------------------------------------------------------*/

#define SCL_Pin GPIO_PIN_13 // SCL --> PB13
#define SDA_Pin GPIO_PIN_15 // SDA --> PB15

#define OLED_W_SCL(x) HAL_GPIO_WritePin(GPIOB, SCL_Pin, (GPIO_PinState)(x))
#define OLED_W_SDA(x) HAL_GPIO_WritePin(GPIOB, SDA_Pin, (GPIO_PinState)(x))

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
  OLED_W_SCL(GPIO_PIN_SET); // Extra clock for acknowledgment, not used here
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
  OLED_I2C_SendByte(0x78); // Slave address
  OLED_I2C_SendByte(0x00); // Command mode
  OLED_I2C_SendByte(command);
  OLED_I2C_Stop();
}

/**
 * @brief  Write data to the OLED
 * @param  data The data to write
 * @retval None
 */
void OLED_WriteData(uint8_t data)
{
  OLED_I2C_Start();
  OLED_I2C_SendByte(0x78); // Slave address
  OLED_I2C_SendByte(0x40); // Data mode
  OLED_I2C_SendByte(data);
  OLED_I2C_Stop();
}

/**
 * @brief  Set the cursor position on OLED
 * @param  y Y-coordinate (0 to 7)
 * @param  x X-coordinate (0 to 127)
 * @retval None
 */
void OLED_SetCursor(uint8_t y, uint8_t x)
{
  OLED_WriteCommand(0xB0 | y);                 // Set Y position
  OLED_WriteCommand(0x10 | ((x & 0xF0) >> 4)); // Set X position high nibble
  OLED_WriteCommand(0x00 | (x & 0x0F));        // Set X position low nibble
}

/**
 * @brief  Clear the OLED display
 * @param  None
 * @retval None
 */
void OLED_Clear(void)
{
  uint8_t i, j;
  for (j = 0; j < 8; j++)
  {
    OLED_SetCursor(j, 0);
    for (i = 0; i < 128; i++)
    {
      OLED_WriteData(0x00);
    }
  }
}

/**
 * @brief  Display a character on OLED
 * @param  line The row to display (1 to 4)
 * @param  column The column to display (1 to 16)
 * @param  char The character to display (ASCII)
 * @retval None
 */
void OLED_ShowChar(uint8_t line, uint8_t column, char character)
{
  uint8_t i;
  OLED_SetCursor((line - 1) * 2, (column - 1) * 8); // Set cursor for upper part
  for (i = 0; i < 8; i++)
  {
    OLED_WriteData(OLED_F8x16[character - ' '][i]); // Display upper part
  }
  OLED_SetCursor((line - 1) * 2 + 1, (column - 1) * 8); // Set cursor for lower part
  for (i = 0; i < 8; i++)
  {
    OLED_WriteData(OLED_F8x16[character - ' '][i + 8]); // Display lower part
  }
}

/**
 * @brief  Display a string on OLED
 * @param  line The row to display (1 to 4)
 * @param  column The starting column (1 to 16)
 * @param  str The string to display
 * @retval None
 */
void OLED_ShowString(uint8_t line, uint8_t column, char *str)
{
  uint8_t i;
  for (i = 0; str[i] != '\0'; i++)
  {
    OLED_ShowChar(line, column + i, str[i]);
  }
}

/**
 * @brief  Power function for OLED (X^Y)
 * @retval The result of X raised to the power of Y
 */
uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
  uint32_t Result = 1;
  while (Y--)
  {
    Result *= X;
  }
  return Result;
}

/**
 * @brief  Display a number on OLED (unsigned, decimal)
 * @param  line The row to display (1 to 4)
 * @param  column The starting column (1 to 16)
 * @param  number The number to display
 * @param  length The number of digits to display (1 to 10)
 * @retval None
 */
void OLED_ShowNum(uint8_t line, uint8_t column, uint32_t number, uint8_t length)
{
  uint8_t i;
  for (i = 0; i < length; i++)
  {
    OLED_ShowChar(line, column + i, number / OLED_Pow(10, length - i - 1) % 10 + '0');
  }
}

/**
 * @brief  Display a signed decimal number on the OLED.
 * @param  line The row to display (1 to 4)
 * @param  column The starting column (1 to 16)
 * @param  number The number to display (-2147483648 to 2147483647)
 * @param  length The number of digits to display (1 to 10)
 * @retval None
 */
void OLED_ShowSignedNum(uint8_t line, uint8_t column, int32_t number, uint8_t length)
{
  uint8_t i;
  uint32_t Number1;
  if (number >= 0)
  {
    OLED_ShowChar(line, column, '+'); // Display positive sign
    Number1 = number;
  }
  else
  {
    OLED_ShowChar(line, column, '-'); // Display negative sign
    Number1 = -number;
  }
  for (i = 0; i < length; i++)
  {
    OLED_ShowChar(line, column + i + 1, Number1 / OLED_Pow(10, length - i - 1) % 10 + '0');
  }
}

/**
 * @brief  Display a hexadecimal number on the OLED.
 * @param  line The row to display (1 to 4)
 * @param  column The starting column (1 to 16)
 * @param  number The number to display (0 to 0xFFFFFFFF)
 * @param  length The number of digits to display (1 to 8)
 * @retval None
 */
void OLED_ShowHexNum(uint8_t line, uint8_t column, uint32_t number, uint8_t length)
{
  uint8_t i, SingleNumber;
  for (i = 0; i < length; i++)
  {
    SingleNumber = number / OLED_Pow(16, length - i - 1) % 16;
    if (SingleNumber < 10)
    {
      OLED_ShowChar(line, column + i, SingleNumber + '0'); // Display 0-9
    }
    else
    {
      OLED_ShowChar(line, column + i, SingleNumber - 10 + 'A'); // Display A-F
    }
  }
}

/**
 * @brief  Display a binary number on the OLED.
 * @param  line The row to display (1 to 4)
 * @param  column The starting column (1 to 16)
 * @param  number The number to display (0 to 1111 1111 1111 1111)
 * @param  length The number of digits to display (1 to 16)
 * @retval None
 */
void OLED_ShowBinNum(uint8_t line, uint8_t column, uint32_t number, uint8_t length)
{
  uint8_t i;
  for (i = 0; i < length; i++)
  {
    OLED_ShowChar(line, column + i, number / OLED_Pow(2, length - i - 1) % 2 + '0'); // Display 0 or 1
  }
}

/**
 * @brief  Initialize the OLED display
 * @param  None
 * @retval None
 */
void OLED_Init(void)
{
  uint32_t i, j;

  // Power-up delay
  for (i = 0; i < 1000; i++)
  {
    for (j = 0; j < 1000; j++)
      ;
  }

  OLED_I2C_Init(); // Initialize I2C pins

  OLED_WriteCommand(0xAE); // Turn off display

  OLED_WriteCommand(0xD5); // Set display clock divide ratio/oscillator frequency
  OLED_WriteCommand(0x80);

  OLED_WriteCommand(0xA8); // Set multiplex ratio
  OLED_WriteCommand(0x3F);

  OLED_WriteCommand(0xD3); // Set display offset
  OLED_WriteCommand(0x00);

  OLED_WriteCommand(0x40); // Set display start line

  OLED_WriteCommand(0xA1); // Set segment re-map (normal)

  OLED_WriteCommand(0xC8); // Set COM output scan direction (normal)

  OLED_WriteCommand(0xDA); // Set COM pins hardware configuration
  OLED_WriteCommand(0x12);

  OLED_WriteCommand(0x81); // Set contrast control
  OLED_WriteCommand(0xCF);

  OLED_WriteCommand(0xD9); // Set pre-charge period
  OLED_WriteCommand(0xF1);

  OLED_WriteCommand(0xDB); // Set VCOMH deselect level
  OLED_WriteCommand(0x30);

  OLED_WriteCommand(0xA4); // Entire display on/off (resume to RAM content)

  OLED_WriteCommand(0xA6); // Set normal display

  OLED_WriteCommand(0x8D); // Enable charge pump
  OLED_WriteCommand(0x14);

  OLED_WriteCommand(0xAF); // Turn on OLED panel

  OLED_Clear(); // Clear OLED screen
}
