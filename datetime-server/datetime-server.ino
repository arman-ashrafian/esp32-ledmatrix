// Arman Ashrafian
// ESP32 LED Matrix Program

#define ESP32

// -------- Standard Libraries --------
#include <PxMatrix.h>
#include <WiFi.h>
#include "time.h"

#include "secret.h" // wifi info

// Pins connected to LED Matrix
#define P_LAT 22
#define P_A 19
#define P_B 23
#define P_C 18
#define P_D 5
#define P_E 15
#define P_OE 2

const char* ssid       = SSID;
const char* password   = PASSWORD;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

const char* HTML_STRING = "<!DOCTYPE html>"
"<html>"
"<head>"
"    <meta charset='utf-8' />"
"    <meta http-equiv='X-UA-Compatible' content='IE=edge'>"
"    <title>LED Matrix</title>"
"    <meta name='viewport' content='width=device-width, initial-scale=1'>"
"</head>"
"<body>"
"    <button><a href='/T'>Display Date & Time</a></button>"
"</body>"
"</html>";

hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

// this controls the brightness somehow
uint8_t display_draw_time=30;

// contructor for PXMatrix
PxMATRIX display(64,32,P_LAT,P_OE,P_A,P_B,P_C,P_D);

// server listening on port 80
WiFiServer server(80);

// http request
String header;

// client connecting to this server
WiFiClient client;

// my name
char firstname[] = "ARMAN";
char lastname[] = "ASH";

// -------------- Some standard colors ----------------
uint16_t myRED = display.color565(255, 0, 0);
uint16_t myGREEN = display.color565(0, 255, 0);
uint16_t myBLUE = display.color565(0, 0, 255);
uint16_t myWHITE = display.color565(255, 255, 255);
uint16_t myYELLOW = display.color565(255, 255, 0);
uint16_t myCYAN = display.color565(0, 255, 255);
uint16_t myMAGENTA = display.color565(255, 0, 255);
uint16_t myCOLORS[7]={myRED,myGREEN,myBLUE,myWHITE,myYELLOW,myCYAN,myMAGENTA};

void IRAM_ATTR display_updater()
{
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&timerMux);
  display.display(display_draw_time);
  portEXIT_CRITICAL_ISR(&timerMux);
}

void display_update_enable(bool is_enable)
{
  if (is_enable)
  {
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &display_updater, true);
    timerAlarmWrite(timer, 2000, true);
    timerAlarmEnable(timer);
  }
  else
  {
    timerDetachInterrupt(timer);
    timerAlarmDisable(timer);
  }

}

// ------------------------------
//      Drawing Functions
// ------------------------------

void draw_name(uint16_t color) {
  display_update_enable(true);

  display.setTextColor(myBLUE);
  display.setTextSize(1);
  display.setCursor(5,2);

  display.print(firstname);
  delay(1000);
  display.write(' ');
  display.setTextColor(myGREEN);
  display.print(lastname);
  delay(1000);

  yield();

}


void draw_datetime(uint16_t color1, uint16_t color2)
{
  struct tm timeinfo;       // unix time info
  char buffer1[20];         // buffer to hold "Day Hour:Min"
  char buffer2[20];         // buffer to hold "Month Day"

  if(!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  display.clearDisplay();

  // Day Hour:Min
  strftime(buffer1,20,"%a %I:%M",&timeinfo);
  display.setCursor(2,2);
  display.setTextColor(myCOLORS[color1]);
  display.print(buffer1);
  Serial.println(buffer1);
  // Month Day
  strftime(buffer2,20,"%b %d",&timeinfo);
  display.setCursor(20,20);
  display.setTextColor(myCOLORS[color2]);
  display.print(buffer2);
  Serial.println(buffer2);

  display_update_enable(true);
  yield();
  delay(10000);
  display_update_enable(false);
}


// connects to wifi and and displays
// when finished. 
void connect_wifi() {
  WiFi.begin(ssid, password);
  display.setCursor(2,5);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  display.clearDisplay();
  display.setTextColor(myCYAN);
  display.setCursor(10,2);
  display.print("WiFi");
  display.setCursor(2,10);
  display.print("Connected!");

  Serial.println(WiFi.localIP());

  display_update_enable(true);
  yield();
}

// --------- Sketch --------

void setup() {
  Serial.begin(115200);
  display.begin(16);  // 1:16 Scan Rate

  // slow update
  display.setFastUpdate(true);

  // delay 5 seconds after connecting 
  connect_wifi();
  delay(2000);
  // config to PST
  configTime(gmtOffset_sec * 4, daylightOffset_sec, ntpServer);
  // start server on port 80
  server.begin();
}

// starting color indices
int color_index1 = 0;
int color_index2 = 3;
bool showtime = false;
// ------- Main Loop --------
void loop() { 
  client = server.available();

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /D") >= 0) {
              display.clearDisplay();
              display.setCursor(5,5);
              display.setTextColor(myBLUE);
              display.print("GET /D");
            }
            else if(header.indexOf("GET /T") >= 0) {
              showtime = true;
            }
            
            client.println(HTML_STRING);
            // The HTTP response ends with another blank line
            client.println();
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }

  if(showtime) {
    struct tm timeinfo;       // unix time info
    char buffer1[20];         // buffer to hold "Day Hour:Min"
    char buffer2[20];         // buffer to hold "Month Day"

    if(!getLocalTime(&timeinfo)) {
      Serial.println("Failed to obtain time");
      return;
    }

    display.clearDisplay();

    // Day Hour:Min
    strftime(buffer1,20,"%a %I:%M",&timeinfo);
    display.setCursor(2,2);
    display.setTextColor(myCOLORS[color_index1]);
    display.print(buffer1);

    // Month Day
    strftime(buffer2,20,"%b %d",&timeinfo);
    display.setCursor(20,20);
    display.setTextColor(myCOLORS[color_index2]);
    display.print(buffer2);

    // change color
    color_index1 = (color_index1 + 1) % 7;
    color_index2 = (color_index2 + 1) % 7;

    delay(2000);
  }
}
