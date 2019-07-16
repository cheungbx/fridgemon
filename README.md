# fridgemon

This is a ESP8266 (nodeMCU D1 Mini) project to read DHT22 humidity sensor for temperature (-40C to 125C) and humidity and display at the OLED I2C SSD1306 128 x 64 pixels. Wifi is used publish data to the MQTT server of adafruit.io
WIFI is also used to connect to the NTP server to set the real time clock inside ESP8266 during boot up.
A digital clock will be displayed on the OLED.
The whole thing can be attached to the door of the refrigerator with the sensor placed inside the ice box to post temperature and humidity info around the clock once every 10 minutes and whenever the temperature and humidity value form the sensor changes.
Optionally, you can use a button to control a light switch (additional Arduino Relay module required).
This can then be mounted inside the light switch on the wall to record temp and humidity and allow the light to be turned on or off using MQTT.

You can also link your MQTT broker with IFTTT and with Google Home assistant or Amazon Alexis to control light switch by voice.

A push button for "Upgrade" can be configured on the MQTT dashboard, if clicked, you can put the ESP8266 in the firmware upgrade mode and receive download from the Arduino IDE for the latest codes. 

// define the ESP 8266 WeMos Mini D1 pin out.
//  Note the pin name printed on the circuit board is in the form of "D1, D2" may confuse you as the real GPIO pin numnbers are totally different.
//  Refer to this mapping 
//  A0 = ADC0   
//  D0 = GPIO16 Wake from sleep, No pull-up resistor, but pull-down instead Should be connected to RST to wake up
//  D1 = GPIO5  SCL (I2C)
//  D2 = GPIO4  SDA (I2C)
//  D3 = GPIO0  Boot Mode Select, 3.3V No Hi-Z
//  D4 = GPIO2  Boot Mode Select, 3.3V at boot up time. Don’t connect to ground at boot time Sends debug data at boot time
//  D5 = GPIO14 SCK  (SPI)
//  D6 = GPIO12 MISO (SPI)
//  D7 = GPIO13 MOSI (SPI)
//  D8 = GPIO15 SS   (SPI), At 0V, don't use any pull-up resistor, otherwise, will short circuit the power supply.
//  TX = GPIO1
//  RX = GPIO3
//       GPIO 6-11     Connected internally for Flash memory control,  Not usable, and not broken out

#define versionText  "Fridge Mon v1.0"
#define authorText  "Billy Cheung"
 
#define button_PIN  12    //  pushbutton pin -  on ESP 8266 generic module ESP-01s
#define DHTPIN      13    // data pin for the humidity sensor
#define light_PIN   14    //  Relay pin - on ESP 8266 generic module ESP-01s
