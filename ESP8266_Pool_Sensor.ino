
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
  
  Needed libraries:

  <OneWire.h>                   // for temperature sensor 18d20 (Arduino Libraries)
  <DallasTemperature.h>         // for temperature sensor 18d20 (Arduion Libraries)
  <ESP8266WiFi.h>               // For WiFi connection with ESP8266 --> MQTT
  <BlynkSimpleEsp8266.h>        // https://github.com/blynkkk/blynk-library
  <PubSubClient.h>              // For MQTT (in this case publishing only)
  <WiFiUdp.h>                   // For NTP Signal fetch
  <EasyNTPClient.h>             // For NTP Signal read https://github.com/aharshac/EasyNTPClient
  <TimeLib.h>                   // For converting NTP time https://github.com/PaulStoffregen/Time
  <Timezone.h>                  // http://github.com/JChristensen/Timezone  JUST IGNORE COMPILE WARNING!

  Hardware Settings Mac: 
  LOLIN(WEMOS) D1 mini Pro, 80 MHz, Flash, 16M (14M SPIFFS), v2 Lower Memory, Disable, None, Only Sketch, 921600 on /dev/cu.SLAB_USBtoUART

  V1.0: initial compile and commit: 07/07/19
  V1.1: improved error handling if no WiFi connection possible --> take a nap for 1 min and retry instead of a reset
  V1.2: added switch for Fahrenheit in Settings.h
  V1.3: added NTP time fetch including DST conversion and publish last update timestamp to BLYNK app
  V1.4: added battery saving mode when battery goes below 3.3 V

////  Features :  //////////////////////////////////////////////////////////////////////////////////////////////////////
                                                                                                                         
// 1. Connect to Wi-Fi, and upload the data to either Blynk App and/or MQTT broker

// 2. Remote water temperature and voltage status (because of solar charging)

// 3. Using Sleep mode to reduce the energy consumed                                        

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************
 * VERY IMPORTANT:                                 *
 *                                                 *
 * Enter first your settings in Settings.h !       *
 *                                                 *
 **************************************************/

#include "Settings.h"
#include <OneWire.h>                         // for temperature sensor 18d20
#include <DallasTemperature.h>               // for temperature sensor 18d20
#include <ESP8266WiFi.h>                     // For WiFi connection with ESP8266 --> MQTT
#include <BlynkSimpleEsp8266.h>              // https://github.com/blynkkk/blynk-library
#include <PubSubClient.h>                    // For MQTT (in this case publishing only)
#include <WiFiUdp.h>                         // For NTP Signal fetch
#include <EasyNTPClient.h>                   // For NTP Signal read https://github.com/aharshac/EasyNTPClient
#include <TimeLib.h>                         // For converting NTP time https://github.com/PaulStoffregen/Time.git

#define ONE_WIRE_BUS 2                       // Data wire 18d20 Sensor is plugged into port 2 @ ESP8266

OneWire oneWire(ONE_WIRE_BUS);               // Setup a oneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire);         // Pass our oneWire reference to Dallas Temperature. 

WiFiUDP udp;                                 // Initializing UPD for NTP read
EasyNTPClient ntpClient(udp, NTP_SERVER, 0); // Initializing NTP call

WiFiClient espClient;                        // Initializing WiFiClient for MQTT
PubSubClient client(espClient);              // Initializing MQTT

//**** Variables ****
const int numReadings = 32;                  // the higher the value, the smoother the average (default: 32)
float readings[numReadings];                 // the readings from the analog input
float total = 0;                             // the running total
float PoolTemp;
char actualtime[16];
bool battstate = 1;

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("Starting Pool Monitor");

  go_online();                               // open WiFi and go online
  get_NTP_time();                            // get time from NTP server

  if (MQTT) {
    randomSeed(micros());                    // used for random client id

    connect_to_MQTT();                       // connect to MQTT broker --> do not forget Settings.h

    client.publish("home/debug", "PoolMonitor: Sensor started");  // publishing debug messages via MQTT
    delay(50);
    client.publish("home/debug", "PoolMonitor: Requesting temperature");
    delay(50);
  }
  
  Serial.print("Requesting temperature...");
  
  sensors.begin();
  sensors.requestTemperatures();      // Send the command to get temperatures
  int i = 0;
  while (getTemperature() != true) {
    delay(500);
    i++;
    if (i > 20) {
      Serial.println("Could not get a correct temperature reading!");
      Serial.println("Doing a break for 1 Minute and retry a connection from scratch.");
      goToSleep(1);   // go to sleep and retry after 1 min
    }      
    Serial.println("Doing another measurement due to wrong reading");
  }
  Serial.println("Temperature reading ok");

  if (MQTT) {
    client.publish("home/debug", "PoolMonitor: Measuring voltage of battery");
    delay(50);
  }

  float adcVoltage = analogRead(A0) / 1023.0;    
  float batteryVoltage = adcVoltage * 5.31;    // Voltage divider R1 = 220k+100k+220k =540k and R2=100k

  Serial.print("Measured battery voltage: ");
  Serial.println(batteryVoltage);

  if (batteryVoltage < 3.3) battstate = 0;     // Battery low, needs do be recharged

  /********** writing data to MQTT ********************************/
  if (MQTT) {
    /* handling pool temp */
    char _PoolTemp[8];                                // Buffer big enough for 7-character float
    dtostrf(PoolTemp, 3, 1, _PoolTemp);               // Leave room for too large numbers!

    client.publish("home/pool/solarcroc/tempc", _PoolTemp, 1);         // ,1 = retained
    delay(50);
    client.publish("home/debug", "PoolMonitor: Just published pool temp to home/pool/solarcroc/tempc");  
    delay(50); 

    /*handling battery voltage */
    char _batteryVoltage[8];                          // Buffer big enough for 7-character float
    dtostrf(batteryVoltage, 3, 2, _batteryVoltage);   // Leave room for too large numbers!
  
    client.publish("home/pool/solarcroc/battv", _batteryVoltage, 1);   // ,1 = retained
    delay(50);
    client.publish("home/debug", "PoolMonitor: Just published batt voltage to home/pool/solarcroc/battv");
    delay(50);

    if (battstate == 0) {
      /*sending BATT LOW to MQTT*/
      client.publish("home/debug", "PoolMonitor: RUNNING LOW ON BATTERY!");
      delay(50);
    }
  }
  
  /*********** preparing timestamp for BLYNK **********************/
  
  sprintf(actualtime, "%02u:%02u", hour(now()), minute(now()));
  Serial.print("Update time sent to Blynk: ");
  Serial.println(actualtime);
  
  /********** writing data to Blynk *******************************/
  
  Blynk.virtualWrite(11, PoolTemp);           // virtual pin 11
  Blynk.virtualWrite(12, batteryVoltage);     // virtual pin 12
  Blynk.virtualWrite(13, actualtime);         // virtual pin 13
  
  Serial.println("Writing to Blynk completed ...");

  if (battstate == 1) {
    goToSleep(sleepTimeMin);  // normal sleeping time (as set in Settings.h)
  }
  else {
    goToSleep(360);           // Batt low, sleeping for 6 hours
  }
  
}

void loop(void)
{ 
                                     // we do not loop anything
}

void go_online() {
  WiFi.hostname("PoolMonitor");     // This changes the hostname of the ESP8266 to display neatly on the network esp on router.
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Serial.print("---> Connecting to WiFi ");
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    i++;
    if (i > 50) {
      Serial.println("Could not connect to WiFi!");
      Serial.println("Doing a break for 1 Minute and retry a connection from scratch.");
      goToSleep(1);   // go to sleep and retry after 1 min
    }  
    Serial.print(".");
  }
  Serial.print("Wifi connected ok. ");
  Serial.println(WiFi.localIP());
  Blynk.begin(auth, ssid, pass);    // connecting to Blynk Server
  Serial.println("Blynk connected ok.");

} //end go_online

void get_NTP_time() {
  time_t utc;
  
  Serial.println("---> Now reading time from NTP Server");

  while (!ntpClient.getUnixTime()) {
    delay(100);
    Serial.print(".");
  }
  setTime(ntpClient.getUnixTime());           // get UNIX timestamp (seconds from 1.1.1970 on)

  utc = now();
  setTime(MyTZ.toLocal(utc));                 // convert into your timezone (defined in Settings.h) and store it in now()

  Serial.print("Actual UNIX timestamp: ");
  Serial.println(now());

  Serial.print("Time & Date: ");
  Serial.print(hour(now()));
  Serial.print(":");
  Serial.print(minute(now()));
  Serial.print(":");
  Serial.print(second(now()));
  Serial.print("; ");
  Serial.print(day(now()));
  Serial.print(".");
  Serial.print(month(now()));
  Serial.print(".");
  Serial.println(year(now()));
} // end get_NTP_time()

void connect_to_MQTT() {
  Serial.print("---> Connecting to MQTT, ");
  client.setServer(mqtt_server, 1883);
  
  while (!client.connected()) {
    Serial.println("reconnecting MQTT...");
    reconnect(); 
  }
  Serial.println("MQTT connected ok.");
} //end connect_to_MQTT

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection with ");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    Serial.print(clientId.c_str());
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println(" - connected");
       // Once connected, publish an announcement...
      client.publish("home/debug", "PoolMonitor: client started...");
      delay(50);
    } else {
      Serial.print(" ...failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
} //end void reconnect*/

bool getTemperature() {
  for (int thisReading = 0; thisReading < numReadings; thisReading++) // take # of readings for smoothening
  {
    readings[thisReading] = sensors.getTempCByIndex(0);
    total = total + readings[thisReading];
  }
  PoolTemp = total / numReadings;

  if (!is_metric) PoolTemp = PoolTemp * 1.8 + 32;
  
  Serial.print("Measured pool temperature: ");
  Serial.println(PoolTemp);

  if (PoolTemp > -127 && PoolTemp < 85) {                            // error handling for bogus readings
    return true;
  }
  else {
    return false;
  }
}

void goToSleep(unsigned int sleepmin) {
  if (MQTT) {
    char tmp[128];
    String sleepmessage = "PoolMonitor: Taking an nap for " + String (sleepmin) + " Minutes";
    sleepmessage.toCharArray(tmp, 128);
    client.publish("home/debug",tmp);
    delay(50);
 
    Serial.println("INFO: Closing the MQTT connection");
    client.disconnect();
  
    Serial.println("INFO: Closing the Wifi connection");
    WiFi.disconnect();

    while (client.connected() || (WiFi.status() == WL_CONNECTED)) {
      Serial.println("Waiting for shutdown before sleeping");
      delay(10);
    }
    delay(50);
  }
  Serial.print ("Going to sleep now for ");
  Serial.print (sleepmin);
  Serial.print (" Minute(s).");
  ESP.deepSleep(sleepmin * 60 * 1000000); // convert to microseconds
} // end of goToSleep()
