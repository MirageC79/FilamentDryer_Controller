/*
 * 3D Printing Filament Dehydrator Control Module
 * 
 * This sketch will control the heater element(s) of a standard Food Dehydrator 
 * to enable drying various 3d printing filament to their suggested dryinh tempertaure and time.
 * 
 * An Arduino is used to display and capture menu selections through 3 momentary push buttons.  
 * Pre stored values for targetted temperature will be compared to the inputs from a DS18b20 Dallas digital temperature sensor.
 * Results of this comparision will then be used to control the heater element relay via 5v signal.
 * 
 * ####-WARNING-#### Timer function is ONLY for reference, NO ACTION to the dehydrator will be executed by the module through timer functions.
 * The reason for this non-action is to prevent the filament to re-absorb humidity in case of long running time after auto-shutdown of heater element.
 * Buzzer / Alarm might be added in the future
 * 
 * BOM (Bill of Material):
 * Qty - Desc
 * 1 - Arduino Pro-Mini or other
 * 1 - OLED Display
 * 1 - DS18B20 Dalla temp sensor
 * 1 - Single Relay Module
 * 1 - 120/220VAC to 5DC converter (Any phone charger or other adapter
 * 3 - Momentary on Push Buttons
 * 3 - 10K Resistor
 * 1 - 4.7K Resistor
 * 
 * Wiring Diagram..........:
 * 3D printed enclosure....:
 * This code GitHub........: https://github.com/MirageC79/FilamentDryer_Controller
 * 
 * Created by:
 * Olivier Royer-Tardif (alias MirageC)
 * October 12th, 2018
 * 
 * Revision Log:
 * 
 * Version    DATE                DESC                    RELEASED BY:
 * V1       12-Oct-2018       Initial release         Olivier Royer-Tardif
 * 
 * Known Bugs
 * 1 - Memory issue if any additionnal variables added.
 * 2 - "Set Timer" menu not functionnal
 * 
 */


/* Library References:
- https://github.com/adafruit/Adafruit-GFX-Library
- https://github.com/adafruit/Adafruit_SSD1306
*/

/*----------------USER DEFINED-----------------------*/
#define ONE_WIRE_BUS 5                    
#define CONTROL_RELAY 3
/*---------------------------------------------------*/

#include "FilamentDryerMenu.h"
#include <OneWire.h>
#include <DallasTemperature.h>
OneWire oneWire(ONE_WIRE_BUS);                    // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)  
DallasTemperature sensors(&oneWire);              // Pass our oneWire reference to Dallas Temperature.

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);
#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2
#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16

static const unsigned char PROGMEM logo16_glcd_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000
};

#if (SSD1306_LCDHEIGHT != 32)                       // Check if the right LCD parameter being used in function of the OLED unit connected
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

/*-----------------------GLOBAL VARIABLES-----------------------------*/
byte setTemp = 0;                                   // Store target temperature from menu table 
unsigned int setTime = 0;                           // Store drying time from menu table
char setKeyw[5];                                    // Store 4 letter material keyword from menu table
bool heaterStat = 1;                                // Heating request **Status is inverted: 1=off 0=on due to pullup resistors**
bool dotHeat = 1;                                   // Flashing dot while heating
byte setMode = 0;                                   // 1: Thermostat display, 0 =  Main Menu Type, 2 = Set timer, 
unsigned long lastmillis = 0;                       // Last time flashing dot changed to on/off
unsigned long lastmillisSensor = 0;                 // Last time temp sensor was querried
byte pageSelect;                                   
byte BTN[] = {7, 8, 9};
byte BtnStat[] = {HIGH, HIGH, HIGH};
byte previous[] = {HIGH, HIGH, HIGH};               // Last button status
unsigned long millis_held[] = {0, 0, 0};            // How long the button was held (milliseconds)
unsigned long firstTime[] = {0, 0, 0};              // Time Stamp when button is first pressed
unsigned long millisStart = 0;
byte selectedPos = 0;
byte previousPos = 0;
MenuItems *mAddress;
float sensorTemp;




/************************************************************SETUP************************************************************************/
void setup()   {
  //Serial.begin(115200);

  //sensors.begin();                                  // initiate sensors library


  pinMode(3, OUTPUT);
  for (int i = 0; i < 3; i++) {
    pinMode(BTN[i], INPUT);
  }

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  // init done
  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(500);

  setMode = 0;                  // open menu on powerup
}

/**********************************************************MENU SELECTION******************************************************************/
int MenuSelection() {

  byte tmp_setMode;
  byte tmp_setTemp;
  unsigned int tmp_setTime;
  char tmp_setKeyw[5];

  MenuItems Menu1[] = {
    //    DESC           ShortDESC sMode  T  t
    {"- Exit",            "",     1,   0, 0},
    {"- Select Material", "",     2,   0, 0},
    {"- Set timer",       "",     1,   0, 0}                    // Function to be implemented in the future
  };

  MenuItems Menu2[] = {
    //    DESC           ShortDESC sMode  T  t
    {"<-Return",          "",      0,  0,   0},
    {"- PLA......45C 4h",  "PLA",  1, 45, 240},
    {"- ABS......60C 2h",  "ABS",  1, 60, 120},
    {"- PETG.....65C 2h",  "PETG", 1, 65, 120},
    {"- NYLON... 70C 12h", "NYLN", 1, 70, 720},
    {"- PVA......45C 4h",  "PVA",  1, 45, 240},
    {"- TPU/TPE..50C 4h",  "TPE",  1, 50, 240},
    {"- ASA......60C 4h",  "ASA",  1, 60, 240},
    {"- PP.......55C 6h",  "PP",   1, 55, 360},
    {"- Dsscnt...65C 3h",  "Dsct", 1, 65, 180},
    {"- Test.....29C 1m",  "Test", 1, 29,  60}
  };

  MenuItems *m;

  byte nItem1 = sizeof Menu1 / sizeof Menu1[0];
  byte nItem2 = sizeof Menu2 / sizeof Menu2[0];
  byte nItemS;

  switch (setMode) {
    case 0:
      nItemS = nItem1;
      m = Menu1;
      break;
    case 2:
      nItemS = nItem2;
      m = Menu2;
      break;
  }

  if (BtnStat[1] == LOW && selectedPos > 0 ) {
    selectedPos += -1;
  } else if (BtnStat[2] == LOW && selectedPos < nItemS - 1) {
    selectedPos += 1;
  }

  BtnStat[1] = HIGH;
  BtnStat[2] = HIGH;

  ClearBuffer();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  int menupage = (selectedPos / 3) * 3;
  display.print("Main Menu      p ");
  display.print((selectedPos + 3) / 3);
  display.print("/");
  display.println(ceil(nItemS / 3), 0);

  for (int i = menupage; i < selectedPos + 3 && i < nItemS ; i++) {
    mAddress = m + i;
    if (i == selectedPos) {
      display.print("*");
      tmp_setMode = (*mAddress).mode;
      tmp_setTemp = (*mAddress).drytemp;
      tmp_setTime = (*mAddress).drytime;
      strcpy(tmp_setKeyw, (*mAddress).shortdesc);
    } else {
      display.print(" ");
    }
    display.println((*mAddress).desc);
  }
  display.display();
  ClearBuffer();

  if (BtnStat[0] == LOW) {
    setMode = tmp_setMode;
    setTemp = tmp_setTemp;
    setTime = tmp_setTime;
    strcpy(setKeyw,tmp_setKeyw);
    selectedPos = 0;
    millisStart = millis();
    BtnStat[0] = HIGH;
  }
  return 0;
}

/*******************************************************GET BUTTON STATUS****************************************************************/
byte ButtonStatus(byte btn) {
  byte r = HIGH;
  byte statbtn = digitalRead(BTN[btn]);
  
  if (statbtn == LOW && previous[btn] == HIGH && (millis() - firstTime[btn]) > 50) {
    firstTime[btn] = millis();                            // Take time stamp when first pressed. 200 milliseconds acts as debouncing.
  }
  
  millis_held[btn] = (millis() - firstTime[btn]);         // Count how many milliseconds the button has been pressed for.
  
  if (statbtn == HIGH && previous[btn] == LOW && millis_held[btn] >= 50) {
    r = LOW;
  }
  
  previous[btn] = statbtn;
  return r;
}

/********************************************************CLEAR DISPLAY BUFFER***********************************************************/
void ClearBuffer() {
  display.clearDisplay();
  display.setCursor(0, 0);
}

/******************************************************DISPLAY THERMOSTAT FUNCTION******************************************************/
void Thermostat() {
  
  ClearBuffer(); // text display tests
  display.setTextSize(1);
  display.setTextColor(WHITE);

  unsigned long tSec;
  unsigned long dHour;
  int dMin;
  int dSec;

  if ((setTime * 60) <= ((millis() - millisStart) / 1000)) {
      tSec = 0;
  }else{
      tSec = ((setTime * 60) - ((millis() - millisStart) / 1000));
  }
  
  dHour = tSec / 3600;
  dMin = tSec / 60;
  dMin = dMin % 60;
  dSec = tSec % 60;

  if (heaterStat == 1) {
    if (dotHeat == 1) {
      display.print("*");
    } else {
      display.print(" ");
    }
    if (millis() > lastmillis + 500) {
      lastmillis = millis();
      dotHeat = !dotHeat;
    }
    display.print(" HEATING...     ");
  }else{
    display.print(" Standby        ");
  }
  
  display.println(setKeyw);
  display.println("Actual:       Target:");
  display.setTextSize(2);
  display.print(sensors.getTempCByIndex(0), 1);
  display.setTextSize(1);
  display.print("         ");
    if (setTemp < 100) {display.print(" ");}
    if (setTemp < 10) {display.print(" ");}
  display.print(setTemp, 1);
  display.println("C");
  display.print("             ");
    if (dHour < 10) { display.print("0");}
  display.print(dHour);
  display.print(":");
    if ((dMin)< 10){display.print("0");}
  display.print(dMin);
  display.print(":");
    if (dSec < 10){display.print("0");}
  display.print(dSec);
  display.display();
  display.clearDisplay();

  if (BtnStat[0] == LOW) {
    setMode = 0;
    BtnStat[0] = HIGH;
  }
}

/**********************************************************************************************************************************
 *                                                                                                                                *
 *                                                        MAIN LOOP                                                               *
 *                                                                                                                                *
 **********************************************************************************************************************************/
void loop() {

  if (millis() > lastmillisSensor + 500) {
    lastmillisSensor = millis();
    sensors.requestTemperatures();
  }
  
  sensorTemp = sensors.getTempCByIndex(0);

  if ((heaterStat == 0) && (sensorTemp < (setTemp - 2))) {
    digitalWrite(3, HIGH);
    heaterStat = 1;
  } else if ((heaterStat == 1) && (sensorTemp >= setTemp)) {
    digitalWrite(3, LOW);
    heaterStat = 0;
  }

  for (int i = 0; i < 3; i++) {
    BtnStat[i] = ButtonStatus(i);
  }

  switch (setMode) {
    case 0:
      MenuSelection();
      //delay(100);
      break;
    case 1:
      Thermostat();
      break;
    case 2:
      MenuSelection();
      break;
  }
}/***************************************************************THE END***********************************************************/
