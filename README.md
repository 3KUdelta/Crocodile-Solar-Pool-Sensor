# Crocodile-Solar-Pool-Sensor
A rather special looking pool sensor measuring the pool temperature and transmitting it via WiFi to Blynk App and to a MQTT broker
https://www.instructables.com/id/Crocodile-Solar-Pool-Sensor/

Version 1.0
- Fully independent from power sources (solar panel feeds the LiPo battery)
- Low power ESP8266 WiFi connected sensor
- Rather high precision temperature sensor (DS 18b20)
- Data transmission of temp and voltage to Blynk APP for your mobile phone
- Data transmission of temp and voltage to a MQTT broker (can be switched on/off in Settings.h)

Version 1.1
- Corrected error handling if there is no WiFi

Version 1.2
- added switch to change between Celsius and Fahrenheit (Settings.h --> is_metric true or false)

Version 1.3
- added NTP time fetch, DST conversion and data transmission of last update timestamp to Blynk APP

Version 1.4
- added battery saving when batt level goes below 3.3 V

[![Crocodile Solar Pool Sensor](https://github.com/3KUdelta/Crocodile-Solar-Pool-Sensor/blob/master/Pool_Croc.jpg)](https://github.com/3KUdelta/Crocodile-Solar-Pool-Sensor)

[![Crocodile Solar Pool Sensor](https://github.com/3KUdelta/Crocodile-Solar-Pool-Sensor/blob/master/Pool_Sensor.jpg)](https://github.com/3KUdelta/Crocodile-Solar-Pool-Sensor)

Please see also how to display the data on a LED matrix:
[![LED matrix MQTT monitor](https://github.com/3KUdelta/MDparola_MQTT_monitor/blob/master/pictures/IMG_3180.JPG)](https://github.com/3KUdelta/MDparola_MQTT_monitor)
https://github.com/3KUdelta/MDparola_MQTT_monitor
