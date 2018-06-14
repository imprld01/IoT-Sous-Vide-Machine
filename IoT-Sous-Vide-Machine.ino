#include "OneWire.h"
#include "DallasTemperature.h"
#include <LiquidCrystal.h>

//for temperature sensor
////////////////////////////////////
const int  one_wire_bus = 2;  //pin 2
const int relay1 =  3; //pin 3
OneWire oneWire(one_wire_bus);
DallasTemperature sensors(&oneWire);
////////////////////////////////////


// select the pins used on the LCD panel
//////////////////////////////////////////////////////
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
// define some values used by the panel and buttons
int lcd_key     = 0;
int adc_key_in  = 0;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5
/////////////////////////////////////////////////////

// read the buttons
///////////////////////////////////////////////////////////////////////////////
int read_LCD_buttons()
{
 adc_key_in = analogRead(0);      // read the value from the sensor 
 // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
 // we add approx 50 to those values and check to see if we are close
 if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
 // For V1.1 us this threshold
 /*if (adc_key_in < 50)   return btnRIGHT;  
 if (adc_key_in < 250)  return btnUP; 
 if (adc_key_in < 450)  return btnDOWN; 
 if (adc_key_in < 650)  return btnLEFT; 
 if (adc_key_in < 850)  return btnSELECT;  
*/
 // For V1.0 comment the other threshold and use the one below:
  
   if (adc_key_in < 50)   return btnRIGHT;  
   if (adc_key_in < 195)  return btnUP; 
   if (adc_key_in < 380)  return btnDOWN; 
   if (adc_key_in < 555)  return btnLEFT; 
   if (adc_key_in < 790)  return btnSELECT;   
  
 return btnNONE;  // when all others fail, return this...
}
////////////////////////////////////////////////////////////////////////////////

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(relay1, OUTPUT);
  sensors.begin();               // temperature sensor
  lcd.begin(16, 2);              // start the lcd library
  lcd.setCursor(0,0);
  lcd.print("SousVide Machine"); // print a simple message
 
  
} 
// control the machine
/////////////////////////////
int temperature = 60;
int cookTime = 1;
int mode = 0; 
/*
 * 0: active machine  
 * mode>0:setting mode; 
 * mode 1 : setting temperature (C)
 * mode 2 : setting count time (min)
 * mode 3 : setting isStart
 */
float startTime = 0;
bool isOn = false;
bool isCooking = false;
float currentTime = 0;
float bounceTime = 0;
float bounceDelay = 300;
void SVMachine(int lcd_key){
  switch (lcd_key)               // depending on which button was pushed, we perform an action
 {
   case btnRIGHT:
     {
       //debounce
        if(currentTime - bounceTime <= bounceDelay) break;
        bounceTime = currentTime;
        /////
       if(mode>0 && mode <3){
        mode++;
       }else if(mode>0){
        mode = 1;
       }
       break;
     }
   case btnLEFT:
     {
         //debounce
        if(currentTime - bounceTime <= bounceDelay) break;
        bounceTime = currentTime;
        /////
       if(mode>0 && mode >1){
        mode--;
       }else if(mode == 1){
        mode = 3;
       }
       break;
     }
   case btnUP:
     { 
       //debounce
        if(currentTime - bounceTime <= bounceDelay-100) break;
        bounceTime = currentTime;
        /////
       if(mode>0){
        if(mode == 1 && temperature <999){
          temperature++;
        }else if(mode == 2 && cookTime <999){
          cookTime++;
        }else if(mode == 3){
          isOn = !isOn;
        }
       }
       break;
     }
   case btnDOWN:
     {
       //debounce
        if(currentTime - bounceTime <= bounceDelay-100) break;
        bounceTime = currentTime;
        /////
       if(mode>0){
        if(mode == 1 && temperature >0){
          temperature--;
        }else if(mode == 2 && cookTime >0){
          cookTime--;
        }else if(mode == 3){
           isOn = !isOn;
        }
       }
       break;
     }
   case btnSELECT:
     {
      //debounce
      if(currentTime - bounceTime <= bounceDelay) break;
      bounceTime = currentTime;
      /////
      if(mode > 0){
        mode = 0;
        lcd.setCursor(0,0);
        lcd.print("SousVide Machine"); // print a simple message
        if(!isCooking && isOn) {
          startTime = currentTime;
          isCooking = true;
        }else if(isOn){
          isCooking = true;
        }else if(!isOn){
          isCooking = false;
        }
      }else{
        mode = 1;
        lcd.setCursor(0,0);
        lcd.print("SousVide Setting"); // print a simple message
      }
       break;
     }
   case btnNONE:
     {
       break;
     }
  }
}
bool isBlink = false;
float blinkTime = 0;
int minute = 0;
int second = 0;
void loop() {
  currentTime = millis();
  // put your main code here, to run repeatedly:
  //int buttonState = digitalRead(button);
  sensors.requestTemperatures(); 
  float TEMP1 = float(sensors.getTempCByIndex(0));
  Serial.println(TEMP1);
  if(isCooking){
    if(TEMP1 > temperature){
      digitalWrite(relay1, LOW); // LOW enable
    }else{
      digitalWrite(relay1, HIGH); // LOW enable
    }
    minute = (int)(cookTime - (currentTime - startTime) /1000 / 60);
    second = (int)(cookTime * 60 - (currentTime - startTime) / 1000) % 60;
    if(minute <=0 &&   second<=0) isCooking = false;
  }else{
      digitalWrite(relay1, LOW); // LOW enable
  }
 // LCD code
 ////////////////////////////////////////////////////////////////////////////////////
 
 if(mode > 0){
   lcd.setCursor(0,1);
   lcd.print("                ");
   lcd.setCursor(0,1);            // move cursor to second line "1" and 9 spaces over
   lcd.print(temperature);
   lcd.print("C  ");
   if(mode == 1 && isBlink){
      lcd.setCursor(0,1);            // move cursor to second line "1" and 9 spaces over
      lcd.print("      ");
    }
   lcd.setCursor(6,1);
   lcd.print(cookTime);
   lcd.print("M  ");
   if(mode == 2 && isBlink){
      lcd.setCursor(6,1);            // move cursor to second line "1" and 9 spaces over
      lcd.print("      ");
    }
   lcd.setCursor(13,1);
   lcd.print((isOn)?" ON":"OFF");
   if(mode == 3 && isBlink){
      lcd.setCursor(13,1);            // move cursor to second line "1" and 9 spaces over
      lcd.print("   ");
    }
 }else{
       if(isOn){
           lcd.setCursor(0,1);            // move cursor to second line "1" and 9 spaces over
           lcd.print(temperature);
           lcd.print("C  ");
           lcd.setCursor(5,1);
           if(TEMP1 < 0) {
                lcd.print("Error");
           }else {
                lcd.print(" ");
                lcd.print((int)TEMP1);
                lcd.print("C  ");
           }
           lcd.setCursor(11,1);
           
           if(!isCooking){
              lcd.setCursor(10,1);
              lcd.print("Finish");
              isCooking = false;
           }else{
             if((minute)/10==0){
                  lcd.print("0");
              }
                  lcd.print(minute);
                  lcd.print(":");
             
             if (second < 10){
                  lcd.print("0");
             }
                  lcd.print(second);
                  lcd.print(" ");
           }
       }else{
           lcd.setCursor(0,1);
           lcd.print("Press Select ...");
       }
 }


 lcd_key = read_LCD_buttons();  // read the buttons
  SVMachine(lcd_key);
  if(currentTime - blinkTime > 300){
    blinkTime = currentTime;
    isBlink = !isBlink;
  }

 //////////////////////////////////////////////////////////////////////////////
}
