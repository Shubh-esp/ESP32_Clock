#include <WiFi.h>
#include <WebServer.h>
#include <EEPROM.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Wire.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Define TFT pins
#define TFT_CS    5
#define TFT_RST   16
#define TFT_DC    17

// Initialize ST7735 TFT display
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Web server on port 80
WebServer server(80);

// WiFi credentials
const char* ssid = "Apocalypse-Zone";
const char* password = "starconnect4g";

// NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Variables for alarm time and settings
int alarmHour = 0;
int alarmMinute = 0;
bool alarmActive = false;
bool screenOrientation = true; // true for portrait, false for landscape
bool alarmTriggered = false; // Flag to indicate if the alarm has been triggered

// Buzzer pin (PWM-capable GPIO 22)
#define BUZZER_PIN 22

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Initialize TFT display
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(screenOrientation ? 1 : 3); // Set initial rotation

  // Initialize EEPROM for persistent storage
  EEPROM.begin(512);

  // Initialize buzzer pin
  pinMode(BUZZER_PIN, OUTPUT);

  // Initialize Wi-Fi in access point mode
  WiFi.softAP("ESP32Clock", "12345678");
  
  // Connect to Wi-Fi network
  connectToWiFi();

  // Initialize NTP client
  timeClient.begin();
  timeClient.setTimeOffset(19800); // Set your time zone offset in seconds
  
  // Initialize web server routes
  server.on("/", handleRoot);
  server.on("/setAlarm", handleSetAlarm);
  server.on("/setOrientation", handleSetOrientation);
  server.on("/setTextSettings", handleSetTextSettings);
  
  server.begin();
}

void loop() {
  server.handleClient();
  timeClient.update();
  updateDisplay();
  checkAlarm();
}

void updateDisplay() {
  // Clear previous display content
  tft.fillScreen(ST7735_BLACK);

  // Fetch current time from NTP server
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  
  // Convert 24-hour format to 12-hour format with AM/PM
  String ampm = (currentHour < 12) ? "AM" : "PM";
  currentHour = (currentHour % 12 == 0) ? 12 : currentHour % 12;

  // Format time into string HH:MM AM/PM
  String formattedTime = String(currentHour) + ":" + (currentMinute < 10 ? "0" : "") + String(currentMinute) + " " + ampm;
  
  // Fetch current date
  time_t rawTime = timeClient.getEpochTime();
  struct tm* timeInfo = localtime(&rawTime);
  char dateBuffer[11];
  strftime(dateBuffer, sizeof(dateBuffer), "%Y-%m-%d", timeInfo);
  String currentDate = String(dateBuffer);

  // Fetch current day
  String currentDay = getCurrentDay();

  // Calculate text sizes and positions
  int screenWidth = tft.width();
  int screenHeight = tft.height();

  // Display current time with adjustable text size
  tft.setTextSize(3); // Default text size
  int16_t textWidth = formattedTime.length() * 18; // Default text width
  int16_t xTime = (screenWidth - textWidth) / 2; // Center horizontally
  int16_t yTime = (screenHeight - 3 * 16) / 2 - 20; // Center vertically with offset
  tft.setCursor(xTime, yTime);
  tft.setTextColor(ST7735_WHITE);
  tft.println(formattedTime);

  // Display the current date with adjustable text size
  tft.setTextSize(2); // Default text size
  textWidth = currentDate.length() * 12; // Default text width
  int16_t xDate = (screenWidth - textWidth) / 2; // Center horizontally
  int16_t yDate = (screenHeight - 2 * 8) / 2 + 20; // Center vertically with offset
  tft.setCursor(xDate, yDate);
  tft.println(currentDate);

  // Display the current day with adjustable text size
  tft.setTextSize(2); // Default text size
  textWidth = currentDay.length() * 12; // Default text width
  int16_t xDay = (screenWidth - textWidth) / 2; // Center horizontally
  int16_t yDay = screenHeight - 2 * 8 - 10; // Bottom with offset
  tft.setCursor(xDay, yDay);
  tft.println(currentDay);

  // Display alarm status if active
  if (alarmActive) {
    tft.setTextSize(2); // Default text size
    textWidth = 10 * 12; // Default text width
    int16_t xAlarm = (screenWidth - textWidth) / 2; // Center horizontally
    int16_t yAlarm = yDay - 2 * 8 - 10; // Above day text with offset
    tft.setCursor(xAlarm, yAlarm);
    tft.setTextColor(ST7735_RED); // Set color to indicate alarm
    tft.println("ALARM ON!");
  }
}

String getCurrentDay() {
  // Days of the week
  String daysOfTheWeek[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
  time_t rawTime = timeClient.getEpochTime();
  struct tm* timeInfo = localtime(&rawTime);
  return daysOfTheWeek[timeInfo->tm_wday];
}

void checkAlarm() {
  // Check if current time matches alarm time
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  
  if (alarmActive && !alarmTriggered && currentHour == alarmHour && currentMinute == alarmMinute) {
    // Activate alarm (buzzer or other action)
    Serial.println("Alarm triggered!");
    alarmTriggered = true;
    activateBuzzer();
  }

  // Reset alarmTriggered flag if the minute has passed
  if (alarmTriggered && (currentHour != alarmHour || currentMinute != alarmMinute)) {
    alarmTriggered = false;
  }
}

void activateBuzzer() {
  unsigned long buzzerStartTime = millis();
  while (millis() - buzzerStartTime < 60000) { // Run buzzer for 60,000 milliseconds (1 minute)
    analogWrite(BUZZER_PIN, 255); // Set PWM duty cycle to 50% (0-255 range)
    delay(500); // Buzzer on for 500 milliseconds
    analogWrite(BUZZER_PIN, 0);   // Turn off buzzer
    delay(500); // Buzzer off for 500 milliseconds
  }
}

void handleRoot() {
  // Get AP IP address
  IPAddress apIP = WiFi.softAPIP();

  // Get WiFi IP address
  IPAddress wifiIP = WiFi.localIP();

  // HTML interface for main page
  String html = "<html><body>";
  html += "<h1 style='text-align:center;'>ESP32 Alarm Clock</h1>";
  html += "<p style='text-align:center;'>Current Time: " + timeClient.getFormattedTime().substring(0, 5) + "</p>";
  html += "<p style='text-align:center;'>AP IP Address: " + apIP.toString() + "</p>";
  html += "<p style='text-align:center;'>WiFi IP Address: " + wifiIP.toString() + "</p>";
  html += "<form action='/setAlarm' method='post' style='text-align:center;'>";
  html += "<label>Set Alarm (HH:MM):</label><br>";
  html += "<input type='number' name='hour' min='0' max='23' required> :";
  html += "<input type='number' name='minute' min='0' max='59' required><br>";
  html += "<input type='submit' value='Set Alarm'>";
  html += "</form>";
  html += "<form action='/setOrientation' method='post' style='text-align:center;'>";
  html += "<label>Set Screen Orientation:</label><br>";
  html += "<input type='radio' name='orientation' value='portrait' checked> Portrait ";
  html += "<input type='radio' name='orientation' value='landscape'> Landscape<br>";
  html += "<input type='submit' value='Set Orientation'>";
  html += "</form>";
  html += "<form action='/setTextSettings' method='post' style='text-align:center;'>";
  html += "<label>Set Text Settings:</label><br>";
  html += "<label>Text Size (1-3):</label><br>";
  html += "<input type='number' name='textSize' min='1' max='3' required><br>";
  html += "<label>Text Color (#RRGGBB):</label><br>";
  html += "<input type='text' name='textColor' pattern='^#([A-Fa-f0-9]{6}|[A-Fa-f0-9]{3})$' required><br>";
  html += "<input type='submit' value='Set Text Settings'>";
  html += "</form>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

void handleSetAlarm() {
  // Handle POST request to set alarm time
  if (server.hasArg("hour") && server.hasArg("minute")) {
    alarmHour = server.arg("hour").toInt();
    alarmMinute = server.arg("minute").toInt();
    alarmActive = true;
    server.send(200, "text/plain", "Alarm set successfully!");
  } else {
    server.send(400, "text/plain", "Invalid request");
  }
}

void handleSetOrientation() {
  // Handle POST request to set screen orientation
  if (server.hasArg("orientation")) {
    String orientation = server.arg("orientation");
    screenOrientation = (orientation == "portrait") ? true : false;
    tft.setRotation(screenOrientation ? 1 : 3);
    server.send(200, "text/plain", "Screen orientation set successfully!");
  } else {
    server.send(400, "text/plain", "Invalid request");
  }
}

void handleSetTextSettings() {
  // Handle POST request to set text size and color
  if (server.hasArg("textSize") && server.hasArg("textColor")) {
    int textSize = server.arg("textSize").toInt();
    String textColor = server.arg("textColor");
    setDisplaySettings(textSize, textColor);
    server.send(200, "text/plain", "Text settings updated successfully!");
  } else {
    server.send(400, "text/plain", "Invalid request");
  }
}

void setDisplaySettings(int textSize, String textColor) {
  // Set text size
  if (textSize >= 1 && textSize <= 3) {
    tft.setTextSize(textSize);
  } else {
    Serial.println("Invalid text size input!");
    return;
  }

  // Set text color
  unsigned int color = strtol(textColor.c_str() + 1, NULL, 16); // Convert hex string to integer
  tft.setTextColor(color);
}

void connectToWiFi() {
  // Connect to Wi-Fi network
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi..");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
}