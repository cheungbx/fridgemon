 /*
 Author: Billy Cheung
 
 

 
 Function:  This project is to create an IoT light switch with embedded temperature and humidity senosr. 
            This utilize a ESP8266 WeMos D1 mini , a DHT-11 or DHT22 temperature and humidity senor
            You can optionally add a switch and a relay module to turn on/off lights.
            You can switch your lights on or off either by pressing a physical button, or toggle the virtual button on Adafruit.IO web page.
            The Adafruit.IO dashboard displays temperature and humidity measurements over time.
            A sample is taken every 10 seconds (can be defined) and publised to the Adafruit.IO dashboard using MQTT messages if the data changes.
            If there are no changes, the data will be published every 10 minutes.
            You can configure Triggers in the  Adafruit.IO alert services to send you emails when the temperature or humidity values  exceeds a predefined threshold.
            The IFTTT set up can be mapped to the google voice assistants or Amazon voice assistant.
            So you can turn lights on or of by your voice: OK Google, turn of bedroom light.
            
    
 Output:  1. Drives button pin on ESP 8266 generic module that connects to the Realy module. 
             The Relay switch can replace the original mechincal switch of the light.
          2. The state of the light is published to the MQTT topic that can be viewed on Adafruit IO web page with 
             audit trail of when the light is switched on and switched off.
          3. Temperature is displayed on Adafruit.io dashboart. 
          4. Humidity is displayed on Adafruit.io dashboard.
 
 Input:   Either one of these inputs can turn the relay on or off.
          1. electronic switch connected to toggle the button pin on ESP 8266 module 
          2. Virtual switches set up on Adafruit.IO web page as dashboard action buttons.
          3. DHT11 or DHT22 (more expensive but more accurate) provides both the temperature and humdity measurement.

  OTA:    Over The Air code upgrade has been included to allow future codes to be upgraded over WIFI.
          Just press reset button on ESP8266, and within 15 seconds, upload codes through Arduino IDE by referencing the IP address of the ESP 8266 microcontroller.
 

 
 ESP 8266 WIFI library and the PubSubClient for MQTT IoT communications
 The Broker chosen is Adafruit IO.
 Adafruit IO is free for use but has a limitation of no more than 10 feeds, and no more than 30 actions/feeds per minute.
 So, you can not toggle the light switch faster than once every 2 seconds.
 Otherwise, all IO will stalled until the frequency of feeds/actions falls below the threshold of 30 per minute.

*/

// uncomment this line for debug mode to print  debug messages using Serial.print
// turn on your serial monitor and set the serial speed to 115200 baud.
#define DEBUG 1
#define lcdDisplay  1



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
                                  
const int OFF = HIGH,  ON = LOW;     // for WIFI relay module 


// These variables are for button debouncing
int reading, ButtonState, LastButtonState;             // the current reading from the input pin

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
const unsigned long debounceDelay = 30;    // the debounce time; increase if the output flickers



bool OTA_mode = false;
unsigned long OTA_timer;

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
//for MQTT messaging
#include <PubSubClient.h>

// for temp and humidity sensor DHT11 or DHT22
#include "DHT.h"
#include "Math.h"
#define DHTTYPE DHT22  // DHT11 or DHT22 - more accuracy and wider range

//    Connect the DHT11 or DHT22  according to the following pin layout.
//    *** WARNING *** Do not swap VCC (+3V) and Ground (0V), otherwised, the chip will be burnt very quickly.
//    ___________
//    |  DHT11  |
//    |    or   |
//    |  DHT22  |
//    |  front  |
//    |  with   |
//    |  holes  |
//    ___________
//      1 2 3 4
//      V D N G
//      C A C N
//      C T   D
//        A


unsigned long lastMeasurementTime = 0;
const unsigned long MeasurementInterval = 10000;  // 10 seconds
unsigned long lastNCMeasurementTime = 0;
const unsigned long NCMeasurementInterval = 600000;  // 10 min
DHT dht(DHTPIN, DHTTYPE, 11); // 11 works fine for ESP8266
float t1 = 99.0, humidity = 99.0;
float last_t1 = 0, last_humidity = 0;

/*
 Save your Credentials in a secret file so it will not accidentially reviewed with your source code.
 your Credentials.h file should be in the same folder as your source code for the arduino project and have the following content:
 
#define ssid          "your home wifi ssid"
#define password      "your home wifi password"
#define OTA_Password  "the password to upgrade firmware through WIFI"

// I am useing adafruit.io as the MQTT broker and dashboard service. You can use other MQTT services on the internet.

#define IO_USERNAME   "username to login to adafruit.io", 
#define IO_KEY        "AIO Key - you can get from adafruit.io - click View AIO Key"
#define SERVER          "io.adafruit.com"     server dns name for Adafruit.IO service.
#define SERVERPORT      1883
#define IO_Temperature "username/feeds/temperature"
#define IO_Humidity    "usedrname/feeds/humidity"
#define IO_Switch      "username/feeds/lightsw"
#define IO_Upgrade     "username/feeds/upgrade"

 */

#include "Credentials.h"



unsigned long entry;
byte LightState1 = 1; 
byte LastLightState1 = 0;


char valueStr[9];

String s1, hString, tString;

WiFiClient WiFiClient;
// create MQTT object
PubSubClient client(WiFiClient);


// variables for NTP time funciton
#include "time.h"
struct tm* timeNow;
const char* ntpServer = "hk.pool.ntp.org"; // if you do not know your local NT server, use "pool.ntp.org"
const long  gmtOffset_sec = 8 * 3600;      // mini sec
const int   daylightOffset_sec = 0;        // mini sec


unsigned long lastDisplayTime = 0;  // the last time the LCD display happened.
const unsigned long displayDelay = 1000;    // the display delay time in ms; increase if the output flickers


// for i2c OLED 128 x 64

#include "SSD1306Wire.h" 
// Include the UI lib
#include "OLEDDisplayUi.h"
// Initialize the OLED display using Wire library
// ESP32
// 21 -> SDA
// 22 -> SCL

SSD1306Wire  display(0x3c, 4, 5);  // OLED's I2c device address, sda pin, scl pin

OLEDDisplayUi ui ( &display );



void scan_i2c()
{
 int j;

int error,  nDevices = 0;
int i2c_addr[] = {0,0,0,0,0,0,0,0};

Serial.println("Scanning...");
 
  for (j = 1; j < 127; j++)
  {
    // The i2c scanner uses the return value of Write.endTransmisstion to see if a device did acknowledge to the address.
    Wire.beginTransmission(j);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
        if (j < 16) {
        Serial.print("0");
      }
      Serial.print(j, HEX);
      Serial.println(" !");
      i2c_addr[nDevices]=j;
      nDevices++;
    }
    else if (error == 4)
    {
      Serial.print("Unknown error at address 0x");
      if (j < 16) {
        Serial.print("0");
      }
      Serial.println(j, HEX);
    }
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
  }
  else {
    Serial.print (nDevices);
    Serial.println (" I2C devices found");   
  }
}
//


bool  reconnect_MQTT()
{

 if (!client.connected())   // check if the MQTT server is still connected, if not, reinitiate the connection.
   {
#ifdef DEBUG 
    Serial.println("Attempting MQTT connection...");
#endif
    // Attempt to connect
    if (client.connect("", IO_USERNAME, IO_KEY)) {
#ifdef DEBUG 
      Serial.print("connected using account name: ");
      Serial.println (IO_USERNAME); 
#endif
      // ... and resubscribe
//      client.subscribe(USERNAME PREAMBLE IO_feed, 1);

      client.subscribe(IO_Switch, 1);
      delay (3000);
      client.subscribe(IO_Upgrade, 1);   
      return true;
     } else {
#ifdef DEBUG 
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
#endif      
      return false;
    }
   }
}   


void setup() {

#ifdef DEBUG  
    Serial.begin(115200);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
#endif

#ifdef lcdDisplay
    display.init();
    scan_i2c();
    display.clear();
    display.flipScreenVertically();
 
    display.setTextAlignment(TEXT_ALIGN_LEFT);

    display.clear();
//  display.setFont(ArialMT_Plain_10);
    display.setFont(ArialMT_Plain_16);

    display.drawString(0, 0,  versionText);
    display.drawString(0, 24, authorText);
    display.drawString(0, 48, ssid);
    display.display();
#endif    


// Connect to WIFI
 
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
#ifdef DEBUG  
    Serial.print(".");
#endif

  }
  s1 = WiFi.localIP().toString();
#ifdef lcdDisplay
  display.clear();
  display.drawString(0, 0,  versionText);
  display.drawString(0, 24, ssid);
  display.drawString(0, 48, s1);
  display.display();   
#endif    

#ifdef DEBUG 
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");

  Serial.println(s1);

   // WiFi.printDiag(Serial);
#endif

//init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);


  
  
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
   ArduinoOTA.setPassword(OTA_Password);

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }
#ifdef DEBUG  
    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
#endif 
  });
  
  ArduinoOTA.onEnd([]() {
#ifdef DEBUG  
    Serial.println("\nEnd");
#endif   
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
#ifdef DEBUG
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
#endif
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
 #ifdef DEBUG  
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");

    }
#endif
  });
  
  ArduinoOTA.begin();
#ifdef DEBUG  
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
#endif


//original setup code starts here
  client.setServer(SERVER, SERVERPORT);
  
  client.setCallback(callback);


 pinMode(button_PIN, INPUT_PULLUP);
 pinMode(light_PIN, OUTPUT);

 digitalWrite(light_PIN, OFF);   // HIGH = turn off relay

  // set up temp. and humidity sensor
 dht.begin();

}





void loop() {

   bool displayNow = false;

// check if somoene switched on on the air firmware upgrade switch on the MQTT dashboard on the IO.adafruit web site.
// if so, perform on the air WIFI download if new firmware. The user will need to use the Adruino IDE and select the port as the IP address of the ESP8266
// then click upload to program the ESO 8266 with new firmware codes within 1 minute. Otherwise, the program will resume the regular funciton and ignore the upgrade.
if (OTA_mode) {
   OTA_timer = 60;

   // handle WIFI code upload requests once at setup
   while (OTA_mode) {
    ArduinoOTA.handle();
    delay (1000); 
    OTA_timer --;
    s1 = String (OTA_timer) + " sec";

#ifdef lcdDisplay 
    display.clear();
    display.drawString(0, 0,  "OTA Upgrade");
    display.drawString(0, 24, "Count Down");
    display.drawString(0, 48, s1);
  display.display();   
#endif

#ifdef DEBUG 
    Serial.println ("Over the Air Upgrade countdown : " + s1);
#endif

   if (OTA_timer <= 0)  OTA_mode = false;
  
   }
}

  yield();  // this command passes control to the WIFI handler to receive any new IP packets received by the WIFI cobntroller, without this line, you cannot receive any new ip packets.


  if (! reconnect_MQTT()) {
   delay(5000);  // if failed to connect to MQTT, wait 5 seconds before retrying.
  }

  time_t now = time (nullptr);
  timeNow = localtime(&now);

if (abs((millis() - lastDisplayTime) > displayDelay)) 
{ 
  lastDisplayTime =   millis();
  displayNow = true;

}
 
  //char buffer[10];
// To avoid bruning the sensors and creating too much data, sensors are only read periodically according to the measurement interval
   if (abs(millis() - lastMeasurementTime) > MeasurementInterval) {

    // take new measurement
     humidity = dht.readHumidity();       // Read humidity (percent)
     t1 = dht.readTemperature(false);     // Read temperature as Celsius
   
     if (isnan(humidity)) {
#ifdef DEBUG  
     Serial.println("Failed to read humidity from DHT sensor!");
#endif    
     }
    else if ((humidity != last_humidity) or  (abs (millis() - lastNCMeasurementTime) > NCMeasurementInterval)) {
     // publish the humidity if it's changed or it's time
#ifdef DEBUG  
     Serial.print("Published Humidity: ");
     Serial.println(humidity);
#endif   
     last_humidity = humidity;
     hString = (String) humidity;
     hString.toCharArray(valueStr, 5);
     client.publish(IO_Humidity, valueStr);
     displayNow = true;
      }
      
    if (isnan(t1)) {
#ifdef DEBUG  
     Serial.println("Failed to read temp from DHT sensor!");
#endif
      }
     else if ((t1 != last_t1) or  (abs(millis() - lastNCMeasurementTime) > NCMeasurementInterval)) {
    // publish the temperature if it's changed or it's time

#ifdef DEBUG    
     Serial.print("Published Temperature: ");
     Serial.println(t1);
#endif
     last_t1 = t1;
     tString = (String) t1;
     tString.toCharArray(valueStr, 5);
     client.publish(IO_Temperature, valueStr);
     displayNow = true;
     }
   lastMeasurementTime = millis();

    if (abs(millis() - lastNCMeasurementTime) > NCMeasurementInterval) {
      lastNCMeasurementTime = NCMeasurementInterval;
      }
    }

 
 #ifdef lcdDisplay  
   if (displayNow) {
    display.clear();
    char dateTimeText[20];
    sprintf(dateTimeText, "%02d/%02d/%02d %02d:%02d:%02d", timeNow->tm_year-100,timeNow->tm_mon+1, timeNow->tm_mday, timeNow->tm_hour,timeNow->tm_min,timeNow->tm_sec);
    display.drawString (0,0, dateTimeText);
     display.drawString(0, 24, "Humidity " + hString+"%");
    display.drawString(0, 48, "Temp.     " + tString+"c");
    display.display();   
   }
#endif

  

// check if anyone pressed the light switch, if so, conduct button debouncing to read the true value of a switch after the debounce period to bypass any electrostatic noises.
  // read the state of the button into a local variable:
   reading = digitalRead(button_PIN);
    
  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != LastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  
 
  if (abs((millis() - lastDebounceTime) > debounceDelay)) 
   {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:
 
    // if the button state has changed:
    if (reading != ButtonState) 
     {

      ButtonState = reading;


      
    //  button pressed = 1 as the input pin it pulled high when not pressed and pulled low when pressed.
    // check if button has been toggled,  i.e. from off (1) to on (0) to off (1).
      if (ButtonState == 1)    // remove this if statement if you are using a toggle switch instead of a press button.
     { 

        if (LightState1==1) 
        { 
           LightState1=0; 
           digitalWrite(light_PIN, OFF);  // turn off relay
           client.publish(IO_Switch, "OFF");
 #ifdef DEBUG 
           Serial.print ("Published ");
           Serial.print (IO_Switch);
           Serial.println (" OFF");
 #endif
            }
        else 
         {
           LightState1=1; 
           digitalWrite(light_PIN, ON);  // turn on relay 
           client.publish(IO_Switch, "ON");
 #ifdef DEBUG 
           Serial.print ("Published ");
           Serial.print (IO_Switch);
           Serial.println (" ON");
 #endif
          }
        }
     }
    }
 
  LastButtonState = reading;

// checks if there is any MQTT messages received, if so, the function "callback" will be invoked to process the message.
  client.loop();
}


//----------------------------------------------------------------------

// this is the function to process MQTT messages received from the MQTT server through the WIFI connected to the internet 


void callback(char* topic, byte * data, unsigned int length) {
  // handle message arrived {

  String s1 = (String)topic;
#ifdef DEBUG  
  Serial.print("MQTT Received - ");
  Serial.print(topic);
  Serial.print(":");
  for (int i = 0; i < length; i++) {
    Serial.print((char)data[i]);
  }
  Serial.println();
#endif

 
  if  (s1 == IO_Switch)
   {

// Test if  the data is "OFF"
    if (data[1] == 'F')  {     
      LightState1 = 0;
      digitalWrite(light_PIN, OFF);  // turn off relay
     } else {
      LightState1 = 1;
      digitalWrite(light_PIN, ON);  // turn on relay 


    }
   }
  else if (s1 == IO_Upgrade) {
    if (data[1] == 'N')  {    
     OTA_mode = true; 

#ifdef DEBUG      
    Serial.println ("Turned ON code upgrade mode.");
#endif
    }
   else if (data[1] == 'F')  {   
         OTA_mode = false; 
#ifdef DEBUG      
    Serial.println ("Turned OFF code upgrade mode..");
#endif
   }

  }

}
