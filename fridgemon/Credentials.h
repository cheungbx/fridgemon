/*

 Save your Credentials in a secret file so it will not accidentially reviewed with your source code.
 your Credentials.h file should be in the same folder as your source code for the arduino project and have the following content:
  I am useing adafruit.io as the MQTT broker and dashboard service. You can use other MQTT services on the internet.
*/
 
#define ssid            "your home wifi ssid"
#define password        "your home wifi password"
#define OTA_Password    "the password to upgrade firmware through WIFI"

#define IO_USERNAME     "username to login to adafruit.io" 
#define IO_KEY          "AIO Key - you can get from adafruit.io - click View AIO Key"
#define SERVER          "io.adafruit.com"  
#define SERVERPORT      1883
#define IO_Temperature "username/feeds/temperature"
#define IO_Humidity    "usedrname/feeds/humidity"
#define IO_Switch      "username/feeds/lightsw"
#define IO_Upgrade     "username/feeds/upgrade"

 
