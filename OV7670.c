/*
===============================================================================
 Name        : ov7670.c
 Author      : Upi Tamminen
 Version     : 1.0
 Copyright   : Upi Tamminen (2012)
 Description : ov7670 register access & image reading - modified my mkir
===============================================================================
*/

#include <stdio.h>
#include "lpc17xx.h"
#include "ov7670.h"
#include "ov7670reg.h"
#include "type.h"
#include "i2c.h"
#include "delay.h"
#include "uart.h"

/* Definitions used to check if corresponding pin is high */

#define D0 (LPC_GPIO1->FIOPIN & (1 << 19))
#define D1 (LPC_GPIO1->FIOPIN & (1 << 20))
#define D2 (LPC_GPIO1->FIOPIN & (1 << 21))
#define D3 (LPC_GPIO1->FIOPIN & (1 << 22))
#define D4 (LPC_GPIO1->FIOPIN & (1 << 23))
#define D5 (LPC_GPIO1->FIOPIN & (1 << 24))
#define D6 (LPC_GPIO1->FIOPIN & (1 << 25))
#define D7 (LPC_GPIO1->FIOPIN & (1 << 26))

#define VSYNC (LPC_GPIO0->FIOPIN & (1 << 1)) 
#define HREF  (LPC_GPIO2->FIOPIN & (1 << 13))  
#define RCK  (LPC_GPIO1->FIOPIN & (1 << 27))

	

uint8_t qqvgaframe1[QQVGA_HEIGHT * QQVGA_WIDTH]; /* rgb565 byte - QQVGA_WIDTH = 160, QQVGA_HEIGHT = 120 */
	
uint32_t count1, count2;	 
	 

uint32_t ov7670_set(uint8_t addr, uint8_t val)
{
    i2c_clearbuffers();

    I2CWriteLength = 3;
    I2CReadLength = 0;
    I2CMasterBuffer[0] = OV7670_ADDR;   /* i2c address - defined sto ov7670.h. Omoiws kai gia ti get */
    I2CMasterBuffer[1] = addr;          /* key */
    I2CMasterBuffer[2] = val;           /* value */

    return I2CEngine();
}

uint8_t ov7670_get(uint8_t addr)
{
    i2c_clearbuffers();
    I2CWriteLength = 2;
    I2CReadLength = 0;
    I2CMasterBuffer[0] = OV7670_ADDR;   /* i2c address */
    I2CMasterBuffer[1] = addr;          /* key */

    I2CEngine();

    Delay(1);

    i2c_clearbuffers();
    I2CWriteLength = 0;
    I2CReadLength = 1;
    I2CMasterBuffer[0] = OV7670_ADDR | RD_BIT;

    while (I2CEngine() == I2CSTATE_SLA_NACK);

    return I2CSlaveBuffer[0];
}

void ov7670_init(void)
{
    UART0_SendString("Initializing ov7670...");

	/* port 0.1 - VSYNC */
    LPC_PINCON->PINSEL0 &= ~(3 << 2); /* function = GPIO */
    LPC_GPIO0->FIODIR &= ~(1 << 1); /* direction = INPUT */

	/* port 2.13 - HREF  */
    LPC_PINCON->PINSEL4 &= ~(3 << 26); /* function = GPIO */
    LPC_GPIO2->FIODIR &= ~(1 << 13); /* direction = INPUT */

//	/* port 0.29 - NOT USED */
//	LPC_PINCON->PINSEL1 &= ~(3 << 26); /* function = gpio */
//	LPC_GPIO0->FIODIR |= (1 << 29); /* direction = OUTPUT */

	/* port 1.29 - RRST */
    LPC_PINCON->PINSEL3 &= ~(3 << 26); /* function = gpio */
	LPC_GPIO1->FIODIR |= (1 << 29); /* direction = OUTPUT */
	LPC_GPIO1->FIOSET |= (1 << 29);  /* Make pin HIGH to prevent reset */

	/* port 0.0 - WEN */
    LPC_PINCON->PINSEL0 &= ~(3 << 0); /* function = gpio */
    LPC_GPIO0->FIODIR |= (1 << 0); /* direction = OUTPUT  */
	LPC_GPIO0->FIOCLR |= (1 << 0); /* Make pin LOW to enable FIFO write */

	/* port 1.27 - RCK */
    LPC_PINCON->PINSEL3 &= ~(3 << 22); /* function = gpio */
	LPC_GPIO1->FIODIR |= (1 << 27); /* direction = OUTPUT */
	LPC_GPIO1->FIOSET |= (1 << 0);  /* Make pin HIGH to prevent reset */

//   LPC_PINCON->PINSEL3 |= (1 << 22); /* function = CLKOUT */
//	LPC_PINCON->PINSEL3 |= (0 << 23); /* function = CLKOUT */
//	LPC_SC->CLKOUTCFG |= (1 << 8); /* Enable CLKOUT sto P1.27 */
//	LPC_SC->CLKOUTCFG |= (1 << 8)| (9 << 4);  /* Enable and divide CCLK by 10 */

	/* port 1.28 OE - CONNECTED - checked */
    LPC_PINCON->PINSEL3 &= ~(3 << 24); /* function = gpio */
    LPC_GPIO1->FIODIR |= (1 << 28); /* direction = OUTPUT */
	LPC_GPIO1->FIOCLR |= (1 << 28);  /* Make pin LOW to enable FIFO read */

    /* ports D0..D7 go to P1.19..P1.26 - checked */
    //LPC_PINCON->PINSEL3 &= ~(0xFFFF); /* function = gpio ? */
	LPC_PINCON->PINSEL3 &= ~(3 << 6);
	LPC_PINCON->PINSEL3 &= ~(3 << 8);
	LPC_PINCON->PINSEL3 &= ~(3 << 10);
	LPC_PINCON->PINSEL3 &= ~(3 << 12);
	LPC_PINCON->PINSEL3 &= ~(3 << 14);
	LPC_PINCON->PINSEL3 &= ~(3 << 16);
	LPC_PINCON->PINSEL3 &= ~(3 << 18);
	LPC_PINCON->PINSEL3 &= ~(3 << 20);
    //LPC_GPIO1->FIODIR &= ~(0x07F80000); /* direction = input ? for pins 1.19 - 1.26 */
	LPC_GPIO1->FIODIR &= ~(1 << 19);
	LPC_GPIO1->FIODIR &= ~(1 << 20);
	LPC_GPIO1->FIODIR &= ~(1 << 21);
	LPC_GPIO1->FIODIR &= ~(1 << 22);
	LPC_GPIO1->FIODIR &= ~(1 << 23);
	LPC_GPIO1->FIODIR &= ~(1 << 24);
	LPC_GPIO1->FIODIR &= ~(1 << 25);
	LPC_GPIO1->FIODIR &= ~(1 << 26);

/********************** CAMERA INITIALIZATION 1 ********************/

    ov7670_set(REG_COM7, 0x80); /* reset to default values */
	ov7670_set(REG_COM7, 0x04); /* rgb */
    ov7670_set(REG_CLKRC, 0x80);
//    ov7670_set(REG_COM11, 0x0A);
    ov7670_set(REG_TSLB, 0x04);
//	ov7670_set(REG_COM8, 0x8F);
//	ov7670_set(REG_COM16, 0x18);
//	ov7670_set(REG_AWBCTR0,  1 << 0);


//	ov7670_set(REG_RGB444, 0x00); /* disable RGB444 */
	ov7670_set(REG_COM15, 0xD0); /* set RGB565 */
//    ov7670_set(REG_COM15, 0xC0); /* set to RAW RGB565 */
//
/*************** VGA - Raw Bayer RGB settings ***************/

// 	ov7670_set(REG_CLKRC, 0x80);
//	ov7670_set(REG_COM7,  0x01);
//    ov7670_set(REG_COM3,  0x00);
//    ov7670_set(REG_COM14, 0x00);
//	ov7670_set(SCALING_XSC, 0x3A);
//	ov7670_set(SCALING_YSC, 0x35);
//	ov7670_set(SCALING_DCWCTR, 0x11);
//	ov7670_set(SCALING_PCLK_DIV, 0xF0);
//	ov7670_set(SCALING_PCLK_DELAY, 0x02);


/******** General settings **************/

    ov7670_set(REG_HSTART, 0x16);
    ov7670_set(REG_HSTOP, 0x04);
    ov7670_set(REG_HREF, 0x24);
    ov7670_set(REG_VSTART, 0x02);
    ov7670_set(REG_VSTOP, 0x7a);
    ov7670_set(REG_VREF, 0x0a);

    ov7670_set(REG_COM10, 0x02);   /* VSYNC polarity - me positive polarity (0x00) den pairnw tipota gia RGB 565*/
//	ov7670_set(REG_COM10, 0x00);   /* VSYNC polarity - me negative polarity (0x02) den pairnw tipota gia Raw RGB */
    ov7670_set(REG_COM3, 0x04);

  ov7670_set(REG_COM14, 0x1a); /* divide PCLK by 4 - why? */
//	ov7670_set(REG_COM14, 0x1b); /* divide PCLK by 8 */
 	ov7670_set(REG_MVFP, 0x27);	/* Mirror/VFlip: Enable/Disable (Datasheet - pg.15) */
   ov7670_set(0x72, 0x22); /* Vertical and Horizontal down sample by 4 */
  //ov7670_set(0x72, 0x33); /* Vertical and Horizontal down sample by 8 */
	ov7670_set(0x73, 0xf2); /* divide PCLK by 4 - changes in combination with COM14 */
//  ov7670_set(0x73, 0xf3); /* divide PCLK by 8 - changes in combination with COM14 */

/********* Test pattern *********/

 	//ov7670_set(0x70, 1 << 7);
	
// 	ov7670_set(0x70, 0x8A);
//	ov7670_set(0x71, 0 << 7);
//  ov7670_set(0x71, 0x85);


/***************** COLOR SETTINGS ********************/

    ov7670_set(0x4f, 0x80);
    ov7670_set(0x50, 0x80);
    ov7670_set(0x51, 0x00);
    ov7670_set(0x52, 0x22);
    ov7670_set(0x53, 0x5e);
    ov7670_set(0x54, 0x80);
    ov7670_set(0x56, 0x40);
    ov7670_set(0x58, 0x9e);
    ov7670_set(0x59, 0x88);
    ov7670_set(0x5a, 0x88);  /* Oi registers apo 5A mehri 5E iparhoun mono sto implementation guide */
    ov7670_set(0x5b, 0x44);
    ov7670_set(0x5c, 0x67);
    ov7670_set(0x5d, 0x49);
    ov7670_set(0x5e, 0x0e);
    ov7670_set(0x69, 0x00);
    ov7670_set(0x6a, 0x40);
    ov7670_set(0x6b, 0x0a);
    ov7670_set(0x6c, 0x0a);
    ov7670_set(0x6d, 0x55);
    ov7670_set(0x6e, 0x11);
    ov7670_set(0x6f, 0x9f);

    ov7670_set(0xb0, 0x84);	/* Einai reserved - giatio to allazei? (I.G. - pg. 61) */

///********************** CAMERA INITIALIZATION 2 ********************/

//    ov7670_set(REG_COM7, 0x80); /* reset to default values */
//	ov7670_set(REG_COM7, 0x02);
//    ov7670_set(REG_CLKRC, 0x80);


// 	ov7670_set(0x3a, 0x04);//
//	ov7670_set(0x40, 0x10);
//	ov7670_set(0x12, 0x14);
//	ov7670_set(0x32, 0x80);
//	ov7670_set(0x17, 0x16);
//        
//	ov7670_set(0x18, 0x04);
//	ov7670_set(0x19, 0x02);
//	ov7670_set(0x1a, 0x7b);
//	ov7670_set(0x03, 0x06);
//	ov7670_set(0x0c, 0x0c);
//    ov7670_set(0x15, 0x00);
//	ov7670_set(0x3e, 0x00);
//	ov7670_set(0x70, 0x00);
//	ov7670_set(0x71, 0x01);
//	ov7670_set(0x72, 0x11);
//	ov7670_set(0x73, 0x09);
//        
//	ov7670_set(0xa2, 0x02);
//	ov7670_set(0x11, 0x00);
//	ov7670_set(0x7a, 0x20);
//	ov7670_set(0x7b, 0x1c);
//	ov7670_set(0x7c, 0x28);
//        
//	ov7670_set(0x7d, 0x3c);
//	ov7670_set(0x7e, 0x55);
//	ov7670_set(0x7f, 0x68);
//	ov7670_set(0x80, 0x76);
//	ov7670_set(0x81, 0x80);
//        
//	ov7670_set(0x82, 0x88);
//	ov7670_set(0x83, 0x8f);
//	ov7670_set(0x84, 0x96);
//	ov7670_set(0x85, 0xa3);
//	ov7670_set(0x86, 0xaf);
//        
//	ov7670_set(0x87, 0xc4);
//	ov7670_set(0x88, 0xd7);
//	ov7670_set(0x89, 0xe8);
//	ov7670_set(0x13, 0xe0);
//	ov7670_set(0x00, 0x00); /* AGC */
//        
//	ov7670_set(0x10, 0x00);
//	ov7670_set(0x0d, 0x00);
//	ov7670_set(0x14, 0x20);
//	ov7670_set(0xa5, 0x05);
//	ov7670_set(0xab, 0x07);
//        
//	ov7670_set(0x24, 0x75);
//	ov7670_set(0x25, 0x63);
//	ov7670_set(0x26, 0xA5);
//	ov7670_set(0x9f, 0x78);
//	ov7670_set(0xa0, 0x68);
//        
//	ov7670_set(0xa1, 0x03);
//	ov7670_set(0xa6, 0xdf);
//	ov7670_set(0xa7, 0xdf);
//	ov7670_set(0xa8, 0xf0);
//	ov7670_set(0xa9, 0x90);
//        
//	ov7670_set(0xaa, 0x94);
//	ov7670_set(0x13, 0xe5);
//	ov7670_set(0x0e, 0x61);
//	ov7670_set(0x0f, 0x4b);
//	ov7670_set(0x16, 0x02);
//        
//	ov7670_set(0x1e, 0x37); 
//	ov7670_set(0x21, 0x02);
//	ov7670_set(0x22, 0x91);
//	ov7670_set(0x29, 0x07);
//	ov7670_set(0x33, 0x0b);
//        
//	ov7670_set(0x35, 0x0b);
//	ov7670_set(0x37, 0x1d);
//	ov7670_set(0x38, 0x71);
//	ov7670_set(0x39, 0x2a);
//	ov7670_set(0x3c, 0x78);
//        
//	ov7670_set(0x4d, 0x40);
//	ov7670_set(0x4e, 0x20);
//	ov7670_set(0x69, 0x5d);
//	ov7670_set(0x6b, 0x40);
//	ov7670_set(0x74, 0x19);
//	ov7670_set(0x8d, 0x4f);
//        
//	ov7670_set(0x8e, 0x00);
//	ov7670_set(0x8f, 0x00);
//	ov7670_set(0x90, 0x00);
//	ov7670_set(0x91, 0x00);
//	ov7670_set(0x92, 0x00);
//        
//	ov7670_set(0x96, 0x00);
//	ov7670_set(0x9a, 0x80);
//	ov7670_set(0xb0, 0x84);
//	ov7670_set(0xb1, 0x0c);
//	ov7670_set(0xb2, 0x0e);
//        
//	ov7670_set(0xb3, 0x82);
//	ov7670_set(0xb8, 0x0a);
//	ov7670_set(0x43, 0x14);
//	ov7670_set(0x44, 0xf0);
//	ov7670_set(0x45, 0x34);
//        
//	ov7670_set(0x46, 0x58);
//	ov7670_set(0x47, 0x28);
//	ov7670_set(0x48, 0x3a);
//	ov7670_set(0x59, 0x88);
//	ov7670_set(0x5a, 0x88);
//        
//	ov7670_set(0x5b, 0x44);
//	ov7670_set(0x5c, 0x67);
//	ov7670_set(0x5d, 0x49);
//	ov7670_set(0x5e, 0x0e);
//	ov7670_set(0x64, 0x04);
//	ov7670_set(0x65, 0x20);
//        
//	ov7670_set(0x66, 0x05);
//	ov7670_set(0x94, 0x04);
//	ov7670_set(0x95, 0x08);
//	ov7670_set(0x6c, 0x0a);
//	ov7670_set(0x6d, 0x55);
//                
//	ov7670_set(0x4f, 0x80);
//	ov7670_set(0x50, 0x80);
//	ov7670_set(0x51, 0x00);
//	ov7670_set(0x52, 0x22);
//	ov7670_set(0x53, 0x5e);
//	ov7670_set(0x54, 0x80);
//       
//	ov7670_set(0x6e, 0x11);
//	ov7670_set(0x6f, 0x9f);
//    ov7670_set(0x55, 0x00); /* 亮度 */
//    ov7670_set(0x56, 0x40); /* 对比度 */
//    ov7670_set(0x57, 0x80); /* change according to Jim's request */
   
    UART0_SendString("Done!");
	Delay(6000);
}	 


void ov7670_read_565_frame(void)
{

    uint32_t i = 0;

//	LPC_GPIO1->FIOCLR |= (1 << 29);  /* Make pin HIGH to prevent reset */
	//LPC_GPIO1->FIOSET |= (1 << 29);  /* Make pin HIGH to prevent reset */
	

   { while (VSYNC); /* wait for the old frame to end, since VSYNC is positive during a frame read*/
    while (!VSYNC); /* wait for a new frame to start */

   while (VSYNC) { 

  		
       // while (!VSYNC && !HREF); /* wait for a line to start */
        //if (VSYNC) break; /* line didn't start, but frame ended */
        while (HREF) {  /* wait for a line to end */
  		   		    
		   /* first byte */
		    /* wait for clock to go high */
		   LPC_GPIO1->FIOSET |= (1 << 27);
           qqvgaframe1[i] = ((LPC_GPIO1->FIOPIN >> 19) & 0x00FF);
		  //LPC_GPIO0->FIOCLR |= (1 << 29);
		  LPC_GPIO1->FIOCLR |= (1 << 27); /* wait for clock to go back low */ 
		  // LPC_GPIO1->FIOCLR |= (1 << 28);
		  i++;
	 		  }

		count1 = i;

    }
	 
	 for (count2 = 0; count2 <= count1; count2++)
		   {	   	
		   
		  // LCD_WriteData(qqvgaframe1[count2]);
		  
		   chartohex(qqvgaframe1[count2]);

		   }
		  

     printf("Pixel count 2 = :%d", count2);

	 Delay(2000);

 	}

 }

