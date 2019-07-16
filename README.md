# fridgemon
/*
 Author: Billy Cheung
 
 

 
 Function:  This project is to create an IoT light switch with embedded temperature and humidity senosr. 
            This utilize a ESP8266 WeMos D1 mini , a  DHT22 temperature and humidity senor
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
          Just press reset button on ESP8266, and within 15 seconds, upload codes through Arduino IDE by referencing the IP address of the ESP 8266 microcontroller.
 

 
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

Connect the DHT22  according to the following pin layout.
*** WARNING *** Do not swap VCC (+3V) and Ground (0V), otherwised, the chip will be burnt very quickly.
    ___________
    |  DHT22  |
    |  front  |
    |  with   |
    |  holes  |
    ___________
      1 2 3 4
      V D N G
      C A C N
      C T   D
        A

Pin 1-VCC connect to the 3V/VCC of the ESP8266 board
pin 2-Data connect to GPIO 13 (appeared on circuit board of NodeMCU D1 as "MOSI/D7" on NodeMCU D1
Pin 4-GND connect to GND (ground) of the ESP8266 board.
Connect your USB cable from your computer to the ESP8266 board. Make sure you use a good cable. 
Many charging only cable do not have the data pins. 
Long USB cables > 1.5M  may  not work as the data signals dies out due to the long distance.

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
Credentials.h - the file that holds all WIFI and MQTT ids and passwords.

For Adruino IDE set up
-----------------------
If this is your first time using Arduino IDE, download the Arduino software from this url:  https://www.arduino.cc/en/main/software
Windows, Mac and Linux client are supported.


Start Adruino IDE
Then click the menu.
Adruino->Preference->"Additional Boards Manager Url:", then input http://arduino.esp8266.com/stable/package_esp8266com_index.json
Then exit Arduino and restart to take in the preference.

Reopen Arduino IDE
Tools->Borad->ESP8266 Wemos D1 R1  (Appear under the section for ESP8266, will not show up unless you did the preference setting above)
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

Onc eyou have the MQTT account on adafruit.io
Set up your dashboards and add the follwoing feeds.
temperature
humidity
lightsw
upgrade

Then update the full path of the feeds to the credential.h file.
e,g, 

#define IO_Temperature "cheungbx2/feeds/temperature"
#define IO_Humidity    "cheungbx2/feeds/humidity"
#define IO_Switch      "cheungbx2/feeds/lightsw"
#define IO_Upgrade     "cheungbx2/feeds/upgrade"


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

Repeat this process until all missing library files have been instlaled.

Once the program is uploaded, the board will be reset, and you will see diagnosis messages on the serail monitor.
Check that WIFI is connected successfully. If no, check the SSID nad password you put into credentials.h.
You can click the reset button on the ESP8266 board to reboot any time.
Or you can adjust the source code and clieck "->" to recompile and upload again.

Once wifi is successfully connected, you will see the message like th eone below
Connecting to: homewifissid
...........WiFi connected at: 192.168.1.113



*/
