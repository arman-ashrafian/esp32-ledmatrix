// Arman Ashrafian
// ESP32 LED Matrix Program

#define ESP32
#include <PxMatrix.h>

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
uint8_t display_draw_time=2;

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
uint16_t myBLACK = display.color565(0, 0, 0);
uint16_t myCOLORS[8]={myRED,myGREEN,myBLUE,myWHITE,myYELLOW,myCYAN,myMAGENTA,myBLACK};

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

// draws grid like dis
// |
// |_________
void draw_grid(uint16_t color) {
  display.drawLine(0,31,63,31,color); // x-axis
  display.drawLine(0,0,0,32,color);   // y-axis

  display_update_enable(true);
  yield();
}

void draw_name(uint16_t color) {
  display.setTextColor(myBLUE);
  display.setTextSize(1);
  display.setCursor(5,2);

  display.print(firstname);
  delay(1000);
  display.write(' ');
  display.print(lastname);
  delay(1000);

  display_update_enable(true);
  yield();

}

void setup() {
  Serial.begin(9600);
  display.begin(16);  // 1:16 Scan Rate

  // slow update
  display.setFastUpdate(false);
}

void loop() { 
  draw_grid(myCYAN);
  delay(1000);
  display.clearDisplay();
  delay(1000);
  draw_name(myRED);

}
