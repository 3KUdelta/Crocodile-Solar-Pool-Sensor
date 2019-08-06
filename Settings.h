
/*----------------------------------------------------------------------------------------------------
  Project Name : Crocodile Solar Pool Sensor
  Features: Temperature and Voltage via Blynk and MQTT
  Author: Marc Stähli (3KU_Delta)
  Website : https://github.com/3KUdelta

  GNU General Public License shall apply: http://www.gnu.org/licenses/gpl-3.0
  
  Main microcontroller (ESP8266, LOLIN (WEMOS) D1 mini pro) and 18d20 temperature sensor both sleep between measurements

  CODE: https://github.com/3KUdelta
  INSTRUCTIONS & HARDWARE: 
  3D FILES: Included in the instructables page

*/

char auth[] = "YOUR-BLYNK-AUTH-TOKEN"; // Blynk Auth Token

char ssid[] = "YOUR-ROUTER-SSID";                           // WiFi Router ssid
char pass[] = "YOUR-ROUTER-PASSWORD";             // WiFi Router password

const boolean MQTT = true;                        // Set this to false if you are not using MQTT

const char* mqtt_server = "192.168.188.87";       // MQTT Server address, leave blank if your are not using MQTT


/****** Additional Settings **************************************************/

const boolean is_metric = true;                   // Set this to true for Celcius, false for Fahrenheit

#define sleepTimeMin (15)                         // setting of deepsleep time in minutes (default: 15)


/****** Timestamp Settings **************************************************/

#include <Timezone.h>           // http://github.com/JChristensen/Timezone  JUST IGNORE COMPILE WARNING!

// Settings for Switzerland
TimeChangeRule CEST = { "CEST", Last, Sun, Mar, 2, +120 };      // Central European Summer Time = UTC/GMT +2 hours
TimeChangeRule CET = { "CET", Last, Sun, Oct, 2, +60 };         // Central European Time = UTC/GMT +1 hour
Timezone MyTZ(CEST, CET);

// Continental US Time Zones (EXAMPLES - pick yours)

// TimeChangeRule EDT = { "EDT", Second, Sun, Mar, 2, -240 };    //Daylight time = UTC - 4 hours
// TimeChangeRule EST = { "EST", First, Sun, Nov, 2, -300 };     //Standard time = UTC - 5 hours
// Timezone MyTZ(EDT, EST);

// TimeChangeRule CDT = { "CDT", Second, Sun, Mar, 2, -300 };    //Daylight time = UTC - 5 hours
// TimeChangeRule CST = { "CST", First, Sun, Nov, 2, -360 };     //Standard time = UTC - 6 hours
// Timezone MyTZ(CDT, CST);

// TimeChangeRule MDT = { "MDT", Second, Sun, Mar, 2, -360 };    //Daylight time = UTC - 6 hours
// TimeChangeRule MST = { "MST", First, Sun, Nov, 2, -420 };     //Standard time = UTC - 7 hours
// Timezone MyTZ(MDT, MST);

// TimeChangeRule PDT = { "PDT", Second, Sun, Mar, 2, -420 };    //Daylight time = UTC - 7 hours
// TimeChangeRule PST = { "PST", First, Sun, Nov, 2, -480 };     //Standard time = UTC - 8 hours
// Timezone MyTZ(PDT, PST);

// change for different ntp (time server)
#define NTP_SERVER "ch.pool.ntp.org"

// Works also with your local servers, such as "us.pool.ntp.org", "time.nist.gov", "pool.ntp.org"


/****** Data transmission overview *******************************************
 *  
 *  Blynk:
 *  
 *  Pool Temperature      = virtual pin 11
 *  Battery Voltage       = virtual pin 12
 *  Last Update Timestamp = virtual pin 13
 *  
 *  MQTT:
 *  
 *  Debug messages to: home/debug
 *  
 *  Sensor data to: home/pool/solarcroc/tempc
 *                  home/pool/solarcroc/battv
 *  
 *  
 ******************************************************************************/
