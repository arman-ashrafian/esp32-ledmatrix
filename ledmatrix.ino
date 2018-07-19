// Arman Ashrafian
// ESP32 LED Matrix Program

#define ESP32

// -------- Standard Libraries --------
#include <PxMatrix.h>
#include <WiFi.h>
#include "time.h"

#include "secret.h"

const char* ssid       = SSID;
const char* password   = PASSWORD;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;


// Pins connected to LED Matrix
#define P_LAT 22
#define P_A 19
#define P_B 23
#define P_C 18
#define P_D 5
#define P_E 15
#define P_OE 2

hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

// this controls the brightness somehow
uint8_t display_draw_time=15;

// Contructor for PXMatrix
PxMATRIX display(64,32,P_LAT, P_OE,P_A,P_B,P_C,P_D);

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

void draw_time(uint16_t color)
{
  struct tm timeinfo;         // unix time info
  char buffer[80];   // buffer to hold "Day Hour:Min"

  if(!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  strftime(buffer,80,"%a %I:%M",&timeinfo);
  display.setCursor(2,2);
  display.setTextColor(color);
  display.print(buffer);

  display_update_enable(true);
  yield();

}

void draw_date(uint16_t color)
{
  struct tm timeinfo; // unix time info
  char buffer[80];    // buffer to hold "Day Hour:Min"

  if(!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  strftime(buffer,80,"%b %d",&timeinfo);
  display.setCursor(20,20);
  display.setTextColor(color);
  display.print(buffer);

  display_update_enable(true);
  yield();

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
  display.setCursor(10,2);
  display.print("WiFi");
  display.setCursor(2,10);
  display.print("Connected!");
  delay(5000);

  display_update_enable(true);
  yield();
}

// --------- Sketch --------

void setup() {
  Serial.begin(115200);
  display.begin(16);  // 1:16 Scan Rate

  // slow update
  display.setFastUpdate(false);

  // delay 5 seconds after connecting 
  connect_wifi();

  // config to PST
  configTime(gmtOffset_sec * 4, daylightOffset_sec, ntpServer);
}

int color_index1 = 0;
int color_index2 = 4;
void loop() { 
  display.clearDisplay();
  draw_time(myCOLORS[color_index1]);
  draw_date(myCOLORS[color_index2]);
  delay(10000); // 10 seconds

  // change color
  color_index1 = (color_index1 + 1) % 7;
  color_index2 = (color_index2 + 1) % 7;
}
