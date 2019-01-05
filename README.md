# FilamentDryer_Controller
Arduino OLED Rlelay and DS18B20 to control heater element of a 3D printing Filament dryer.


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
  * Library Used: (V1.1)
 * - OneWire.h from Paul Stoffregen...............https://github.com/PaulStoffregen/OneWire
 * - Addafruit GFX:...............................https://github.com/adafruit/Adafruit-GFX-Library
 * - Addafruit SSD1306............................https://github.com/adafruit/Adafruit_SSD1306
 * - Arduino Temperature Control by milesburton...https://github.com/milesburton/Arduino-Temperature-Control-Library 
 * 
 * Created by:
 * Olivier Royer-Tardif (alias MirageC)
 * October 12th, 2018
 * 
 * Revision Log:
 * 
 * Version    DATE                DESC                    RELEASED BY:
 * V1       12-Oct-2018       Initial release         Olivier Royer-Tardif
 * V1.1     05-Jan-2019       Library Used listed     Olivier Royer-Tardif
 
 * Known Bugs
 * 1 - Memory issue if any additionnal variables added.
 * 2 - "Set Timer" menu not functionnal
 * 
 */
