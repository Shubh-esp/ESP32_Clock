**************************************************************************************************************************************************************************************************************************************************************************************
*Shubh-esp*
To make this code run, first you'll have to insure proper wiring->
    Wiring
TFT Display:
            VCC: 3.3V
            GND: GND
            CS: GPIO 15
            RST: GPIO 4
            A0: GPIO 2
            LED: GPIO 21 (Backlight)
Buzzer:
            Positive: GPIO 22
            Negative: GND
Now download latest version of "Arduino IDE"
Install board '.json' link file by->
             In Arduino IDE > Sketch > Prefrences > Additional Boards Manager > Paste -> https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
             Now, in--> Tools > Boards Mnager > Search "ESP32" > Install .
Open the "ESP32_Clock.ino" file and change your network credentials(router ssid and password)  If want to use Acces Poinnt (ESP's own wifi without any external wifi) do not change any thing.
If want to increase buzzer sound, change buzzer sound head to the code(the buzzer is already set to max ).
In tools > select board > ESP32 WROOM MODULE.
NOW hit the upload button
Wait for code to be compiled....
When the compiling is completed and connect shows up in the output window .....
Press and hold boot button on ESP 32  for 3 seconds and leave fro 1 second and then click the boot button once and leave.
The code will start uploading in percentages(%)
After the code is done uploading click the EN (reset) button once.
The time will be start showing on the tft.
If connect to a wifi the time will automaticly configired.
IF using access point you'll have to adjust the time by yourself through the WEB PORTAL.
If you are in INDIA your ip will propabbly will--> 192.168.4.1 or wifi ip will be--> 192.168.29.211
IF in another country contact your netork provider or head towards google or any other search engine or AI(chatGPT, etc.)
For any other problem or quiries contact me and Instagram--> @esp_projects.
//shubh_esp
//Instagram- @esp_projects
//YOUTUBE- esp_projects
**************************************************************************************************************************************************************************************************************************************************************
