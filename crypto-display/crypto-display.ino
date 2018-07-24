/*
Arman Ashrafian

Display ETH price & stats on LED matrix
*/

// dev board
#define ESP32

#include "secret.h"

#include <WiFi.h>
#include <WiFiClientSecure.h>

//------- External libraries -------
#include <ArduinoJson.h>
#include <CoinMarketCapApi.h>
#include <PxMatrix.h>

// Pins connected to LED Matrix
#define P_LAT 22
#define P_A 19
#define P_B 23
#define P_C 18
#define P_D 5
#define P_E 15
#define P_OE 2

// led matrix display driver
PxMATRIX display(64,32,P_LAT,P_OE,P_A,P_B,P_C,P_D);

// -------------- Some standard colors ----------------
uint16_t myRED = display.color565(255, 0, 0);
uint16_t myGREEN = display.color565(0, 255, 0);
uint16_t myBLUE = display.color565(0, 0, 255);
uint16_t myWHITE = display.color565(255, 255, 255);
uint16_t myYELLOW = display.color565(255, 255, 0);
uint16_t myCYAN = display.color565(0, 255, 255);
uint16_t myMAGENTA = display.color565(255, 0, 255);
uint16_t myCOLORS[7]={myRED,myGREEN,myBLUE,myWHITE,myYELLOW,myCYAN,myMAGENTA};

hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
uint8_t display_draw_time=30;

// Defined in secret.h
char ssid[] = WIFI_SSID;
char password[] = WIFI_PASSWORD;

WiFiClientSecure client;
CoinMarketCapApi api(client);

// CoinMarketCap's limit is "no more than 10 per minute"
unsigned long api_mtbs = 60000; //mean time between api requests
unsigned long api_due_time = 0;

// --------- Sketch ---------
void setup() {

  Serial.begin(115200);

  // Attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);
}

void printTickerData(String ticker) {
  Serial.println("---------------------------------");
  Serial.println("Getting ticker data for " + ticker);


  CMCTickerResponse response = api.GetTickerInfo(ticker, "usd");
  if (response.error == "") {
    Serial.println(response.symbol);
    Serial.println(response.price_usd);
    Serial.println(response.price_btc);
    Serial.print("%");
    Serial.println(response.percent_change_24h);
  } else {
    Serial.print("Error getting data: ");
    Serial.println(response.error);
  }
  Serial.println("---------------------------------");

  display.clearDisplay();
  display.setCursor(2,10);
  display.setTextColor(myCYAN);
  display.print(response.price_usd);

  display_update_enable(true);
  yield();
  delay(5000);
  display_update_enable(false);
}

void loop() {
  printTickerData("ethereum");
}

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