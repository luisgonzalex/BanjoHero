#include <WiFi.h> //Connect to WiFi Network
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h> //Used in support of TFT Display
#include <mpu9255_esp32.h>

MPU9255 imu;
TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

#define start 0
#define leaderboard 1
#define posting 2

//BUTTONS
const uint8_t PIN_1 = 16; //button 1
const uint8_t PIN_2 = 5; //button 2

float accel_y;

//WIFI
char network[] = "MIT GUEST";
char password[] = "";

//GENERATING USERNAME
char alphabet[50] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
//char query_string[50] = {0};
int char_index;
uint32_t scrolling_timer;
const int scrolling_threshold = 300;
const float angle_threshold = 0.3;
const int LOOP_PERIOD = 40;
//USERNAME
char old_response[1000];
char response[1000];
char user[1000];

//HTTP-REQUESTS
const int RESPONSE_TIMEOUT = 6000; //ms to wait for response from host
const uint16_t IN_BUFFER_SIZE = 1000; //size of buffer to hold HTTP request
const uint16_t OUT_BUFFER_SIZE = 1000; //size of buffer to hold HTTP response
char request_buffer[IN_BUFFER_SIZE]; //char array buffer to hold HTTP request
char response_buffer[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP response

uint8_t state;
uint32_t score;
uint8_t percent;

class Button {
  public:
    uint32_t t_of_state_2;
    uint32_t t_of_button_change;
    uint32_t debounce_time;
    uint32_t long_press_time;
    uint8_t pin;
    uint8_t flag;
    bool button_pressed;
    uint8_t state; // This is public for the sake of convenience
    Button(int p) {
      flag = 0;
      state = 0;
      pin = p;
      t_of_state_2 = millis(); //init
      t_of_button_change = millis(); //init
      debounce_time = 10;
      long_press_time = 1000;
      button_pressed = 0;
    }
    void read() {
      uint8_t button_state = digitalRead(pin);
      button_pressed = !button_state;
    }
    int update() {
      read();
      flag = 0;
      if (state == 0) {
        if (button_pressed) {
          state = 1;
          t_of_button_change = millis();
        }
      } else if (state == 1) {
        if (button_pressed && millis() - t_of_button_change >= debounce_time) {
          state = 2;
          t_of_state_2 = millis();
        }
        else if (!button_pressed) {
          state = 0;
          t_of_button_change = millis();
        }
      } else if (state == 2) {
        if (!button_pressed) {
          state = 4;
          t_of_button_change = millis();
        }
        else if (millis() - t_of_state_2 >= long_press_time) {
          state = 3;
        }
      } else if (state == 3) {
        if (!button_pressed) {
          state = 4;
          t_of_button_change = millis();
        }
      } else if (state == 4) {
        if (!button_pressed && millis() - t_of_button_change >= debounce_time) {
          if (millis() - t_of_state_2 < long_press_time) {
            flag = 1;
          }
          else {
            flag = 2;
          }
          state = 0;
        }
        else if (button_pressed) {
          t_of_button_change = millis();
          if (millis() - t_of_state_2 < long_press_time) {
            state = 2;
          }
          else {
            state = 3;
          }
        }
      }
      return flag;
    }
};

void setup() {
  Serial.begin(115200);

  //INITIATE SCREEN
  tft.init();
  tft.setRotation(2); //adjust rotation
  tft.setTextSize(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN);

  //SET UP BUTTONS
  pinMode(PIN_1, INPUT_PULLUP);
  pinMode(PIN_2, INPUT_PULLUP);

  //CONNECTING TO WIFI
  WiFi.begin(network); //attempt to connect to wifi
  uint8_t count = 0; //count used for Wifi check times
  Serial.print("Attempting to connect to ");
  Serial.println(network);
  while (WiFi.status() != WL_CONNECTED && count < 12) {
    delay(500);
    Serial.print(".");
    count++;
  } delay(2000);
  if (WiFi.isConnected()) { //if we connected then print our IP, Mac, and SSID we're on
    Serial.println("CONNECTED!");
    Serial.println(WiFi.localIP().toString() + " (" + WiFi.macAddress() + ") (" + WiFi.SSID() + ")");
    delay(500);
  } else { //if we failed to connect just Try again.
    Serial.println("Failed to Connect :/  Going to restart");
    Serial.println(WiFi.status());
    ESP.restart(); // restart the ESP (proper way)
  }

  //SET UP IMU
  if (imu.setupIMU(1)) {
    Serial.println("IMU Connected!");
  } else {
    Serial.println("IMU Not Connected :/");
    Serial.println("Restarting");
    ESP.restart(); // restart the ESP (proper way)
  }

  main_menu();
}

uint8_t flag1;
uint8_t flag2;
Button b1(PIN_1);
Button b2(PIN_2);

void loop() {
  flag1 = b1.update();
  flag2 = b2.update();
  accel_y = -1 * imu.accelCount[1] * imu.aRes;

  switch (state) {
    case start:
      Serial.println("State: start");
      if (flag1 == 1) {
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0, 0, 1);
        tft.println("Posting score!");
        state = posting;
      } else if (flag2 == 1) {
        GET();
        show();
        state = leaderboard;
      }
      break;
    case posting:
      Serial.println("State: posting");
      score = 999999;
      percent = 100;
      memset(user, 0, sizeof(user));
      strcat(user, "tranf");
      POST();
      main_menu();
      break;
    case leaderboard:
      Serial.println("State: leaderboard");
      if (flag1 == 1) {
        main_menu();
      }
      break;
  }
}

void main_menu() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0, 1);
  tft.println("Press left to upload highscore");
  tft.println("Press right to view leaderboard");
  state = start;
}

void GET() {
  //val==alltime
  //val==personal.......song,user
  //val==song..........song
  sprintf(request_buffer, "GET http://608dev.net/sandbox/sc/marcof/finalproject/GuitarHeroServer.py?val=%s HTTP/1.1\r\n", "alltime");
  strcat(request_buffer, "Host: 608dev.net\r\n");
  strcat(request_buffer, "\r\n");
  do_http_request("608dev.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
}

void POST() {
  char body[200]; //for body;
  sprintf(body, "user=%s&score=%i&song=%s&percent=%i", user, score, "old town road", percent);
  int body_len = strlen(body); //calculate body length (for header reporting)
  sprintf(request_buffer, "POST http://608dev.net/sandbox/sc/marcof/finalproject/GuitarHeroServer.py HTTP/1.1\r\n");
  strcat(request_buffer, "Host: 608dev.net\r\n");
  strcat(request_buffer, "Content-Type: application/x-www-form-urlencoded\r\n");
  sprintf(request_buffer + strlen(request_buffer), "Content-Length: %d\r\n", body_len); //append string formatted to end of request buffer
  strcat(request_buffer, "\r\n"); //new line from header to body
  strcat(request_buffer, body); //body
  strcat(request_buffer, "\r\n"); //header
  Serial.println(request_buffer);
  do_http_request("608dev.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
}

void show() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0, 1);
  tft.println(response_buffer);
}
//void update(float angle, int button, int button2, char* output) {
//  if (button == 1) {
//    query_string[strlen(query_string)] = alphabet[char_index % 37];
//    char_index = 0;
//  } else if (button2 == 1) {
//    char_index = 0;
//    memset(user, 0, sizeof(user));
//    sprintf(user, "%s", query_string);
//    memset(output, 0, sizeof(output));
//    possibleredraw = true;
//    state = readyornot;
//  } else if ((millis() - scrolling_timer) >= scrolling_threshold && angle >= angle_threshold) {
//    char_index++;
//    scrolling_timer = millis();
//  } else if ((millis() - scrolling_timer) >= scrolling_threshold && angle <= -1 * angle_threshold) {
//    char_index += 36;
//    scrolling_timer = millis();
//  }
//  sprintf(output, "%s%c", query_string, alphabet[char_index % 37]);
//}
