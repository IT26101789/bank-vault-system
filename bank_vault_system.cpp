#define BLYNK_TEMPLATE_ID "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "BANK VAULT"
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_TOKEN"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Keypad.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ================= WIFI =================
char ssid[] = "YOUR_WIFI_NAME";
char pass[] = "YOUR_WIFI_PASSWORD";

// ================= PINS =================

// PIR
#define PIR_PIN 27

// Buzzer
#define BUZZER_PIN 26

// LEDs
#define GREEN_LED 25
#define RED_LED 33

// Servo
#define SERVO_PIN 13

// ================= LCD =================
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ================= SERVO =================
Servo vaultServo;

// Door positions
#define LOCKED_POSITION 0
#define OPEN_POSITION 90

// ================= KEYPAD =================
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {19, 18, 5, 17};
byte colPins[COLS] = {16, 4, 2, 15};

Keypad keypad = Keypad(
  makeKeymap(keys),
  rowPins,
  colPins,
  ROWS,
  COLS
);
// ================= PASSWORD =================

String correctPassword = "1234";
String enteredPassword = "";

// ================= PIR VARIABLES =================

bool pirAlertSent = false;

// ================= SETUP =================

void setup()
{
  Serial.begin(115200);

  // Pins
  pinMode(PIR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);

  // LCD
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("BANK VAULT");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");
  delay(2000);

  // Servo
  vaultServo.attach(SERVO_PIN);
  vaultServo.write(LOCKED_POSITION);

  // Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("VAULT LOCKED");
  lcd.setCursor(0, 1);
  lcd.print("Enter Password");
}

// ================= LOOP =================

void loop()
{
  Blynk.run();

  checkPIR();
  checkKeypad();
}

// ================= PIR FUNCTION =================

void checkPIR()
{
  int pirState = digitalRead(PIR_PIN);

  if (pirState == HIGH)
  {
    Serial.println("Motion detected!");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("INTRUDER");
    lcd.setCursor(0, 1);
    lcd.print("DETECTED!");

    // Sound alarm
    tone(BUZZER_PIN, 2000);

    // Send Blynk alert only once
    if (!pirAlertSent)
    {
      Blynk.logEvent(
        "vault_intrusion",
        "ALERT: Motion detected inside vault room!"
      );

      pirAlertSent = true;
    }

    delay(1000);
    noTone(BUZZER_PIN);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("VAULT LOCKED");
    lcd.setCursor(0, 1);
    lcd.print("Enter Password");
  }
  else
  {
    pirAlertSent = false;
  }
}

// ================= KEYPAD FUNCTION =================

void checkKeypad()
{
  char key = keypad.getKey();

  if (key)
  {
    Serial.print("Key pressed: ");
    Serial.println(key);

    // Clear password
    if (key == '*')
    {
      enteredPassword = "";

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Enter Password");

      return;
    }

    // Submit password
    if (key == '#')
    {
      checkPassword();
      return;
    }

    // Add number to password
    if (enteredPassword.length() < 10)
    {
      enteredPassword += key;

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Password:");

      lcd.setCursor(0, 1);

      // Display * instead of password
      for (int i = 0; i < enteredPassword.length(); i++)
