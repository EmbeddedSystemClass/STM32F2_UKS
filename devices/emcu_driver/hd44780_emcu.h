#ifndef hd44780_emcu_H
#define hd44780_emcu_H


    #include "stm32F10x.h" // ����������� ����� CMSIS ��� STM32.
    void    Delay50cyc (void);
    void    LCD_CMD         (u32 CMD);
    u8      LCD_DAT         (u8 DAT);
    void    LCD_PUTCHAR     (u32 DAT);
    void    LCD_STRING      (u8 *STRING);
    void    LCD_INIT        (void);
    void    LCD_GOTO        (unsigned char str,unsigned char n);
    u8      LCD_READ        (void);
    void    LCD_VIEW_MODE   (u32 CURSOR1, u32 CURSOR2, u32 VIEW_ON);
    void    CREATE_CHAR     (char ADR, char Y1,char Y2,char Y3,char Y4,char Y5,char Y6,char Y7,char Y8);

    #define  NUM_OF_LINE        4   // ����� ����� LCD.
    #define  CHAR_IN_LINE       20  // ����� �������� � ������ ����� LCD.
    #define  LCD_C_PORT         PORTB // ���� ����������� �������� CD, RW, EN.
	#define  LCD_CD             7 // ������ CD �������/������.
	#define  LCD_RW             6 // ������ RW ������/������.
	#define  LCD_EN             5 // ������ EN ���������� ������ �� ����.
    #define  LCD_D_PORT         PORTB // ���� ���� ������
    #define  LCD_D_SHIFT        12 // �������� �� ������ ����� �� ���� ������.
    #define  LCD_D_MASK         0x0FUL<<LCD_D_SHIFT // ����� ���� ������.
    #define  ON                 1 // �������� ��������� "��������".
    #define  OFF                0 // �������� ��������� "���������".
#endif
