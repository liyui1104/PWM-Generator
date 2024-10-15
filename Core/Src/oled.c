#include "OLED_Font.h"
#include "oled.h"

/* Pin definitions */
#define SCL_Pin GPIO_PIN_13 // SCL --> PB13
#define SDA_Pin GPIO_PIN_15 // SDA --> PB15

#define OLED_W_SCL(x) HAL_GPIO_WritePin(GPIOB, SCL_Pin, (GPIO_PinState)(x))
#define OLED_W_SDA(x) HAL_GPIO_WritePin(GPIOB, SDA_Pin, (GPIO_PinState)(x))

/* Pin Initialization */
void OLED_I2C_Init(void)
{
  OLED_W_SCL(GPIO_PIN_SET);
  OLED_W_SDA(GPIO_PIN_SET);
}

/**
 * @brief  I2C Start condition
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
 * @brief  I2C Stop condition
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
 * @brief  I2C Send a byte of data
 * @param  Byte The byte to send
 * @retval None
 */
void OLED_I2C_SendByte(uint8_t Byte)
{
  uint8_t i;
  for (i = 0; i < 8; i++)
  {
    OLED_W_SDA(Byte & (0x80 >> i));
    OLED_W_SCL(GPIO_PIN_SET);
    OLED_W_SCL(GPIO_PIN_RESET);
  }
  OLED_W_SCL(GPIO_PIN_SET); // Extra clock for acknowledgment, not used here
  OLED_W_SCL(GPIO_PIN_RESET);
}

/**
 * @brief  Write a command to the OLED
 * @param  Command The command to write
 * @retval None
 */
void OLED_WriteCommand(uint8_t Command)
{
  OLED_I2C_Start();
  OLED_I2C_SendByte(0x78); // Slave address
  OLED_I2C_SendByte(0x00); // Command mode
  OLED_I2C_SendByte(Command);
  OLED_I2C_Stop();
}

/**
 * @brief  Write data to the OLED
 * @param  Data The data to write
 * @retval None
 */
void OLED_WriteData(uint8_t Data)
{
  OLED_I2C_Start();
  OLED_I2C_SendByte(0x78); // Slave address
  OLED_I2C_SendByte(0x40); // Data mode
  OLED_I2C_SendByte(Data);
  OLED_I2C_Stop();
}

/**
 * @brief  Set the cursor position on OLED
 * @param  Y Y-coordinate (0 to 7)
 * @param  X X-coordinate (0 to 127)
 * @retval None
 */
void OLED_SetCursor(uint8_t Y, uint8_t X)
{
  OLED_WriteCommand(0xB0 | Y);                 // Set Y position
  OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4)); // Set X position high nibble
  OLED_WriteCommand(0x00 | (X & 0x0F));        // Set X position low nibble
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
 * @param  Line The row to display (1 to 4)
 * @param  Column The column to display (1 to 16)
 * @param  Char The character to display (ASCII)
 * @retval None
 */
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)
{
  uint8_t i;
  OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8); // Set cursor for upper part
  for (i = 0; i < 8; i++)
  {
    OLED_WriteData(OLED_F8x16[Char - ' '][i]); // Display upper part
  }
  OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8); // Set cursor for lower part
  for (i = 0; i < 8; i++)
  {
    OLED_WriteData(OLED_F8x16[Char - ' '][i + 8]); // Display lower part
  }
}

/**
 * @brief  Display a string on OLED
 * @param  Line The row to display (1 to 4)
 * @param  Column The starting column (1 to 16)
 * @param  String The string to display
 * @retval None
 */
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String)
{
  uint8_t i;
  for (i = 0; String[i] != '\0'; i++)
  {
    OLED_ShowChar(Line, Column + i, String[i]);
  }
}

/**
 * @brief  Power function for OLED (X^Y)
 * @retval Returns the result of X raised to the power of Y
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
 * @param  Line The row to display (1 to 4)
 * @param  Column The starting column (1 to 16)
 * @param  Number The number to display
 * @param  Length The number of digits to display (1 to 10)
 * @retval None
 */
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
  uint8_t i;
  for (i = 0; i < Length; i++)
  {
    OLED_ShowChar(Line, Column + i, Number / OLED_Pow(10, Length - i - 1) % 10 + '0');
  }
}

/**
 * @brief  Display a signed decimal number on the OLED.
 * @param  Line The row to display (1 to 4)
 * @param  Column The starting column (1 to 16)
 * @param  Number The number to display (-2147483648 to 2147483647)
 * @param  Length The number of digits to display (1 to 10)
 * @retval None
 */
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length)
{
  uint8_t i;
  uint32_t Number1;
  if (Number >= 0)
  {
    OLED_ShowChar(Line, Column, '+'); // Display positive sign
    Number1 = Number;
  }
  else
  {
    OLED_ShowChar(Line, Column, '-'); // Display negative sign
    Number1 = -Number;
  }
  for (i = 0; i < Length; i++)
  {
    OLED_ShowChar(Line, Column + i + 1, Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0');
  }
}

/**
 * @brief  Display a hexadecimal number on the OLED.
 * @param  Line The row to display (1 to 4)
 * @param  Column The starting column (1 to 16)
 * @param  Number The number to display (0 to 0xFFFFFFFF)
 * @param  Length The number of digits to display (1 to 8)
 * @retval None
 */
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
  uint8_t i, SingleNumber;
  for (i = 0; i < Length; i++)
  {
    SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;
    if (SingleNumber < 10)
    {
      OLED_ShowChar(Line, Column + i, SingleNumber + '0'); // Display 0-9
    }
    else
    {
      OLED_ShowChar(Line, Column + i, SingleNumber - 10 + 'A'); // Display A-F
    }
  }
}

/**
 * @brief  Display a binary number on the OLED.
 * @param  Line The row to display (1 to 4)
 * @param  Column The starting column (1 to 16)
 * @param  Number The number to display (0 to 1111 1111 1111 1111)
 * @param  Length The number of digits to display (1 to 16)
 * @retval None
 */
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
  uint8_t i;
  for (i = 0; i < Length; i++)
  {
    OLED_ShowChar(Line, Column + i, Number / OLED_Pow(2, Length - i - 1) % 2 + '0'); // Display 0 or 1
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
