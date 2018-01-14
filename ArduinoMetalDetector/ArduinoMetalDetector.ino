

// Number of cycles from external counter needed to generate a signal event
#define CYCLES_PER_SIGNAL 5000

// Base tone frequency (speaker)
#define BASE_TONE_FREQUENCY 280

// Frequency delta threshold for fancy spinner to trigger
#define SPINNER_THRESHOLD 700

// Pin definitions
#define SENSITIVITY_POT_APIN A1
#define SPEAKER_PIN 2
#define SPINNER_PIN 9
#define TRIGGER_BTN_PIN 11
#define RESET_BTN_PIN 12
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>

LiquidCrystal_PCF8574 lcd(0x27);  // set the LCD address to 0x27 for a 16 chars and 2 line display
const long fps=150;
unsigned long lastSignalTime = 0;
unsigned long signalTimeDelta = 0;
unsigned long currentime=0;
unsigned long passedmillis=0;
boolean firstSignal = true;
unsigned long storedTimeDelta = 0;

// This signal is called whenever OCR1A reaches 0
// (Note: OCR1A is decremented on every external clock cycle)
SIGNAL(TIMER1_COMPA_vect)
{
  unsigned long currentTime = micros();
  signalTimeDelta =  currentTime - lastSignalTime;
  lastSignalTime = currentTime;

  if (firstSignal)
  {
    firstSignal = false;
  }
  else if (storedTimeDelta == 0)
  {
    storedTimeDelta = signalTimeDelta;
  }

  // Reset OCR1A
  OCR1A += CYCLES_PER_SIGNAL;
}


void setuplcd()
{
  Wire.begin();
  Wire.beginTransmission(0x27);
  lcd.begin(16, 2);
  lcd.setBacklight(255);
  lcd.home(); lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Metal Detector");
  lcd.setCursor(0, 1);
  lcd.print("Version 0.1 Beta");
  delay(2000);  
  lcd.home(); lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("     Lambros        Triger  ");
  lcd.setCursor(0, 1);
  lcd.print(" Frantzeskakis      Start ");
  delay(2000);
  for(int i=0; i<14 ; i++)
  lcd.scrollDisplayLeft();
  
  delay(500);
} 
 
 
void displayer(int phace, float sens,int metal){
  currentime=millis();
  if (currentime - passedmillis>= fps) {
  passedmillis = currentime;
  lcd.home();lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Delta");
  lcd.setCursor(10,0);
  lcd.print(phace); //phace
  lcd.setCursor(0,1);
  lcd.print("Sens");
  lcd.setCursor(6,1);
  lcd.print(sens); //
  lcd.setCursor(10,1);
  lcd.print(" ");
  lcd.setCursor(11,1);
      switch (metal) {
        case 0:
          lcd.print("NO");
          break;
        case 100:
          lcd.print("UN");
          break;
        case 200:
          lcd.print("AL");
          break;
        case 300:
          lcd.print("Co");
          break;
        case 400:
          lcd.print("Br");
          break;
        case 500:
          lcd.print("Fe");
          break;
       default:
          lcd.print("WOW");
          break;
      }
  
  }
}

void setup()
{
  // Set WGM(Waveform Generation Mode) to 0 (Normal)
  TCCR1A = 0b00000000;
  
  // Set CSS(Clock Speed Selection) to 0b111 (External clock source on T0 pin
  // (ie, pin 5 on UNO). Clock on rising edge.)
  TCCR1B = 0b00000111;

  // Enable timer compare interrupt A (ie, SIGNAL(TIMER1_COMPA_VECT))
  TIMSK1 |= (1 << OCIE1A);

  // Set OCR1A (timer A counter) to 1 to trigger interrupt on next cycle
  OCR1A = 1;
  setuplcd();
  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(SPINNER_PIN, OUTPUT);
  pinMode(TRIGGER_BTN_PIN, INPUT_PULLUP);
  pinMode(RESET_BTN_PIN, INPUT_PULLUP);
}

void loop()
{
  int metal=0;
  if (digitalRead(TRIGGER_BTN_PIN) == LOW)
  {
    float sensitivity = mapFloat(analogRead(SENSITIVITY_POT_APIN), 0, 1023, 0.5, 10.0);
    int storedTimeDeltaDifference = (storedTimeDelta - signalTimeDelta) * sensitivity;
    tone(SPEAKER_PIN, BASE_TONE_FREQUENCY + storedTimeDeltaDifference);
    metal=(storedTimeDeltaDifference-SPINNER_THRESHOLD)*10;
    displayer(storedTimeDeltaDifference,sensitivity,metal);
    

    if (storedTimeDeltaDifference > SPINNER_THRESHOLD)
    {
     metal=(storedTimeDeltaDifference-SPINNER_THRESHOLD)*10;
     displayer(storedTimeDeltaDifference,sensitivity,metal);
     //digitalWrite(SPINNER_PIN, HIGH);
    }
    else
    {
      metal=0;
      displayer(storedTimeDeltaDifference,sensitivity,metal);
      digitalWrite(SPINNER_PIN, LOW);
    }
  }
  else
  {
    noTone(SPEAKER_PIN);
    digitalWrite(SPINNER_PIN, LOW);
  }

  if (digitalRead(RESET_BTN_PIN) == LOW)
  {
    storedTimeDelta = 0;
  }
}

float mapFloat(int input, int inMin, int inMax, float outMin, float outMax)
{
  float scale = (float)(input - inMin) / (inMax - inMin);
  return ((outMax - outMin) * scale) + outMin;
}

