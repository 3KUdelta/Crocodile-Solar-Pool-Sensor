
/*----------------------------------------------------------------------------------------------------
  Project Name : Crocodile Solar Pool Sensor
  Features: Temperature and Voltage via Blynk and MQTT
  Author: Marc Stähli (3KU_Delta)
  Website : https://github.com/3KUdelta

  GNU General Public License shall apply: http://www.gnu.org/licenses/gpl-3.0
  
  Main microcontroller (ESP8266, LOLIN (WEMOS) D1 mini pro) and 18d20 temperature sensor both sleep between measurements

  CODE: https://github.com/3KUdelta
  INSTRUCTIONS & HARDWARE: https://www.instructables.com/id/Crocodile-Solar-Pool-Sensor/
  3D FILES: Included in the instructables page

*/

char auth[] = "your_blynk_auth_token"; // Blynk Auth Token

char ssid[] = "your_ssid";                        // WiFi Router ssid
char pass[] = "your_wifi_pw";                     // WiFi Router password

const boolean MQTT = true;                        // Set this to false if you are not using MQTT

const char* mqtt_server = "192.xxx.xxx.xxx";      // MQTT Server address, leave blank if your are not using MQTT

/****** Additional Settings **************************************************/

#define sleepTimeMin (15)           // setting of deepsleep time in minutes (default: 15)


/****** Data transmission overview *******************************************
 *  
 *  Blynk:
 *  
 *  Pool Temperature = virtual pin 11
 *  Battery Voltage  = virtual pin 12
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
