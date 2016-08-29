# ESP8266_DHT22_SENSORS-DIFFERENCE_VALUES
Fork project EasyIoT for control sensors in Cloud. Used two DHT22 sensors and calculate difference of values

NOTE:

Used:
 ESP8266-12E:
 - 26Mhz Cristal
 - 32MBit (4KByte) 80Mhz Quad SPI Flash Memory Winbond W25Q32
 
Arduino IDE used for developing. 
 

Connections:
- Pin3  EN - "1"
- Pin16 GPIO15 - "0"
- Pin18 GPIO0 - On reboot "0" - programming mode (115200, 8N1)
- Pin4  GPIO16 connect to Pin1 RST for wake-up from sleep

- GPIO4 sensor 1
- GPIO0 sensor 2

Warning: Not use GPIO2 for sensors in.
Sensors "DHT22" is AOSONG AM2302.

For store variables during wake up from sleep used RTC array of 20 bytes.

Published to broker of project http://iot-playground.com.
Every 60 second board wake up from deep sleep and check sensors witout wifi enabled. If need post values of sensor then enabled wifi, connected to AP, posted values to broker server, and at finished stage entered to deep sleep mode.
After wake up by GPIO16 simulated reset, and program start form setup() section of sketch. Stored values in structure variable "old" restored form RTC memory.


Power:
- Active mode: 72mA with Wifi
- Active mode: 14mA w/o Wifi
- Sleep mode :  0.02mA




