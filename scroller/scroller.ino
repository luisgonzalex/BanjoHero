#include <Adafruit_DotStar.h>
#include <SPI.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();


#define NUMPIXELS 25

// Here's how to control the LEDs from any two pins:
#define DATAPIN1    33
#define CLOCKPIN1   25

#define DATAPIN2    26
#define CLOCKPIN2   27

#define DATAPIN3    14
#define CLOCKPIN3   12

Adafruit_DotStar strip1(NUMPIXELS, DATAPIN1, CLOCKPIN1, DOTSTAR_RBG);

Adafruit_DotStar strip2(NUMPIXELS, DATAPIN2, CLOCKPIN2, DOTSTAR_RBG);

Adafruit_DotStar strip3(NUMPIXELS, DATAPIN3, CLOCKPIN3, DOTSTAR_RBG);

uint32_t last_time;

int led_pos = NUMPIXELS; //This number will indicate which led is 'on'
int old_pos; //Keeps track of the prev position, turns it off
uint32_t color = 0xFF0100;      // 'On' color (starts red)

uint32_t refresh_rate = 100; //200 ms refresh reate

const uint8_t PIN_1 = 16; //button 1

int score; // player's score
char output[50]; //hold message for tft
char old_output[50]; //hold old message for tft

void setup() {
  delay(3000); // 3 second delay for recovery
  Serial.begin(115200); //for debugging if needed.
  pinMode(PIN_1, INPUT_PULLUP);

  tft.init();  //init screen
  tft.setRotation(2); //adjust rotation
  tft.setTextSize(3); //default font size
  tft.fillScreen(TFT_WHITE); //fill background
  tft.setTextColor(TFT_BLACK, TFT_WHITE); //set color for font

#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000L)
  clock_prescale_set(clock_div_1); // Enable 16 MHz on Trinket
#endif
  strip1.setBrightness(10);
  strip2.setBrightness(10);
  strip3.setBrightness(10);

  strip1.begin(); // Initialize pins for output
  strip1.clear();
  strip1.show();  // Turn all LEDs off ASAP

  strip2.begin(); // Initialize pins for output
  strip2.clear();
  strip2.show();  // Turn all LEDs off ASAP

  strip3.begin(); // Initialize pins for output
  strip3.clear();
  strip3.show();  // Turn all LEDs off ASAP

  last_time = millis();
  delay(100);
}




void loop()
{
  scroller();

  if (strcmp(output, old_output) != 0) {//only draw if changed!
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0, 1);
    tft.println(output);
  }
  memset(old_output, 0, sizeof(old_output));
  strcat(old_output, output);
}


void scroller() {
  if (millis() - last_time >= refresh_rate) {
    if (led_pos < 0) {
      if (digitalRead(PIN_1) == 0) {
        score += 2;
      }
      led_pos = NUMPIXELS;
    }
    if (old_pos < 0) {
      if (digitalRead(PIN_1) == 0) {
        score ++;
      }
      old_pos = NUMPIXELS;
    }
    old_pos = led_pos;
    led_pos--;
    strip1.setPixelColor(led_pos, color); // 'On' pixel at head
    strip1.setPixelColor(old_pos, 0);     // 'Off' pixel at tail

    strip2.setPixelColor(led_pos, color); // 'On' pixel at head
    strip2.setPixelColor(old_pos, 0);     // 'Off' pixel at tail

    strip3.setPixelColor(led_pos, color); // 'On' pixel at head
    strip3.setPixelColor(old_pos, 0);     // 'Off' pixel at tail

    strip1.show();                     // Refresh strip
    strip2.show();                     // Refresh strip
    strip3.show();                     // Refresh strip
    last_time = millis();
  }
  sprintf(output, "Score: %d", score);
}

void button_press() {
  if (digitalRead(PIN_1) == 0) {
    Serial.println(led_pos);
    if (led_pos == NUMPIXELS) {
      score += 2;
    }
    else if (old_pos == NUMPIXELS) {
      score ++;
    }
    sprintf(output, "Score: %d", score);
  }
}
