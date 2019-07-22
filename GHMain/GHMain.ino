#include <Adafruit_DotStar.h>
#include <WiFi.h> //Connect to WiFi Network
#include <SPI.h>
#include <TFT_eSPI.h>
#include <mpu9255_esp32.h>
#include<math.h>
#include<string.h>
#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>

// led stuff
TFT_eSPI tft = TFT_eSPI();


#define NUMPIXELS 25

// Here's how to control the LEDs from any two pins:

#define DATAPIN1    14
#define CLOCKPIN1   12

#define DATAPIN2    26
#define CLOCKPIN2   27

#define DATAPIN3    19
#define CLOCKPIN3   25


// Speaker stuff
bool playing;
bool started;
bool checking;

bool playing1;
bool wait;

const uint32_t hang_time = 2500;
uint32_t waiting_timer;

// strtok stuff
const char s[20] = "&";
char *token;
char *token_temp;
char debug[400];

HardwareSerial mySoftwareSerial(2);
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);
int song_num;
const int numsongs = 2;
const char* song_list[] = {"birthday sex", "old town roads"};



Adafruit_DotStar strip(NUMPIXELS, DATAPIN1, CLOCKPIN1, DOTSTAR_RBG);
Adafruit_DotStar strip2(NUMPIXELS, DATAPIN2, CLOCKPIN2, DOTSTAR_RBG);
Adafruit_DotStar strip3(NUMPIXELS, DATAPIN3, CLOCKPIN3, DOTSTAR_RBG);


// light control and display variables, from light_tester.ino
int glow_list[NUMPIXELS + 1] = {0};
int glow_list2[NUMPIXELS + 1] = {0};
int glow_list3[NUMPIXELS + 1] = {0};
bool pressed;
bool pressed2;
bool pressed3;
bool going = false;
int indexer = 0;
uint32_t timerx;
int song_array[2400] = {0}; // size 2400 needed for a 4 minute song maximum
int song_array2[2400] = {0};
int song_array3[2400] = {0};
char song[2400];
char song2[2400];
char song3[2400];
uint32_t refresh_rate = 100;
char song_name[100];
char user[20];

// scoring variables here
int total;
int score;
float percent;

//state machine stuff
int state; //the state of the state machine...duh
#define MAIN 0
#define PLAY_USER 1
#define PLAY_SONG 2
#define PLAY_GAME 3
#define PLAY_PAUSE 4
#define PLAY_RESULTS 5
#define LEARN_SONG 6
#define LEARN_RECORD 7
#define LEARN_RESULTS 8
#define ACCEPT_RECORDING 9
#define SCORES 10
#define WAITING 11
uint32_t last_time;

//keyboard stuff
char alphabet[50] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
char message[400] = {0}; //contains previous query response
char user_string[50] = {0};
int char_index;
unsigned long scrolling_timer;
const int scrolling_threshold = 150;
const float angle_threshold = 0.3;


// buttons
const uint8_t PIN_1 = 16; //button 1
const uint8_t PIN_2 = 5; // button 2
const uint8_t PIN_3 = 17; // button 3
const uint8_t PIN_4 = 13; // button 4

// tft screen
char output[400]; //hold message for tft
char old_output[400]; //hold old message for tft

// imu stuff
MPU9255 imu; //imu object called, appropriately, imu


// wifi stuff
char network[] = "MIT";   // your network SSID (name of wifi network)
char password[] = ""; // your network password

// server side stuff
const int RESPONSE_TIMEOUT = 6000; //ms to wait for response from host
const int GETTING_PERIOD = 60000; //periodicity of getting time from server in ms
const uint16_t IN_BUFFER_SIZE = 10000; //size of buffer to hold HTTP request
const uint16_t OUT_BUFFER_SIZE = 10000; //size of buffer to hold HTTP response
char request_buffer[IN_BUFFER_SIZE]; //char array buffer to hold HTTP request
char response[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP request
char response_buffer[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP response


// main functionality
void guitar_hero_sm(int bv1, int bv2, int bv3, int bv4) {
  switch (state) {
    case MAIN:
      // idle state
      sprintf(output, "Welcome to Guitar Hero!\nHere's how to navigate\n Button 1: Play\n Button 2: Record\n Button 3: Leaderboards");
      if (bv1 == 1 or bv1 == 2) {
        state = PLAY_USER;
      }
      if (bv2 == 1 or bv2 == 2) {
        state = LEARN_SONG;
      }
      if (bv3 == 1 or bv3 == 2) {
        state = SCORES;
      }
      break;

    case PLAY_USER:
      // have the user input their username
      sprintf(output, "Tilt to type!\n Button 1: Accept letter\n Button 2: Done\n Button 3: Main Menu\n");
      float x, y;
      get_angle(&x, &y); //get angle values
      keyboard(-y, bv1, message);
      sprintf(output, "%s%s", output, message);
      if (bv2 == 1 or bv2 == 2) {
        state = PLAY_SONG;
      }
      if (bv3 == 1 or bv3 == 2) {
        state = MAIN;
      }
      break;
    case PLAY_SONG:
      // have the user choose a song
      sprintf(song_name, "%s", song_list[song_num % numsongs]);
      sprintf(output, "Choose a song:\n%s", song_name);
      if (bv4 == 1 or bv4 == 2) {
        GET_song();
        playing1 = true;
        score = 0;
        total = 0;
        percent = 0.0;
        wait = true;
        state = WAITING;
        going = true;
        waiting_timer = millis();
        song_num = 0;
      }
      if (bv1 == 1 or bv1 == 2) {
        song_num++;
        if (started) {
          Serial.println("started");
          next();
          playpause();
        }
        else {
          Serial.println("not started");
          next();
          next();
          playpause();
        }

      }
      if (bv2 == 1 or bv2 == 2) {
        song_num++;
        song_num = song_num % numsongs;
        if (!started) {
          prev();
          playpause();

        }
      }
      break;
    case WAITING:
      if (millis() - waiting_timer >= hang_time) {
        playpause();
        Serial.println(song);
        Serial.println(song2);
        Serial.println(song3);
        state = PLAY_GAME;
      }
      break;
    case PLAY_GAME:
      // actual gameplay of the song, showing the lights and such and evaluating button presses
      // should play differently depending on if coming from play pause or play song
      // NOTE: need to play the lights (amount lights * ms per shift) for this amount of time before the actual song for the song is called.
      // == 25 * 100ms = 2500 ms in advance
      if (playing1 == false) {
        state = PLAY_RESULTS;
        playpause();
        percent = (float) score / (float) total;
      }
      else {
        playing_song();
      }
      break;
    case PLAY_PAUSE:
      // pause the game, so freeze the reading of the system (no reading here) until wanting to start again at play_game
      break;
    case PLAY_RESULTS:
      //
      // show the score post to database, wait for button press to go back into menu

      sprintf(output, "Here's the score: %d\nHere's percent: %f\nButton 4 to main.", score, percent);

      if (bv4 == 1 or bv4 == 2) {
        state = MAIN;
      }

      break;
    case LEARN_SONG:
      // choose the song title that you're learning for, similar to play_song
      sprintf(song_name, "%s", song_list[song_num % numsongs]);
      sprintf(output, "Choose a song:\n%s", song_name);
      if (bv4 == 1 or bv4 == 2) {
        state = LEARN_RECORD;
        going = true;
        song_num = 0;
        playpause();
      }
      if (bv1 == 1 or bv1 == 2) {
        song_num++;
        if (started) {
          Serial.println("started");
          next();
          playpause();
        }
        else {
          Serial.println("not started");
          next();
          next();
          playpause();
        }

      }
      if (bv2 == 1 or bv2 == 2) {
        song_num++;
        song_num = song_num % numsongs;
        if (!started) {
          prev();
          playpause();

        }
      }
      break;
    case LEARN_RECORD:
      // should also add the concurrent song playing with this. when song ends, going becomes
      // false. should we also implement a way for the user to re-do it?
      if (bv4 == 1 or bv4 == 2) {
        going = false;
        state = LEARN_RESULTS;
      }
      if (going) {
        record_song();
      }
      break;
    case LEARN_RESULTS:
      // send the song array to the server (would be a shorter form of the playback function)
      // following the send, set indexer, glow_lists 1/2/3, song_array all back to default
      char temp[2];
      char temp2[2];
      char temp3[2];
      for (int i = 0; i < indexer - 1; i++) {
        sprintf(temp, "%d", song_array[i]);
        sprintf(temp2, "%d", song_array2[i]);
        sprintf(temp3, "%d", song_array3[i]);
        strcat(song, temp);
        strcat(song2, temp2);
        strcat(song3, temp3);
        song_array[i] = 0;
        song_array2[i] = 0;
        song_array3[i] = 0;
      }
      for (int i = 0; i < NUMPIXELS + 1; i++) {
        glow_list[i] = 0;
        glow_list2[i] = 0;
        glow_list3[i] = 0;
        strip.setPixelColor(i, 0, 0, 0);
        strip2.setPixelColor(i, 0, 0, 0);
        strip3.setPixelColor(i, 0, 0, 0);
      }

      strip.show();
      strip2.show();
      strip3.show();

      //send to database here!!

      indexer = 0;
      state = ACCEPT_RECORDING;
      playpause();
      break;

    case ACCEPT_RECORDING:
      sprintf(output, "Press button 1 to accept recording, button 2 to reject");
      if (bv1 == 1 or bv1 == 2) {
        //send to database here!!
        POST_song();
        state = MAIN;
        memset(song, 0, sizeof(song));
        memset(song2, 0, sizeof(song2));
        memset(song3, 0, sizeof(song3));
      }
      else if (bv2 == 1 or bv2 == 2) {
        state = MAIN;
        memset(song, 0, sizeof(song));
        memset(song2, 0, sizeof(song2));
        memset(song3, 0, sizeof(song3));
      }
      break;
    case SCORES:
      break;

  }

}

// helper functions

//used to get x,y values from IMU accelerometer!
void get_angle(float* x, float* y) {
  imu.readAccelData(imu.accelCount);
  *x = imu.accelCount[0] * imu.aRes;
  *y = imu.accelCount[1] * imu.aRes;
}

//our implementation of a keyboard
void keyboard(float angle, int button, char* output) {
  char new_letter[2] = {alphabet[char_index], 0};
  sprintf(message, "%s%s", user_string, new_letter);
  if (button == 1) {
    strcat(user_string, new_letter);
    sprintf(message, "%s", user_string);
    char_index = 0;
  }
  else if (millis() - scrolling_timer >= scrolling_threshold) {
    if (angle >= angle_threshold) {
      char_index ++;
      if (char_index > 36) {
        char_index = 0;
      }
      scrolling_timer = millis();
    }
    else if (angle <= -angle_threshold) {
      char_index--;
      if (char_index < 0) {
        char_index = 36;
      }
      scrolling_timer = millis();
    }
  }
}

// helper functions for playing game
void playing_song() {
  if (indexer == strlen(song)) {
    checking = false;
  }
  else if (indexer == strlen(song) + NUMPIXELS) {
    playing1 = false;
  }
  if (millis() - last_time >= refresh_rate) {

    //evaluate button press prior to value changes

    if (glow_list[0] == 1) {
      if (not(digitalRead(PIN_1))) {
        score += 1;
      }
      total += 1;
    }
    if (glow_list2[0] == 1) {
      if (not(digitalRead(PIN_2))) {
        score += 1;
      }
      total += 1;
    }
    if (glow_list3[0] == 1) {
      if (not(digitalRead(PIN_3))) {
        score += 1;

      }
      total += 1;
    }

    glow_list[0] = 0;
    glow_list2[0] = 0;
    glow_list3[0] = 0;
    for (int i = 0; i < NUMPIXELS; i++) {
      glow_list[i] = glow_list[i + 1];
      glow_list2[i] = glow_list2[i + 1];
      glow_list3[i] = glow_list3[i + 1];
    }

    if (checking) {

      if (song[indexer] == '1') {
        pressed = true;
      }
      else {
        pressed = false;
      }
      if (song2[indexer] == '1') {
        pressed2 = true;
      }
      else {
        pressed2 = false;
      }
      if (song3[indexer] == '1') {
        pressed3 = true;
      }
      else {
        pressed3 = false;
      }

      if (pressed) {
        glow_list[NUMPIXELS] = 1;
      }
      else {
        glow_list[NUMPIXELS] = 0;
      }
      if (pressed2) {
        glow_list2[NUMPIXELS] = 1;
      }
      else {
        glow_list2[NUMPIXELS] = 0;
      }
      if (pressed3) {
        glow_list3[NUMPIXELS] = 1;
      }
      else {
        glow_list3[NUMPIXELS] = 0;
      }

    }

    for (int i = 0; i < NUMPIXELS; i++) {
      if (glow_list[i] == 1) {
        strip.setPixelColor(i, 255, 0, 0);
      }
      else {
        strip.setPixelColor(i, 0);
      }
      if (glow_list2[i] == 1) {
        strip2.setPixelColor(i, 0, 255, 0);
      }
      else {
        strip2.setPixelColor(i, 0);
      }
      if (glow_list3[i] == 1) {
        strip3.setPixelColor(i, 0, 0, 255);
      }
      else {
        strip3.setPixelColor(i, 0);
      }
    }
    strip.show();
    strip2.show();
    strip3.show();
    last_time = millis(); // Refresh strip
    indexer = indexer + 1;
  }
}



//speaker stuff helper functions

void playsong() {
  Serial.println(F("myDFPlayer.play(1)"));
  myDFPlayer.play(1);  //Play the first mp3
  started = true;
  playing = true;
}

void playpause() {
  if (playing) {
    Serial.println(F("myDFPlayer.pause()"));
    myDFPlayer.pause();  //pause the mp3
    playing = false;
  }
  else {
    Serial.println(F("myDFPlayer.start()"));
    myDFPlayer.start();  //start the mp3 from the pause
    playing = true;
  }

}

void volumeup() {
  myDFPlayer.volumeUp(); //Volume Up
}

void volumedown() {
  myDFPlayer.volumeDown(); //Volume Down
}

void next() {
  started = true;
  playing = true;
  Serial.println(F("myDFPlayer.next()"));
  myDFPlayer.next();  //Play next mp3
}

void prev() {
  started = true;
  playing = true;
  Serial.println(F("myDFPlayer.previous()"));
  myDFPlayer.previous();  //Play previous mp3
}


void record_song() {

  if (millis() - last_time >= refresh_rate) {
    if (not(digitalRead(PIN_1))) {
      pressed = true;
    }
    else {
      pressed = false;
    }
    if (not(digitalRead(PIN_2))) {
      pressed2 = true;
    }
    else {
      pressed2 = false;
    }
    if (not(digitalRead(PIN_3))) {
      pressed3 = true;
    }
    else {
      pressed3 = false;
    }
    glow_list[0] = 0;
    glow_list2[0] = 0;
    glow_list3[0] = 0;
    for (int i = 0; i < NUMPIXELS; i++) {
      glow_list[i] = glow_list[i + 1];
      glow_list2[i] = glow_list2[i + 1];
      glow_list3[i] = glow_list3[i + 1];
    }
    if (pressed) {
      song_array[indexer] = 1;
      glow_list[NUMPIXELS] = 1;
    }
    else {
      song_array[indexer] = 0;
      glow_list[NUMPIXELS] = 0;
    }
    if (pressed2) {
      song_array2[indexer] = 1;
      glow_list2[NUMPIXELS] = 1;
    }
    else {
      song_array2[indexer] = 0;
      glow_list2[NUMPIXELS] = 0;
    }
    if (pressed3) {
      song_array3[indexer] = 1;
      glow_list3[NUMPIXELS] = 1;
    }
    else {
      song_array3[indexer] = 0;
      glow_list3[NUMPIXELS] = 0;
    }
    for (int i = 0; i < NUMPIXELS; i++) {
      if (glow_list[i] == 1) {
        strip.setPixelColor(i, 255, 0, 0);
      }
      else {
        strip.setPixelColor(i, 0);
      }
      if (glow_list2[i] == 1) {
        strip2.setPixelColor(i, 0, 255, 0);
      }
      else {
        strip2.setPixelColor(i, 0);
      }
      if (glow_list3[i] == 1) {
        strip3.setPixelColor(i, 0, 0, 255);
      }
      else {
        strip3.setPixelColor(i, 0);
      }
    }
    strip.show();
    strip2.show();
    strip3.show();
    last_time = millis(); // Refresh strip
    indexer = indexer + 1;
  }
}

// Button class for traversing state machine
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
          t_of_button_change = millis();
          state = 1;
        }
      }
      else if (state == 1) {
        if (button_pressed and millis() - t_of_button_change >= debounce_time) {
          state = 2;
          t_of_state_2 = millis();

        }
        else if (!button_pressed) {
          state = 0;
          t_of_button_change = millis();
        }
      } else if (state == 2) {
        if (button_pressed and millis() - t_of_state_2 >= long_press_time) {
          state = 3;
        }
        else if (!button_pressed) {
          state = 4;
          t_of_button_change = millis();
        }
      } else if (state == 3) {
        if (!button_pressed) {
          state = 4;
          t_of_button_change = millis();
        }
      } else if (state == 4) {
        // CODE HERE
        if (button_pressed and millis() - t_of_state_2 >= long_press_time) {
          state = 3;
          t_of_button_change = millis();
        }
        else if (button_pressed and millis() - t_of_state_2 < long_press_time) {
          state = 2;
          t_of_button_change = millis();
        }
        else if (!button_pressed and millis() - t_of_button_change >= debounce_time) {
          if (millis() - t_of_state_2 >= long_press_time) {
            flag = 2;
          }
          else {
            flag = 1;
          }
          state = 0;
        }
      }
      return flag;
    }
};


Button button1(PIN_1); //button object!
Button button2(PIN_2); //button object!
Button button3(PIN_3); //button object!
Button button4(PIN_4); //button object!


void setup() {
  // put your setup code here, to run once:
  delay(3000); // 3 second delay for recovery
  started = false;
  song_num = 0;
  mySoftwareSerial.begin(9600, SERIAL_8N1, 32, 33);  // speed, type, RX, TX
  Serial.begin(115200); //for debugging if needed.
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  delay(1000);
  if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.

    Serial.println(myDFPlayer.readType(), HEX);
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while (true);
  }
  Serial.println(F("DFPlayer Mini online."));

  myDFPlayer.setTimeOut(500); //Set serial communictaion time out 500ms

  //----Set volume----
  myDFPlayer.volume(20);  //Set volume value (0~30).
  myDFPlayer.volumeUp(); //Volume Up
  myDFPlayer.volumeDown(); //Volume Down

  //----Set different EQ----
  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
  //----Set device we use SD as default----
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
  int delayms = 100;
  WiFi.begin(network, password); //attempt to connect to wifi
  uint8_t count = 0; //count used for Wifi check times
  Serial.print("Attempting to connect to ");
  Serial.println(network);
  while (WiFi.status() != WL_CONNECTED && count < 12) {
    delay(500);
    Serial.print(".");
    count++;
  }
  delay(2000);
  if (WiFi.isConnected()) { //if we connected then print our IP, Mac, and SSID we're on
    Serial.println("CONNECTED!");
    Serial.println(WiFi.localIP().toString() + " (" + WiFi.macAddress() + ") (" + WiFi.SSID() + ")");
    delay(500);
  } else { //if we failed to connect just Try again.
    Serial.println("Failed to Connect :/  Going to restart");
    Serial.println(WiFi.status());
    ESP.restart(); // restart the ESP (proper way)
  }
  if (imu.setupIMU(1)) {
    Serial.println("IMU Connected!");
  } else {
    Serial.println("IMU Not Connected :/");
    Serial.println("Restarting");
    ESP.restart(); // restart the ESP (proper way)
  }
  pinMode(PIN_1, INPUT_PULLUP);
  pinMode(PIN_2, INPUT_PULLUP);
  pinMode(PIN_3, INPUT_PULLUP);
  pinMode(PIN_4, INPUT_PULLUP);
  tft.init();  //init screen
  tft.setRotation(2); //adjust rotation
  tft.setTextSize(1); //default font size
  tft.fillScreen(TFT_BLACK); //fill background
  tft.setTextColor(TFT_WHITE, TFT_BLACK); //set color for font

#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000L)
  clock_prescale_set(clock_div_1); // Enable 16 MHz on Trinket
#endif

  strip.setBrightness(10);
  strip2.setBrightness(10);
  strip3.setBrightness(10);

  strip.begin(); // Initialize pins for output
  strip.clear();
  strip.show();  // Turn all LEDs off ASAP

  strip2.begin(); // Initialize pins for output
  strip2.clear();
  strip2.show();  // Turn all LEDs off ASAP

  strip3.begin(); // Initialize pins for output
  strip3.clear();
  strip3.show();  // Turn all LEDs off ASAP

  last_time = millis();
  delay(100);
}

void loop() {
  // put your main code here, to run repeatedly:
  //sprintf(debug,"output: %s\nold_output: %s", output, old_output);
  //  sprintf(debug, "debug: %s", song_name);
  int bv1 = button1.update(); //get button value
  int bv2 = button2.update(); //get button value
  int bv3 = button3.update(); //get button value
  int bv4 = button4.update();
  guitar_hero_sm(bv1, bv2, bv3, bv4); // call the state machine with bval
  if (strcmp(output, old_output) != 0) {//only draw if changed!
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0, 1);
    tft.println(output);
  }
  memset(old_output, 0, sizeof(old_output));
  strcat(old_output, output);
}


void parser() {
  int counter = 0;
  /* get the first token */
  token = strtok(response_buffer, s);
  /* walk through other tokens */
  char* temp;
  while ( token != NULL ) {
    temp = token;
    if (counter == 0) {
      strcpy(song2, temp);
    }
    if (counter == 1) {
      strcpy(song3, temp);
    }
    if (counter == 2) {
      strcpy(song, temp);
    }
    token = strtok(NULL, s);
    counter++;
  }
}

void POST_score() {
  char body[200]; //for body;
  sprintf(body, "user=%s&score=%i&song=%s&percent=%i", user, score, song_name, percent);
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

void POST_song() {
  char body[5000]; //for body;
  sprintf(body, "red=%s&green=%s&blue=%s&title=%s", song2, song3, song, song_name);
  int body_len = strlen(body); //calculate body length (for header reporting)
  sprintf(request_buffer, "POST http://608dev.net/sandbox/sc/marcof/finalproject/gh_song_db.py HTTP/1.1\r\n");
  strcat(request_buffer, "Host: 608dev.net\r\n");
  strcat(request_buffer, "Content-Type: application/x-www-form-urlencoded\r\n");
  sprintf(request_buffer + strlen(request_buffer), "Content-Length: %d\r\n", body_len); //append string formatted to end of request buffer
  strcat(request_buffer, "\r\n"); //new line from header to body
  strcat(request_buffer, body); //body
  strcat(request_buffer, "\r\n"); //header
  Serial.println(request_buffer);
  do_http_request("608dev.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
}

void GET_song() {
  //val==alltime
  //val==personal.......song,user
  //val==song..........song
  sprintf(request_buffer, "GET http://608dev.net/sandbox/sc/marcof/finalproject/gh_song_db.py?title=%s HTTP/1.1\r\n", song_name);
  strcat(request_buffer, "Host: 608dev.net\r\n");
  strcat(request_buffer, "\r\n");
  do_http_request("608dev.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
  parser();
}


void GET_leaderboard() {
  //val==alltime
  //val==personal.......song,user
  //val==song..........song
  sprintf(request_buffer, "GET http://608dev.net/sandbox/sc/marcof/finalproject/GuitarHeroServer.py?val=%s HTTP/1.1\r\n", "alltime");
  strcat(request_buffer, "Host: 608dev.net\r\n");
  strcat(request_buffer, "\r\n");
  do_http_request("608dev.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
}
