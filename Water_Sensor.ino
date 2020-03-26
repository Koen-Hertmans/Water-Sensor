#define BLYNK_PRINT Serial


#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "q3HJ6YAwB2B8yISyloqzaCK9Ui6wxNr7";

// Your WiFi credentials.
char ssid[] = "telenet-20389";
char pass[] = "rUwAnN9p2NSj"; // Set password to "" for open networks.

int sensorPin = 34; //analog pin watersensor
int waterlvl = 0;
const int buzzerPin = 16;  // pin buzzer
int buzzer = 1; //setup value to turn buzzer on and off, default on;

// setting PWM properties
int freq = 5000;
const int Channel = 0;
const int resolution = 8;

BlynkTimer timer; //make timer object
WidgetLCD lcd(V1); //declare lcdwidget

BLYNK_CONNECTED() { //send notification when hardware is connected to server also sends notification if hardware is disconnected(configured in app widget)
  Blynk.notify("Water sensor is connected");
}

BLYNK_WRITE(V2) //get value to change frequncy alarm
{
  freq = param.asInt(); // Get value as integer
  ledcSetup(Channel, freq, resolution); //setup pwm with differnt frequency
}

BLYNK_WRITE(V3) //get valu to turn alarm on or off
{
  buzzer = param.asInt(); // Get value as integer
}

void Sensordata() //pushes data to blynk app widgets
{
  Blynk.virtualWrite(V0, waterlvl); //writes the sensor value to the app

  //print the Water level on lcd in app
  if (waterlvl < 100) {
    lcd.print(0, 0, "Water Level:    ");
    lcd.print(0, 1, "LOW           ");
  }
  if (waterlvl >= 100 && waterlvl <= 1000) {
    lcd.print(0, 0, "Water Level:    ");
    lcd.print(0, 1, "MEDIUM          ");
  }
  if (waterlvl > 1000 && waterlvl < 2000) {
    lcd.print(0, 0, "Water Level:    ");
    lcd.print(0, 1, "HIGH           ");
  }
  if (waterlvl >= 2000) {
    lcd.print(0, 0, "FLOODING!!! ");
    lcd.print(0, 1, "                ");
  }
}
void Notificatians() { //Send email and notifycation if water level is rising
  if (waterlvl >= 1000) {
    Blynk.email("koen.hertmans@gmail.com", "Water Level", "Water level is rising!");
    Blynk.notify("Water level is rising!");
  }
}
void setup() {
  Serial.begin(9600);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Setup oled on address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  Blynk.begin(auth, ssid, pass); // connect to blynk server
  timer.setInterval(1000L, Sensordata); // setup timer for pushing data every second
  timer.setInterval(30000L, Notificatians); // setup timer for sending messages every 30sec
  lcd.clear(); //clear LCD widget on app

  // configure PWM functionalitites
  ledcSetup(Channel, freq, resolution);
  ledcAttachPin(buzzerPin, Channel);  // attach the channel to the GPIO to be controlled
}

void loop() {
  waterlvl = analogRead(sensorPin); //read value watersensor
  Serial.println(waterlvl);

  //Configure OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  if (waterlvl < 100) {
    display.print("Water Level: Laag");
    ledcWrite(Channel, 0);
  }
  if (waterlvl >= 100 && waterlvl <= 1000) {
    display.print("Water Level: Medium");
    ledcWrite(Channel, 0);
  }
  if (waterlvl > 1000 && waterlvl < 2000) {
    display.print("Water Level: Hoog");
    ledcWrite(Channel, 0);
  }
  if (waterlvl >= 2000) {
    display.print("OVERSTROMING!!!");
    if (buzzer == 1) {
      ledcWrite(Channel, 200); //Turn on Alarm
    }
  }

  if (buzzer == 0) {
    ledcWrite(Channel, 0); // Turn off alarm
  }
  display.display();
  Blynk.run(); 
  timer.run(); // Initiates BlynkTimer
}
