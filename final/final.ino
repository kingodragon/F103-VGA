#include "bluevga.h"
#include "font.h"
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

BlueVGA vga(ASCII_FONT);
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); //hardware wrie
//U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, /* clock=*/ PB8, /* data=*/ PB9, /* reset=*/ U8X8_PIN_NONE);  //software wrie
int32_t channel_1_start=0, channel_1_stop=0, channel_1=0;
uint8_t DHT11=PA0,set_t=15;
uint8_t I_RH,D_RH,I_Temp,D_Temp,CheckSum,DHT11_data[50],index1=0;
uint8_t xCenter = (VRAM_WIDTH - 14) / 2;
char temp[30]="",hum[30]="";

void setup() {
   systick_enable();
  // put your setup code here, to run once:
   nvic_irq_set_priority(NVIC_TIMER1_CC, 0xF);
   nvic_irq_set_priority(NVIC_TIMER4, 0xF);
   nvic_irq_set_priority(NVIC_TIMER2, 0x0);
   vga.clearScreen();
   vga.setTextColor(RGB_GREEN, RGB_BLACK);
   //disable USB to enable PA11，PA12
   Serial.end();
   pinMode(PA11, OUTPUT);
   pinMode(PA12, OUTPUT);
   digitalWrite(PA11, LOW);
   digitalWrite(PA12, LOW);
  //remap I2C1 SSD1306
  
   I2C1->sda_pin = 9;
   I2C1->scl_pin = 8;
   Wire.setClock(400000);
   
   u8g2.begin();
   u8g2.enableUTF8Print();   // enable UTF8 support for the Arduino print() function
   u8g2.setFont(u8g2_font_cu12_tr);  // use chinese2 for all the glyphs of "你好世界"
   
   Serial2.begin(115200);
   Serial2.println(F("DHTxx test!"));

}

void loop() {
  // put your main code here, to run repeatedly:
    Request() ;  /* send start pulse */
    vga.clearScreen();
    vga.waitVSync(5);  // 16ms
    
    Serial2.println(index1);
    if(index1==41&&((I_RH + D_RH + I_Temp + D_Temp) == CheckSum))
    {
      Receive_data();
      sprintf(temp, "%s%d%c%d %s" , "    ",I_Temp,'.' ,D_Temp,"C"); 
      sprintf(hum, "%s%d%c%d %c    " , "    ",I_RH,'.',D_RH,'%'); 
      char set_t_c[20]="";
      sprintf(set_t_c, "     %d C" ,set_t); 
      if(I_Temp>set_t )
      {
        digitalWrite(PA12, HIGH);
        
        u8g2.firstPage();
        do {        
          u8g2.drawXBM(0, 0,  16, 16,hanz[0]);
          u8g2.drawXBM(16, 0,  16, 16,hanz[1]);
          u8g2.setCursor(16, 0+16);
          u8g2.print(':'+(String)temp);
          u8g2.drawXBM(0, 20,  16, 16,hanz[2]);
          u8g2.drawXBM(16, 20,  16, 16,hanz[3]);
          u8g2.setCursor(16, 20+16);
          u8g2.print(':'+(String)hum);
          u8g2.drawXBM(0, 40,  16, 16,hanz[4]);
          u8g2.drawXBM(16, 40,  16, 16,hanz[5]);
          u8g2.drawXBM(16+20, 40,  16, 16,hanz[6]);
          u8g2.drawXBM(32+20, 40,  16, 16,hanz[7]);
        }while ( u8g2.nextPage() );
        
        vga.waitVSync(1);  // 16ms
        vga.printStr(xCenter, 18, vga.getColorCode(RGB_RED, RGB_BLACK), (char *) "    Turns ");
      }
       else
      {

          u8g2.firstPage();
            do {        
              u8g2.drawXBM(0, 0,  16, 16,hanz[0]);
              u8g2.drawXBM(16, 0,  16, 16,hanz[1]);
              u8g2.setCursor(16, 0+16);
              u8g2.print(':'+(String)temp);
              u8g2.drawXBM(0, 20,  16, 16,hanz[2]);
              u8g2.drawXBM(16, 20,  16, 16,hanz[3]);
              u8g2.setCursor(16, 20+16);
              u8g2.print(':'+(String)hum);
              u8g2.drawXBM(0, 40,  16, 16,hanz[4]);
              u8g2.drawXBM(16, 40,  16, 16,hanz[5]);
              u8g2.drawXBM(16+20, 40,  16, 16,hanz[8]);
              u8g2.drawXBM(32+20, 40,  16, 16,hanz[9]);
           } while ( u8g2.nextPage() );
           
          vga.waitVSync(1);  // 16ms
          vga.printStr(xCenter, 18, vga.getColorCode(RGB_YELLOW, RGB_BLACK), (char *) "    Stops");
      }
      vga.printStr((VRAM_WIDTH - 18) / 2, 2, vga.getColorCode(RGB_YELLOW, RGB_BLACK), (char *)"WXl's STM32F103 VGA!");
      vga.printStr(xCenter, 8, vga.getColorCode(RGB_RED, RGB_BLACK), (char *)    "--Temperature--");
      vga.printStr(xCenter, 10, vga.getColorCode(RGB_MAGENTA, RGB_BLACK), (char *) temp);
      vga.printStr(xCenter, 12, vga.getColorCode(RGB_BLUE, RGB_BLACK), (char *)  "---Humidity---");
      vga.printStr(xCenter, 14, vga.getColorCode(RGB_CYAN, RGB_BLACK), (char *)   hum);
      vga.printStr(xCenter, 16, vga.getColorCode(RGB_GREEN, RGB_BLACK), (char *)  "--Fan  Status--    ");
      vga.printStr(xCenter, 20, vga.getColorCode(RGB_WHITE, RGB_BLACK), (char *)  "---threshold---");
      vga.printStr(xCenter, 22, vga.getColorCode(RGB_BLUE, RGB_BLACK), (char *)  set_t_c);
    }
    else
    {
      
      u8g2.firstPage();
      do {        
        u8g2.drawXBM(25, 30,  16, 16,hanz[10]);
        u8g2.drawXBM(25+16, 30,  16, 16,hanz[11]);
        u8g2.drawXBM(25+16*2, 30,  16, 16,hanz[12]);
        u8g2.drawXBM(25+16*3, 30,  16, 16,hanz[13]);
        u8g2.drawXBM(25+16*4, 30,  16, 16,hanz[14]);
       } while ( u8g2.nextPage() );
       
      vga.waitVSync(1);  // 16ms
      vga.printStr(xCenter, 8, vga.getColorCode(RGB_RED, RGB_BLACK), (char *)    "                        ");
      vga.printStr(xCenter, 10, vga.getColorCode(RGB_MAGENTA, RGB_BLACK), (char *) "                     ");
      vga.printStr(xCenter, 12, vga.getColorCode(RGB_BLUE, RGB_BLACK), (char *)  "                        ");
      vga.printStr(xCenter, 14, vga.getColorCode(RGB_RED, RGB_BLACK), (char *)  "  Sensor Error");
      vga.printStr(xCenter, 16, vga.getColorCode(RGB_RED, RGB_BLACK), (char *)  "Please Check DHT11");
      vga.printStr(xCenter, 18, vga.getColorCode(RGB_YELLOW, RGB_BLACK), (char *) "                       ");
      vga.printStr(xCenter, 20, vga.getColorCode(RGB_WHITE, RGB_BLACK), (char *)  "                        ");
      vga.printStr(xCenter, 22, vga.getColorCode(RGB_BLUE, RGB_BLACK), (char *)  "                          ");
      vga.printStr(xCenter, 24, vga.getColorCode(1, RGB_BLACK), (char *)  "                                 ");
    }
    for(int i=0;i<50;i++)
      DHT11_data[i]=0;
    vga.waitVSync(110);
    index1=0;
}

//  uint32_t m=0,m1=0,m2=0;
void Request()      /* Microcontroller send request */
{
    pinMode(DHT11, OUTPUT);
    digitalWrite(DHT11,HIGH); 
    uint16_t PulseLOW = 18000; // should > 18ms
    uint16_t PulseHIGH = 30; // not important
    pinMode(DHT11, PWM);
    Timer2.pause(); // stop the timers before configuring them
    timer_oc_set_mode(TIMER2, 1, TIMER_OC_MODE_PWM_2, TIMER_OC_PE);//set PWM Mode2,and enable
    Timer2.setPrescaleFactor(72); // 1 microsecond resolution
    Timer2.setOverflow(PulseLOW + PulseHIGH-1);
    Timer2.setCompare(TIMER_CH1, PulseLOW);
   // Triggered after PulseLOW
    Timer2.attachInterrupt(TIMER_CH1,Response);
    // counter setup in one pulse mode
    //TIMER2_BASE->CR1 = TIMER_CR1_CEN;
    TIMER2_BASE->CR1 = TIMER_CR1_OPM; // one pulse mode
    TIMER2_BASE->CCER =  TIMER_CCER_CC1E ; // enable channels 1 
    Timer2.refresh(); // start timer 2
    Timer2.resume(); // let timer 2 run

/*
                    Using PWM Mode 2

  _______20MS__________-------------------18MS-------------------___________
  ^                    ^                                         ^
  |<----PulseLOW------>|<-- PulseHIGH, but don't need attention->|      
                       ^
                       |
                       
              Compare1Interrupt
              execution Response(),
           Change the mode of the Timer2

 */

}
 

void Response()
{

      pinMode(DHT11, INPUT_PULLUP);
      Timer2.pause();
      Timer2.attachInterrupt(TIMER_CH1,handler_channel_1);
      TIMER2_BASE->CCR1=0;
      TIMER2_BASE->CR1 = TIMER_CR1_CEN;
      TIMER2_BASE->CR2 = 0;
      TIMER2_BASE->SMCR = 0;
      TIMER2_BASE->DIER = TIMER_DIER_CC1IE;
      TIMER2_BASE->EGR = 0;
      TIMER2_BASE->CCMR1 = 0b100000001;
      TIMER2_BASE->CCMR2 = 0;
      TIMER2_BASE->CCER = TIMER_CCER_CC1E;
      TIMER2_BASE->PSC = 71;
      TIMER2_BASE->ARR = 0xFFFF;
      TIMER2_BASE->DCR = 0;
}

void handler_channel_1(void) {                           //This function is called when channel 1 is captured.
    if (0b1 & GPIOA_BASE->IDR  >> 0) {                     //If the receiver channel 1 input pulse on A0 is high.
      channel_1_start = TIMER2_BASE->CCR1;                 //Record the start time of the pulse.
      TIMER2_BASE->CCER |= TIMER_CCER_CC1P;                //Change the input capture mode to the falling edge of the pulse.
    }
    else {                                                 //If the receiver channel 1 input pulse on A0 is low.
      channel_1 = TIMER2_BASE->CCR1 - channel_1_start;     //Calculate the total pulse time.
      if (channel_1 < 0)channel_1 += 0xFFFF;               //If the timer has rolled over a correction is needed.
      TIMER2_BASE->CCER &= ~TIMER_CCER_CC1P;               //Change the input capture mode to the rising edge of the pulse.
      DHT11_data[index1++]=channel_1;
      if(index1==0)
        Timer2.detachInterrupt(TIMER_CH1);
    }
}

 
void Receive_data()    /* Receive data */
{
    int q=0;  
    I_RH=0;
    D_RH=0;
    I_Temp=0;
    D_Temp=0;
    CheckSum=0;
    for (q=1; q<9; q++)
    {
      if(DHT11_data[q]>30)  /* If high pulse is greater than 30ms */
      I_RH = (I_RH<<1)|(0x01);/* Then its logic HIGH */
      else    /* otherwise its logic LOW */
      I_RH = (I_RH<<1);
    }     
    for (q=9; q<17; q++)
    {
      if(DHT11_data[q]>30)  /* If high pulse is greater than 30ms */
      D_RH = (D_RH<<1)|(0x01);/* Then its logic HIGH */
      else    /* otherwise its logic LOW */
      D_RH = (D_RH<<1);
    }   
    for (q=17; q<25; q++)
    {
      if(DHT11_data[q]>30)  /* If high pulse is greater than 30ms */
      I_Temp = (I_Temp<<1)|(0x01);/* Then its logic HIGH */
      else    /* otherwise its logic LOW */
      I_Temp = (I_Temp<<1);
    }
    for (q=25; q<33; q++)
    {
      if(DHT11_data[q]>30)  /* If high pulse is greater than 30ms */
      D_Temp = (D_Temp<<1)|(0x01);/* Then its logic HIGH */
      else    /* otherwise its logic LOW */
      D_Temp = (D_Temp<<1);
    }  
    for (q=33; q<41; q++)
    {
      if(DHT11_data[q]>30)  /* If high pulse is greater than 30ms */
      CheckSum = (CheckSum<<1)|(0x01);/* Then its logic HIGH */
      else    /* otherwise its logic LOW */
      CheckSum = (CheckSum<<1);
    }                                                                                                                                                                        
}
