# fridgemon

This is a ESP8266 (nodeMCU D1 Mini) project to read DHT22 humidity sensor for temperature (-40C to 125C) and humidity and display at the OLED I2C SSD1306 128 x 64 pixels , and also use WIFI MQTT to post to the MQTT server of adafruit.io
WIFI is also used to connect to the NTP server to set the real time clock inside ESP8266 during boot up.
A digital clock will be displayed on the OLED.
The whole thing can be attached to the door of the refrigerator with the sensor placed inside the ice box to post temperature and humidity info around the clock once every 10 minutes and whenever the temperature and humidity value form the sensor changes.
Optionally, you can use a button to control a light switch (additional Arduino Relay module required).
This can then be mounted inside the light switch on the wall to record temp and humidity and allow the light to be turned on or off using MQTT.

You can also link your MQTT broker with IFTTT and with Google Home assistant or Amazon Alexis to control light switch by voice.

A push button for "Upgrade" can be configured on the MQTT dashboard, if clicked, you can put the ESP8266 in the firmware upgrade mode and receive download from the Arduino IDE for the latest codes. 
