
#include <Adafruit_SSD1306.h>

// screen

#define PIXEL_WIDTH 128
#define WIDTH_CENTER 64

#define PIXEL_HEIGHT 64
#define HEIGHT_CENTER 32

// hours

#define HOURS_POS_X 32
#define HOURS_POS_Y 32

#define CICLE_OUT_RADIUS 30
#define CICLE_IN_RADIUS 28

#define SECONDS_RADIUS 25
#define MINUTES_RADIUS 15
#define HOURS_RADIUS 10


// temp

#define TEMP_PIN 2
#define TEMP_HEIGHT 24
#define TEMP_WIDTH 77

// oled screen

#define PIN_RESET_OLED -1
#define ADDR_I2C 0x3C

#define TAILLE_POLICE 2

#define COLOR_WHITE SSD1306_WHITE
#define COLOR_BLACK SSD1306_BLACK

#define TIMESTAMP_BUFFER_LENGHT 4

#define DELAY 500

// text message display

#define TEXT_MAX_LENGHT 40

#define TEXT_DISPLAY_DURATION 3000

// light sensor

#define LIGHT_SENSOR_PIN 3

#define LIGHT_SENSIBILITY 716 // 70 * 1024 / 100


#include <SoftwareSerial.h>
#define rxPin 11 // Broche 11 en tant que RX, à raccorder sur TX du HC-05
#define txPin 10 // Broche 10 en tant que TX, à raccorder sur RX du HC-05


// global var

SoftwareSerial mySerial(rxPin, txPin);

Adafruit_SSD1306 ecranOLED(PIXEL_WIDTH, PIXEL_HEIGHT, & Wire, PIN_RESET_OLED);

char message[TEXT_MAX_LENGHT];

unsigned long  start_timestamp = 0;

unsigned short timestamp = 0;

unsigned short printMessageDeltaTime = 0;


// fuction

float fastCos(unsigned char val) {

  float cosArray[16] = {
    0.0, 0.10452846326765346, 0.20791169081775923, 0.30901699437494745, 0.4067366430758004, 0.5000000000000001, 0.5877852522924731, 0.6691306063588582, 0.7431448254773942, 0.8090169943749475, 0.8660254037844387, 0.9135454576426009, 0.9510565162951535, 0.9781476007338057, 0.9945218953682733, 1.0
  }; // calculated float for cos 

  if (val <= 15) {
    return cosArray[val];
  } 
  
  if (val <= 30) {
    return cosArray[30 - val];
  }
  
  if (val <= 45) {
    return -cosArray[val - 30];
  } 

  return  -cosArray[60 - val];
}

float fastSin(unsigned char val) {
  return fastCos((val + 15) % 60);
}

void showClockTime(void) {

  unsigned long tmp = millis() / 1000 - start_timestamp + timestamp; // timestamp in seconds

  unsigned char seconds_ch = tmp % 60;
  tmp /= 60;
  unsigned char minutes_ch = (tmp) % 60;
  tmp /= 60;
  unsigned char hours_ch = (tmp) % 24;



  ecranOLED.fillCircle(HOURS_POS_X, HOURS_POS_Y, CICLE_OUT_RADIUS, COLOR_WHITE); // clock circle
  ecranOLED.fillCircle(HOURS_POS_X, HOURS_POS_Y, CICLE_IN_RADIUS, COLOR_BLACK);

  unsigned char x, y; // position x and y

  x = HOURS_POS_X + fastCos(seconds_ch) * SECONDS_RADIUS;
  y = HOURS_POS_Y - fastSin(seconds_ch) * SECONDS_RADIUS;
  ecranOLED.drawLine(HOURS_POS_X, HOURS_POS_Y, x, y, COLOR_WHITE); // draw seconds

  x = HOURS_POS_X + fastCos(minutes_ch) * MINUTES_RADIUS;
  y = HOURS_POS_Y - fastSin(minutes_ch) * MINUTES_RADIUS;
  ecranOLED.drawLine(HOURS_POS_X, HOURS_POS_Y, x, y, COLOR_WHITE); // draw minutes


  hours_ch = (hours_ch % 12) * 5;
  x = HOURS_POS_X + fastCos(hours_ch) * HOURS_RADIUS;
  y = HOURS_POS_Y - fastSin(hours_ch) * HOURS_RADIUS;
  ecranOLED.drawLine(HOURS_POS_X, HOURS_POS_Y, x, y, COLOR_WHITE); // draw hours
  
}


void update_timestamp(void) {

  if (mySerial.available() > TIMESTAMP_BUFFER_LENGHT) {

  timestamp = 0;
  unsigned short val = 1000;

  for (int i = 0; i < TIMESTAMP_BUFFER_LENGHT; i++) {
     timestamp += val * (mySerial.read() - '0');  
     val /= 10;
  }

  timestamp *= 10;
  start_timestamp = (millis() / 1000);
  start_timestamp -= start_timestamp % 60;    
  }

}



void getMessage(void) {

  unsigned char letters = mySerial.available() - 1;
  

  if (letters <= 0) {
    return;
  } 

  if (letters > TEXT_MAX_LENGHT) {
    letters = TEXT_MAX_LENGHT;
  }

  

  for (int i = 0; i < letters; i++) {
    *(message + i) = (char) mySerial.read();
  } 

  if (mySerial.available() > 0) {

    for (int i = TEXT_MAX_LENGHT - 3; i < TEXT_MAX_LENGHT; i++) {

      *(message + i) = '.';
    }
    
    while (mySerial.available() > 0) {
      mySerial.read();  
    }  
  }
  

  *(message + letters) = 0;

  printMessageDeltaTime = TEXT_DISPLAY_DURATION;
}


void drawMessage(void) {

  if (printMessageDeltaTime > 0) {

    ecranOLED.setCursor(0, 0);
    ecranOLED.setTextColor(COLOR_BLACK, COLOR_WHITE);

    ecranOLED.print(message);
    
    printMessageDeltaTime -= DELAY;
  }
  
}


void show_temp() {

    float temp = analogRead(TEMP_PIN);
    temp *= 5000 / 1023;
    temp /= 10;
    temp /= 3.127;
    temp += 2;

    unsigned char final_temp = (int) temp % 99;

    ecranOLED.setCursor(TEMP_WIDTH, TEMP_HEIGHT);
    ecranOLED.setTextColor(COLOR_WHITE, COLOR_BLACK);
    ecranOLED.print(final_temp); 
    ecranOLED.print(" C");
    ecranOLED.drawCircle(105, 26, 2, COLOR_WHITE);
      
  }

void setScreenColor() {
  if (analogRead(LIGHT_SENSOR_PIN) < LIGHT_SENSIBILITY) {
    ecranOLED.invertDisplay(1);
  } else {
    ecranOLED.invertDisplay(0);
  }
}

// main functions

void setup() {

  while(!ecranOLED.begin(SSD1306_SWITCHCAPVCC, ADDR_I2C)); // loop if no screen

  ecranOLED.setTextSize(TAILLE_POLICE);    // define var for text
  ecranOLED.setTextColor(COLOR_WHITE, COLOR_BLACK);

  pinMode(LIGHT_SENSOR_PIN, INPUT);   
  pinMode(TEMP_PIN, INPUT);   
  
  pinMode(rxPin, INPUT);
  mySerial.begin(9600);
  pinMode(txPin, OUTPUT);
  Serial.begin(9600); // only for debug

}


void loop(void) {
  ecranOLED.clearDisplay();

  showClockTime();

  show_temp();


  if (mySerial.available()) {
    update_timestamp();
    getMessage();
  }

  drawMessage();

  setScreenColor();
  ecranOLED.display();

  delay(DELAY);
}
