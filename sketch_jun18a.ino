 
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Ustawienie adresu ukladu na 0x27
const byte pin = 2;

#define POLLS 5
#define WHEEL_RADIUS 2.27137

int revolutions[POLLS + 1];
volatile int ms = 1000;
volatile int sec = false;

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);   // Inicjalizacja LCD 2x16
  
  lcd.backlight(); // zalaczenie podwietlenia 
  lcd.setCursor(0,0); // Ustawienie kursora w pozycji 0,0 (pierwszy wiersz, pierwsza kolumna) 

  pinMode(pin, INPUT);
  attachInterrupt(digitalPinToInterrupt(2), handleChange, CHANGE);

  // Attaching to TIMER0
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
}
 
void loop() {

  static int seconds = 0;
  
  { // Must take less than a millisecond
    if (!sec) {
      return;
    }

    ++seconds;
    sec = false;
  }

  { // Must take less than a second
    shiftRevolutions();

    int aRPM = rpm();
    int aKPH = kph(aRPM);
    
    lcd.setCursor(0, 0);

    char msg[255];
    snprintf(msg, sizeof(msg), "C: %d RPM: %d      ", *revolutions, aRPM);
    lcd.print(msg);
    
    lcd.setCursor(0, 1);
    snprintf(msg, sizeof(msg), "T: %d KPH: %d      ", seconds, aKPH);
    lcd.print(msg);
  }

}

void shiftRevolutions() {
  for (int i=POLLS; i; --i) {
    revolutions[i] = revolutions[i-1];  
  }
}

int rpm() {
  return (int)((float)(revolutions[1] - revolutions[POLLS]) * 60.0 / (float)POLLS);
}

int kph(int rpm) {
  return (int)(WHEEL_RADIUS * (float)rpm * 60.0 / 1000.0);
}

void handleChange() {
  delay(10);
  if (!digitalRead(pin)) {
    ++*revolutions;
  }
}

// Interrupt is called once a millisecond, 
SIGNAL(TIMER0_COMPA_vect) {
  if (!--ms) {
    ms = 1000;
    sec = true;
  }
}
