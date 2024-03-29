 fridgemon v2.0
 2019 08 06 
 Author: Billy Cheung
 
 Extra libraries required:
 This program uses these two sensor libraries tht can be downloaded from the links below. 
 Click "Clone and Download"  to download the zip file.
 Then add to Arduino IDE by clicking 
 Sketch->Include Library->Manage Library->Add .zip library

 SHT20 - Humidity and Temperature sensor by DFRobot.
 https://github.com/DFRobot/DFRobot_SHT20


 light sensor BH1750FVI by PeterEmbedded
 https://github.com/PeterEmbedded/BH1750FVI

 PubSubClient by Nick O’Leary version 2.7 
 https://github.com/knolleary/pubsubclient/releases/tag/v2.7

 ESP8266 and ESP32 Oled Driver for SSD1306 Display  v 3.2.6 by Daniel Eichhorn, Fabrice Weiberg.
 https://github.com/ThingPulse/esp8266-oled-ssd1306

 
 
 Function:  This project is to create an IoT light switch with embedded temperature and humidity senosr. 
            This utilize a ESP8266 WeMos D1 mini , a  SHT20 temperature and humidity senor
            You can optionally add a lux meter BH1750 to sense the light intensity
            You can optionally add a switch and a relay module to turn on/off lights.
            You can switch your lights on or off either by pressing a physical button, or toggle the virtual button on Adafruit.IO web page.
            The Adafruit.IO dashboard displays temperature and humidity measurements over time.
            A sample is taken every 10 seconds (can be defined) and publised to the Adafruit.IO dashboard using MQTT messages if the data changes.
            If there are no changes, the data will be published every 10 minutes.
            You can configure Triggers in the  Adafruit.IO alert services to send you emails when the temperature or humidity values  exceeds a predefined threshold.
            The IFTTT set up can be mapped to the google voice assistants or Amazon voice assistant.
            So you can turn lights on or of by your voice: OK Google, turn of bedroom light.
            You can add an I2C OLED SSD1306 module (1 inch) to display the status.
            
    
 Output:  1. Drives button pin on ESP 8266 generic module that connects to the Realy module or an LED. 
             The Relay switch can replace the original mechincal switch of the light.
          2. The state of the light is published to the MQTT topic that can be viewed on Adafruit IO web page with 
             audit trail of when the light is switched on and switched off.
          3. Temperature is displayed on Adafruit.io dashboart. 
          4. Humidity is displayed on Adafruit.io dashboard.
          5. If you also installed the OLED all these status will also be displayed on the OLED.
 
 Input:   Either one of these inputs can turn the relay on or off.
          1. electronic switch connected to toggle the button pin on ESP 8266 module 
          2. Virtual switches set up on Adafruit.IO web page as dashboard action buttons.
          3. DHT22 provides both the temperature and humdity measurement.

  OTA:    Over The Air code upgrade has been included to allow future codes to be upgraded over WIFI.
          Just press the "Upgrade" button on the MQTT dashboard to put the ESP8266 in a firmware program mode. 
          Then on the Arduino IDE, click Ports, and find the matching IP address for your board. 
          If you do not see the ip address, you can try exiting Arduino IDE and restarting again.
          Once you see the IP address listed as a port (under network port), you can 
          Click "->" to comple and upload new codes.
          It's much faster uploading codes thorugh WIFI than over Serial port.
          The only draw back is that the Serial port monitor function will not work, as the port has been changed from Serial to IP.
          If the upload is not done within 90 secponds, ESP8266 will resume the normal program execution mode.
 

 
 ESP 8266 WIFI library and the PubSubClient for MQTT IoT communications
 The Broker chosen is Adafruit.io.
 Adafruit IO is free for use but has a limitation of no more than 10 feeds, and no more than 30 actions/feeds per minute.
 So, you can not toggle the light switch or send a feed more frequently than once every 2 seconds.
 Otherwise, all feeds  will stall until the frequency of feeds falls below the threshold of 30 per minute.

*/


/*
 Detailed instructions for beginners - Billy Cheung 2019-07-16
======================================================================
Hardware set up
----------------
Get the ESP8266 NodeMCU D1 mini with the pins pre-soldered. Otherwise, solder the pins on first.
Insert the NodeMCon onto the breadboard.
Then make the following connections

Connect the SHT20  according to the following pin layout.
*** WARNING *** Do not swap VCC (+3V) and Ground (0V), otherwised, the chip will be burnt very quickly.
    ___________
    |  SHT20  |
    |         |
    ___________
      1 2 3 4
      V S S G
      C D C N
      C A L D
        

Connect the the Vcc of the SHT20 to +3V (VCC) of the ESP8266
Connect the GND of the SHT20  to the GND of the ESP8266
Connect the SDA of the SHT20 to GPIO GPIO4 aka D2 of the ESP8266
Connect the SCL of the SHT20 to GPIO GPIO5 aka D1 of the ESP8266s. 
The SHT sensor can be connected to a max 1 Meter long  dupoint cable.
Cable > 1 Meter may not work as the  data signals dies out due to the long distance and the interference may be too much.


Optionally, you can add a light sensor BH1750 for a Lux meter.
*    ___________
*
*    |  BH1750  |
*    |          |
*    ___________
*     1 2 3 4 5
*     V G S S A
*     C N C D D
*     C D L A R

Connect the the Vcc of the BH1750 to +3V (VCC) of the ESP8266
Connect the GND of the BH1750  to the GND of the ESP8266
Connect the SDA of the BH1750 to GPIO GPIO4 aka D2 of the ESP8266
Connect the SCL of the BH1750 to GPIO GPIO5 aka D1 of the ESP8266s. 
Unless wnat to use more than one BH1750 in your design, the Adr pin can be left unconnected or connected to GND to set to i2c address of 0x23
Otherwise, connect the 2nd BH1750's ADR pin to VCC or 3.3V  to set a differnet I2c address of  0x5C.

The BH1750 sensor can be connected to a max 1 Meter long dupoint cable.
Cable > 1M may not work as the  data signals dies out due to the long distance and the interference may be too much.

Optionally, if want to use this as a light switch for a lamp (or LED).
Connect a push button, one end to ground, the other end to GPIO12, D6 .
Connect the -ve terminal of an LED to GPIO 14 - D5
Connect the +ve terminal of the LED to one leg of a 1K ohm resister.
Connect the 2nd leg of gthe 1K ohm resister to ground, VCC (+3V).
  
Optionally, if you want to display the status on an SSD1306 I2C OLED.
Connect the the Vcc of the OLED to +3V (VCC) of the ESP8266
Connect the GND of the OLED  to the GND of the ESP8266
Connect the SDA of the OLED to GPIO GPIO4 aka D2 of the ESP8266
Connect the SCL of the OLED to GPIO GPIO5 aka D1 of the ESP8266


Software set up
---------------
This source can be found in  this github location.

https://github.com/cheungbx/fridgemon

Download the zip file into your computer.
Then unzip the file to the Arduino folder of your computer.
If you are using a MAC computer, then this will be your Documents\Arduino folder.
A folder called fridgemon will be created.
Then you will see two files
fridgemon.ino - this file
Credentials.h - the file that holds all WIFI and MQTT ids and passwords, and path names of all the MQTT feeds

For Adruino IDE set up
-----------------------
If this is your first time using Arduino IDE, download the Arduino software from this url:  https://www.arduino.cc/en/main/software
Windows, Mac and Linux client are supported.


Start Adruino IDE
Then click the menu.

Adruino->Preference->"Additional Boards Manager Url:", then input http://arduino.esp8266.com/stable/package_esp8266com_index.json
Then exit Arduino and restart to take in the preference.

Reopen Arduino IDE
Tools->Borad:Atmega... ->Boards Manager
Input ESP8266 to search and find the matching board drivers for ESP8266
Click Install to install.

Tools->Borad->LOLIN（Wemos）D1 R2 & Mini (Appear under the section for ESP8266, will not show up unless you did the preference setting above)
Tools->Flash size-> "4M (3M SPIFFS)"  (to partition how much flash memory to store the temperature and humidity data. 3M is the max. 1M is the min.)
Tools->Port:->"/dev/cu......."  - select your serail port used to connect to the ESP8266, if nothing shows up, check your cable or your driver for that USB-Serail port.
                                  Most ESP ports used the CH340 driver (e.g. Wemos D1 R1) or the CP1201 driver.

File->Open-> open this sketch file (main program source).
Update the credentials.h with your home wifi ssid and passwords
If you already have an account on adafruit.io, put in the id and passwords you used to login to Adafruit.io
and the AIO key.

Otherwise, you need to first register at Adafruit.io to get a free account.
Mr. Andreas Spiess has created a very good youtube video about how to set up your own MQTT account on Adafruit.io.
Please view this youtube video.  #48 Connect ESP8266 with the world (and IFTT) through MQTT and Adafruit.io (Tutorial)
https://www.youtube.com/watch?v=9G-nMGcELG8

Once you have the MQTT account on adafruit.io
Set up your dashboards and add the follwoing feeds.
temperature
humidity
lightsw
upgrade

Then update the full path of the feeds to the credential.h file.
e,g, 


#define  WIFIssid        "??????"
#define  WIFIpassword    "??????"
#define  OTA_password    "??????"

#define IO_USERNAME      "???????"
#define IO_KEY           "***************************"
#define MQTT_SERVER      "io.adafruit.com"    
#define MQTT_SERVERPORT  1883
#define MQTT_LUX         "????????/feeds/lux"
#define MQTT_AirTemp     "????????/feeds/airtemp"
#define MQTT_Humidity    "????????/feeds/humidity"
#define MQTT_Led         "????????/feeds/led"
#define MQTT_Pump        "????????/feeds/pump"
#define MQTT_Upgrade     "????????/feeds/upgrade"


Click tools->Serial Monitor. Set the serial speed to "115200 baud" to  match with this program.
This will allow you to see all the diagnosis messages from the program once the compile and upload is done.
Resize the serial monitor window size and the Arduino IDE size so you can see both on your screen.
On the to right of the Arduino IDE window where the source code is displayed, click "->" to compile and upload.
If you get file not found on files like "??????.h" e.g. NTPClient.h do the following:
Sketch->Include Library->Manage Libraries
at the search box type in the name of the file without the extension, e.g. NTPClient
Select to install the most matching library.
Sometimes you cannot find these missing libraries within the Adruino IDE.
Then you need to search in google and download them fro Github as zip files.
Then click Sketch->Include Library->Add .Zip libraries   and open these zip files to add to the Aruindo IDE.
Then recompile by clicking "->" to compile and upload.

This program uses these two sensor libraries tht can be downloaded from the links below. Click "Clone and Download"  to download the zip file.
Then add to Arduino IDE by clicking 
Sketch->Include Library->Manage Library->Add .zip library


 SHT20 - Humidity and Temperature sensor by DFRobot.
 https://github.com/DFRobot/DFRobot_SHT20


 light sensor BH1750FVI by PeterEmbedded
 https://github.com/PeterEmbedded/BH1750FVI

 PubSubClient by Nick O’Leary version 2.7 
 https://github.com/knolleary/pubsubclient/releases/tag/v2.7

 ESP8266 and ESP32 Oled Driver for SSD1306 Display  v 3.2.6 by Daniel Eichhorn, Fabrice Weiberg.
 https://github.com/ThingPulse/esp8266-oled-ssd1306



Repeat this process until all missing library files have been instlaled.

Once the program is uploaded, the board will be reset, and you will see diagnosis messages on the serail monitor.
Check that WIFI is connected successfully. If no, check the SSID nad password you put into credentials.h.
You can click the reset button on the ESP8266 board to reboot any time.
Or you can adjust the source code and clieck "->" to recompile and upload again.

Once wifi is successfully connected, you will see the message like the one below
Connecting to: yourwifissid
...........WiFi connected at: 192.168.1.113


Then you can login to your MQTT account in adafruit.io to view the dashboard and the feeds.
And create charts for the feeds and buttons for the light switch and upgrade.
Then test the other feature

*/
