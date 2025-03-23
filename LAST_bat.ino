#include <FS.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include "Free_Fonts.h"
#include <Keypad.h>
#include <Arduino.h>

// After other variable declarations
enum TouchState {
    STATE_IDLE,
    STATE_CONFIRM_WAIT,
    STATE_AUTO_PROCESS,
    STATE_RESET_PROCESS,
    STATE_MANUAL_SAVE,
    STATE_PHASE_START
};

TouchState currentState = STATE_IDLE;
unsigned long stateStartTime = 0;

TFT_eSPI tft = TFT_eSPI();

// Touch calibration
#define CALIBRATION_FILE "/TouchCalData1"
// Calibration Repeat
#define REPEAT_CAL false

uint16_t x = 0, y = 0;
int textWidth = 60;
int textHeight = 28;
#define offset_row 20
#define offset_col 0.5
#define LINE tft.setCursor(offset_col, offset_row + tft.getCursorY())

char homeKeyLabel[4][9] = {"MANUAL A", "AUTO B", "PHASE C", "RESET D"};

#define CUSTOM_PURPLE tft.color565(255 - 244, 255 - 92, 255 - 250)
#define CUSTOM_LIGHT_PINK tft.color565(255 - 92, 255 - 250, 255 - 122)
#define CUSTOM_LIGHT_RED tft.color565(255 - 79, 255 - 87, 255 - 249)
#define CUSTOM_LIGHT_BLUE tft.color565(255 - 248, 255 - 79, 255 - 79)

// Relay pin
#define relay1 15
#define relay2 4

uint16_t homeKeyColor[4] = {CUSTOM_PURPLE, CUSTOM_LIGHT_PINK, CUSTOM_LIGHT_RED, CUSTOM_LIGHT_BLUE};

// home key instances
TFT_eSPI_Button homeKey[4];

// manual keypad key instance
TFT_eSPI_Button manualKeypadKey[16];
char* manualKeypadKeyLabel[16] = {"1", "2", "3", "U",
                                  "4", "5", "6", "D",
                                  "7", "8", "9", "<-",
                                  ".", "0", ":", "OK"};

// manual key instance
TFT_eSPI_Button manualKey[4];
char* manualKeyLabel[4] = {"HOME A", "START B", "<-- C", "RESET"};

// manual phase key instance
TFT_eSPI_Button manualPhaseKey[4];
char* manualPhaseKeyLabel[4] = {"YELLOW", "LAMINA", "COLOR F.", "STEAM"};

// phase key instance
TFT_eSPI_Button phaseKey[4];
char* phaseKeyLabel[6] = {"YELLOWING 1", "LAMINA 2", "COLOR FIXING 3", "STEAM 4", "START A", "HOME B"};
uint16_t phaseKeyColor[6] = {tft.color565(255, 0, 255), tft.color565(255, 0, 255),
                            tft.color565(255, 0, 255), tft.color565(255, 0, 255),
                            CUSTOM_PURPLE, CUSTOM_LIGHT_RED};


// Confirm Key instance
TFT_eSPI_Button confirmKey[2];

// Button Touch variables
uint16_t t_x = 0, t_y = 0;


// Field color controller variable
byte selected_manual_field = 1;


// manual field strings
String manual_dry_temp_str = "";
String manual_wet_temp_str = "";
String manual_time_str = "";

// Phase time value string
String phase_time_value_str = "";

// home page time strings
String home_cur_time_str = "00:00:00";
String home_limit_time_str = "00:00:00";

char* phase_label[5] = {"NO PHASE", "YELLOWING", "LAMINA", "COLOR FIX.", "STEAM"};
char* mode_label[4] = {"NO MODE", "AUTO", "PHASE MODE", "MANUAL"};


// Phase parameters
const byte phase_temp[4][14][2] = {{{95, 92}, {96, 93}, {98, 94}, {99, 95}, {100, 96}},
                              {{100, 96}, {102, 96}, {104, 97}, {106, 97}, {108, 98}, {110, 98}, {112, 98}, {114, 99}, {116, 99}, {118, 100}, {120, 100}},
                              {{120, 100}, {122, 100}, {124, 101}, {126, 101}, {128, 102}, {130, 102}, {132, 102}, {134, 103}, {136, 103}, {138, 104}, {140, 104}, {142, 104}, {144, 105}, {145, 105}},
                              {{145, 105}, {147, 106}, {151, 107}, {153, 107}, {155, 108}, {157, 108}, {159, 109}, {161, 109}, {163, 110}, {163, 110}, {165, 110}}};



const byte phase_temp2[4][40][2] = {{{95, 92}, {96, 93}, {98, 94}, {99, 95}, {100, 96}, {100, 96}, {100, 96}, {100, 96}, {100, 96}, {100, 96}, {100, 96}, {100, 96}, {100, 96}, {100, 96}, {100, 96}, {100, 96}, {100, 96}, {100, 96}, {100, 96}, {100, 96}, {100, 96}, {100, 96}, {100, 96}, {100, 96}, {100, 96}, {100, 96}, {100, 96}, {100, 96}, {100, 96}, {100, 96}, {100, 96}, {100, 96}, {100, 96}, {100, 96}, {100, 96}, {100, 96}, {100, 96}},
                              {{100, 96}, {102, 96}, {104, 97}, {106, 97}, {108, 98}, {110, 98}, {112, 98}, {114, 99}, {116, 99}, {118, 100}, {120, 100}, {120, 100}, {120, 100}, {120, 100}, {120, 100}, {120, 100}, {120, 100}, {120, 100}, {120, 100}, {120, 100}, {120, 100}, {120, 100}, {120, 100}, {120, 100}, {120, 100}, {120, 100}, {120, 100}, {120, 100}, {120, 100}, {120, 100}, {120, 100}, {120, 100}, {120, 100}, {120, 100}, {120, 100}},
                              {{120, 100}, {122, 100}, {124, 101}, {126, 101}, {128, 102}, {130, 102}, {132, 102}, {134, 103}, {136, 103}, {138, 104}, {140, 104}, {142, 104}, {144, 105}, {145, 105}, {145, 105}, {145, 105}, {145, 105}, {145, 105}, {145, 105}, {145, 105}, {145, 105}, {145, 105}, {145, 105}, {145, 105}, {145, 105}, {145, 105}, {145, 105}, {145, 105}, {145, 105}, {145, 105}, {145, 105}, {145, 105}, {145, 105}, {145, 105}, {145, 105}},
                              {{145, 105}, {147, 106}, {151, 107}, {153, 107}, {155, 108}, {157, 108}, {159, 109}, {161, 109}, {163, 110}, {163, 110}, {165, 110}, {165, 110}, {165, 110}, {165, 110}, {165, 110}, {165, 110}, {165, 110}, {165, 110}, {165, 110}, {165, 110}, {165, 110}, {165, 110}, {165, 110}, {165, 110}, {165, 110}, {165, 110}, {165, 110}, {165, 110}, {165, 110}, {165, 110}, {165, 110}, {165, 110}, {165, 110}, {165, 110}, {165, 110}, {165, 110}}};

const byte phase_duration_hour[5] = {0, 5, 11, 13, 11};    //4 10 12 10
const byte phase_duration_min[5] = {0, 0, 0, 30, 0};   //0 0 30 0
const byte phase_duration_sec[5] = {0, 0, 0, 0, 0};





// Keypad variables
const byte ROWS = 4;
const byte COLS = 4;


char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {33, 32, 16, 17};
byte colPins[COLS] = {14, 27, 26, 25};


Keypad keypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);



// Temp sensor variables
#define temp1 12
#define temp2 13
// Onewire objects
OneWire oneWire1(temp1);
OneWire oneWire2(temp2);

// DallasTemperature objects
DallasTemperature sensor1(&oneWire1);
DallasTemperature sensor2(&oneWire2);

float tempF[2] = {0, 0};

// Temp print string
String dry_temp_str = " --";
String wet_temp_str = " --";

String set_dry_temp_str = " --";
String set_wet_temp_str = " --";



// Controller variables
uint8_t pre_interface = 0;
uint8_t selected_interface = 1;
uint8_t selected_phase_phase = 0;

// main controller variables
uint64_t cur_time = 0;
uint64_t pre_time = 0;
uint64_t pre_eeprom_time = 0;

uint8_t second = 0;
uint8_t minute = 0;
uint8_t hour = 0;

uint8_t limit_sec = 0;
uint8_t limit_min = 0;
uint8_t limit_hour = 0;

int limit_db = 0;
int limit_wb = 0;

byte selected_mode = 0;
byte selected_phase = 0;
byte selected_pre_phase = -1;

bool cur_time_flag = false;

byte temp_phase_select = 0;




//EEPROM address
int selected_mode1 = 0;
int selected_phase1 = 20;
// int selected_pre_phase1 = 40;
int hour1 = 60;
int min1 = 80;
int sec1 = 100;
int final_limit_hour1 = 120;
int final_limit_min1 = 140;
int final_limit_sec1 = 160;
int flag1 = 180;

int selected_mode2 = 200;
int selected_phase2 = 220;
// int selected_pre_phase2 = 240;
int hour2 = 260;
int min2 = 280;
int sec2 = 300;
int final_limit_hour2 = 320;
int final_limit_min2 = 340;
int final_limit_sec2 = 360;
int flag2 = 380;
int manual_dry_temp1 = 400;
int manual_dry_temp2 = 420;
int manual_wet_temp1 = 440;
int manual_wet_temp2 = 460;

int stored_selected_phase = 0;

bool alarmable = true;
byte confirm = 0;

#define buzzer 3

#define EEPROM 0x50


// EEPROM WRITE
// EEPROM WRITE
void writeIntToEEPROM(byte deviceAddress, byte memAddress, int data) {
    Wire.beginTransmission(deviceAddress);
    Wire.write(memAddress);           // Memory address
    Wire.write((data >> 24) & 0xFF);  // Write MSB (Most Significant Byte)
    Wire.write((data >> 16) & 0xFF);  // Write next byte
    Wire.write((data >> 8) & 0xFF);   // Write next byte
    Wire.write(data & 0xFF);          // Write LSB (Least Significant Byte)
  
    uint8_t error = Wire.endTransmission();
  
    if (error != 0) {
      Serial.print("EEPROM write error: ");
      Serial.println(error);
      // Add retry logic here if desired
    }
  
    delay(10); // Ensure EEPROM write cycle is complete
  }


// Function to read an integer from EEPROM
int readIntFromEEPROM(byte deviceAddress, byte memAddress) {
  int value = 0;

  Wire.beginTransmission(deviceAddress);
  Wire.write(memAddress);           // Memory address
  Wire.endTransmission();

  Wire.requestFrom(deviceAddress, (uint8_t)4);  // Request 4 bytes (size of int)
  if (Wire.available() == 4) {
    value = (Wire.read() << 24);    // Read MSB
    value |= (Wire.read() << 16);   // Read next byte
    value |= (Wire.read() << 8);    // Read next byte
    value |= Wire.read();           // Read LSB
  }
  return value;
}

// check function
void check_eeprom() {
    if(readIntFromEEPROM(EEPROM, flag1) == 200){
        selected_mode = readIntFromEEPROM(EEPROM, selected_mode1);
        delay(20);
        if(selected_mode == 3) {

            limit_db = readIntFromEEPROM(EEPROM, manual_dry_temp1);
            limit_wb = readIntFromEEPROM(EEPROM, manual_wet_temp1);

            set_dry_temp_str = String(limit_db);
            set_wet_temp_str = String(limit_wb);
        }
        selected_phase = readIntFromEEPROM(EEPROM, selected_phase1);

        hour = readIntFromEEPROM(EEPROM, hour1);
        minute = readIntFromEEPROM(EEPROM, min1);
        second = readIntFromEEPROM(EEPROM, sec1);
        limit_hour = readIntFromEEPROM(EEPROM, final_limit_hour1);
        limit_min = readIntFromEEPROM(EEPROM, final_limit_min1);
        limit_sec = readIntFromEEPROM(EEPROM, final_limit_sec1);

        cur_time_flag = true;
        // warning_section(10);
    }
    else if(readIntFromEEPROM(EEPROM, flag2) == 200){
        selected_mode = readIntFromEEPROM(EEPROM, selected_mode2);
        delay(20);
        if(selected_mode == 3) {

            limit_db = readIntFromEEPROM(EEPROM, manual_dry_temp2);
            limit_wb = readIntFromEEPROM(EEPROM, manual_wet_temp2);

            set_dry_temp_str = String(limit_db);
            set_wet_temp_str = String(limit_wb);
        }
        selected_phase = readIntFromEEPROM(EEPROM, selected_phase2);

        hour = readIntFromEEPROM(EEPROM, hour2);
        minute = readIntFromEEPROM(EEPROM, min2);
        second = readIntFromEEPROM(EEPROM, sec2);
        limit_hour = readIntFromEEPROM(EEPROM, final_limit_hour2);
        limit_min = readIntFromEEPROM(EEPROM, final_limit_min2);
        limit_sec = readIntFromEEPROM(EEPROM, final_limit_sec2);

        cur_time_flag = true;
        // warning_section(10);
    }
    return;
}

bool flip = true;




// Touch Calibration Function
void touch_calibrate()
{
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // check file system exists
  if (!SPIFFS.begin()) {
    Serial.println("formatting file system");
    SPIFFS.format();
    SPIFFS.begin();
  }

  // check if calibration file exists and size is correct
  if (SPIFFS.exists(CALIBRATION_FILE)) {
    if (REPEAT_CAL)
    {
      // Delete if we want to re-calibrate
      SPIFFS.remove(CALIBRATION_FILE);
    }
    else
    {
      File f = SPIFFS.open(CALIBRATION_FILE, "r");
      if (f) {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL) {
    // calibration data valid
    tft.setTouch(calData);
  } else {
    // data not valid so recalibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    if (REPEAT_CAL) {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Set REPEAT_CAL to false to stop this running again!");
    }

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");

    // store data
    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}


// Time string converter
String time_to_str(uint8_t h, uint8_t m, uint8_t s) {
    String x = "";
    if(h <= 9){
        x += '0';
    }
    x += String(h) + ':';

    if(m <= 9) {
        x += '0';
    }
    x += String(m) + ':';

    if(s <= 9){
        x += '0';
    }
    x += String(s);

    return x;
}



// Home touch function
void home_touch() {
    bool pressed = tft.getTouch(&t_x, &t_y);

    // MANUAL KEY
    if(pressed && homeKey[0].contains(t_x, t_y)){
        Serial.println("Manual key pressed");
        selected_interface = 2;
        homeKey[0].press(true);
    }
    else{
        homeKey[0].press(false);
    }

    // AUTO KEY
    if(pressed && homeKey[1].contains(t_x, t_y)){
        Serial.println("Auto key pressed");
        homeKey[1].press(true);
        confirmation_screen();
        currentState = STATE_CONFIRM_WAIT;
        stateStartTime = millis();
    }

    // PHASE KEY - ADDED THIS
    if(pressed && homeKey[2].contains(t_x, t_y)){
        Serial.println("Phase key pressed");
        selected_interface = 3;
        homeKey[2].press(true);
    }
    else{
        homeKey[2].press(false);
    }

    // RESET KEY
    if(pressed && homeKey[3].contains(t_x, t_y)){
        Serial.println("Reset key pressed");
        homeKey[3].press(true);
        confirmation_screen();
        currentState = STATE_RESET_PROCESS;
        stateStartTime = millis();
    }
    else{
        homeKey[3].press(false);
    }

    if(homeKey[0].justReleased()) homeKey[0].drawButton();
    if(homeKey[0].justPressed()) homeKey[0].drawButton(true);
    if(homeKey[1].justReleased()) homeKey[1].drawButton();
    if(homeKey[1].justPressed()) homeKey[1].drawButton(true);
    if(homeKey[2].justReleased()) homeKey[2].drawButton(); // Added for phase
    if(homeKey[2].justPressed()) homeKey[2].drawButton(true); // Added for phase
    if(homeKey[3].justReleased()) homeKey[3].drawButton();
    if(homeKey[3].justPressed()) homeKey[3].drawButton(true);
}

void manual_touch() {
    bool pressed = tft.getTouch(&t_x, &t_y);

    // HOME KEY
    if(pressed && manualKey[0].contains(t_x, t_y)){
        Serial.println("home key pressed");
        selected_interface = 1;
        manualKey[0].press(true);
    }
    else{
        manualKey[0].press(false);
    }

    // SAVE KEY
    if(pressed && manualKey[1].contains(t_x, t_y)){
        Serial.println("save key pressed");
        manualKey[1].press(true);
    }
    else{
        manualKey[1].press(false);
    }

    // START KEY
    if(pressed && manualKey[2].contains(t_x, t_y)){
        Serial.println("start key pressed");
        manualKey[2].press(true);

        // Confirmation
        confirmation_screen();

        delay(10);
        while(1){
            confirm_touch();
            if(confirm == 1){
                confirm = 0;
                break;
            }
            else if(confirm == 2){
                confirm = 0;
                selected_interface = 1;
                return;
            }
        }

        int t_hour = 0;
        int t_min = 0;
        int t_sec = 0;
        int t_db = 0;
        int t_wb = 0;
        // working code
        for(int i=0; i<manual_dry_temp_str.length(); i++){
            int x = manual_dry_temp_str[i] - '0';
            // if(i != manual_dry_temp_str.length()-1){
            t_db = t_db * 10;
            // }
            t_db += x;
        }
        for(int i=0; i<manual_wet_temp_str.length(); i++){
            int x = manual_wet_temp_str[i] - '0';
            // if(i != manual_wet_temp_str.length()-1){
            t_wb = t_wb*10;
            // }
            t_wb += x;
        }
        // int ct = 0;
        // for(int i=0; i<manual_time_str.length(); i++){
        //     if(manual_time_str[i] == ':'){
        //         ct++;
        //         continue;
        //     }
        //     else {
        //         if(ct == 0)
        //             t_hour *= 10;
        //         else if(ct == 1){
        //             t_min *= 10;
        //         }
        //         else if(ct == 2){
        //             t_sec *= 10;
        //         }
        //     }
        //     int x = manual_time_str[i] - '0';
        //     if(ct == 0){
        //         t_hour += x;
        //     }
        //     else if(ct == 1){
        //         t_min += x;
        //     }
        //     else if(ct == 2){
        //         t_sec += x;
        //     }
        // }
        // if(t_hour > 60) t_hour = 0;
        // if(t_min >= 60) t_min = 0;
        // if(t_sec >= 60) t_sec = 0;

        selected_interface = 1;
        pre_interface = -1;
        limit_db = t_db;
        limit_wb = t_wb;
        // limit_hour = t_hour;
        // limit_min = t_min;
        // limit_sec = t_sec;
        hour = 0;
        minute = 0;
        second = 0;
        selected_mode = 3;
        cur_time_flag = true;
        selected_phase = 0;
        set_dry_temp_str = String(limit_db);
        set_wet_temp_str = String(limit_wb);

        writeIntToEEPROM(EEPROM, manual_dry_temp1, limit_db);
        writeIntToEEPROM(EEPROM, manual_wet_temp1, limit_wb);

        writeIntToEEPROM(EEPROM, manual_dry_temp2, limit_db);
        writeIntToEEPROM(EEPROM, manual_wet_temp2, limit_wb);

        writeIntToEEPROM(EEPROM, flag1, 0);
        writeIntToEEPROM(EEPROM, flag2, 0);
        delay(10);
    }
    else{
        manualKey[2].press(false);
    }

    // RESET KEY
    if(pressed && manualKey[3].contains(t_x, t_y)){
        Serial.println("reset key pressed");
        manualKey[3].press(true);

        // reseting values
        pre_interface = 100;
        selected_interface = 2;
        selected_manual_field = 1;
        manual_dry_temp_str = "";
        manual_wet_temp_str = "";
        manual_time_str = "";
        delay(300);
        tft.fillScreen(TFT_BLACK);
        field_change_update();
    }
    else{
        manualKey[3].press(false);
    }

    if(manualKey[0].justReleased()) manualKey[0].drawButton();
    if(manualKey[0].justPressed()) manualKey[0].drawButton(true);
    if(manualKey[1].justReleased()) manualKey[1].drawButton();
    if(manualKey[1].justPressed()) manualKey[1].drawButton(true);
    if(manualKey[2].justReleased()) manualKey[2].drawButton();
    if(manualKey[2].justPressed()) manualKey[2].drawButton(true);
    if(manualKey[3].justReleased()) manualKey[3].drawButton();
    if(manualKey[3].justPressed()) manualKey[3].drawButton(true);

    // Keypad key
    // 1
    if(pressed && manualKeypadKey[0].contains(t_x, t_y)){
        Serial.println("1 key pressed");
        manualKeypadKey[0].press(true);

        // work here
        update_field('1');
        delay(100);
    }
    else{
        manualKeypadKey[0].press(false);
    }

    // 2
    if(pressed && manualKeypadKey[1].contains(t_x, t_y)){
        Serial.println("save key pressed");
        manualKeypadKey[1].press(true);

        // work here
        update_field('2');
        delay(100);
    }
    else{
        manualKeypadKey[1].press(false);
    }

    // 3
    if(pressed && manualKeypadKey[2].contains(t_x, t_y)){
        Serial.println("start key pressed");
        manualKeypadKey[2].press(true);

        // work here
        update_field('3');
        delay(100);
    }
    else{
        manualKeypadKey[2].press(false);
    }

    // U
    if(pressed && manualKeypadKey[3].contains(t_x, t_y)){
        Serial.println("reset key pressed");
        manualKeypadKey[3].press(true);

        // work here
        selected_manual_field--;
        if(selected_manual_field < 1) selected_manual_field = 2;
        field_change_update();
        delay(100);
    }
    else{
        manualKeypadKey[3].press(false);
    }

    // 2nd row keypad
    // 4
    if(pressed && manualKeypadKey[4].contains(t_x, t_y)){
        Serial.println("1 key pressed");
        manualKeypadKey[4].press(true);

        // work here
        update_field('4');
        delay(100);
    }
    else{
        manualKeypadKey[4].press(false);
    }

    // 5
    if(pressed && manualKeypadKey[5].contains(t_x, t_y)){
        Serial.println("save key pressed");
        manualKeypadKey[5].press(true);

        // work here
        update_field('5');
        delay(100);
    }
    else{
        manualKeypadKey[5].press(false);
    }

    // 6
    if(pressed && manualKeypadKey[6].contains(t_x, t_y)){
        Serial.println("start key pressed");
        manualKeypadKey[6].press(true);

        // work here
        update_field('6');
        delay(100);
    }
    else{
        manualKeypadKey[6].press(false);
    }

    // D
    if(pressed && manualKeypadKey[7].contains(t_x, t_y)){
        Serial.println("reset key pressed");
        manualKeypadKey[7].press(true);

        // work here
        selected_manual_field = (selected_manual_field == 1) ? 2 : 1;
        field_change_update();
        delay(100);
    }
    else{
        manualKeypadKey[7].press(false);
    }

    //3rd keypad row
    // 7
    if(pressed && manualKeypadKey[8].contains(t_x, t_y)){
        Serial.println("1 key pressed");
        manualKeypadKey[8].press(true);

        // work here
        update_field('7');
        delay(100);
    }
    else{
        manualKeypadKey[8].press(false);
    }

    // 8
    if(pressed && manualKeypadKey[9].contains(t_x, t_y)){
        Serial.println("save key pressed");
        manualKeypadKey[9].press(true);

        // work here
        update_field('8');
        delay(100);
    }
    else{
        manualKeypadKey[9].press(false);
    }

    // 9
    if(pressed && manualKeypadKey[10].contains(t_x, t_y)){
        Serial.println("start key pressed");
        manualKeypadKey[10].press(true);

        // work here
        update_field('9');
        delay(100);
    }
    else{
        manualKeypadKey[10].press(false);
    }

    // <- backspace
    if(pressed && manualKeypadKey[11].contains(t_x, t_y)){
        Serial.println("reset key pressed");
        manualKeypadKey[11].press(true);

        // work here
        if(selected_manual_field == 1){
            if(manual_dry_temp_str.length() > 0){
                // manual_dry_temp_str[manual_dry_temp_str.length()-1] = '\0';
                // manual_dry_temp_str.setCharAt(manual_dry_temp_str.length() - 1, '\0');
                manual_dry_temp_str = manual_dry_temp_str.substring(0, manual_dry_temp_str.length() - 1);
            }
        }
        if(selected_manual_field == 2){
            if(manual_wet_temp_str.length() > 0){
                // manual_wet_temp_str[manual_wet_temp_str.length()-1] = '\0';
                // manual_wet_temp_str.setCharAt(manual_wet_temp_str.length() - 1, '\0');
                manual_wet_temp_str = manual_wet_temp_str.substring(0, manual_wet_temp_str.length() - 1);
            }
        }
        if(selected_manual_field == 3){
            if(manual_dry_temp_str.length() > 0){
                // manual_time_str[manual_time_str.length()-1] = '\0';
                // manual_time_str.setCharAt(manual_time_str.length() - 1, '\0');
                manual_time_str = manual_time_str.substring(0, manual_time_str.length() - 1);
            }
        }
        field_change_update();
        delay(100);
    }
    else{
        manualKeypadKey[11].press(false);
    }

    // 4th keypad row
    // .
    if(pressed && manualKeypadKey[12].contains(t_x, t_y)){
        Serial.println("1 key pressed");
        manualKeypadKey[12].press(true);

        // work here
        update_field('.');
        delay(100);
    }
    else{
        manualKeypadKey[12].press(false);
    }

    // 0
    if(pressed && manualKeypadKey[13].contains(t_x, t_y)){
        Serial.println("save key pressed");
        manualKeypadKey[13].press(true);

        // work here
        update_field('0');
        delay(100);
    }
    else{
        manualKeypadKey[13].press(false);
    }

    // :
    if(pressed && manualKeypadKey[14].contains(t_x, t_y)){
        Serial.println("start key pressed");
        manualKeypadKey[14].press(true);

        // work here
        update_field(':');
        delay(100);
    }
    else{
        manualKeypadKey[14].press(false);
    }

    // ok
    if(pressed && manualKeypadKey[15].contains(t_x, t_y)){
        Serial.println("reset key pressed");
        manualKeypadKey[15].press(true);

        // work here
        delay(100);
    }
    else{
        manualKeypadKey[15].press(false);
    }

    for(int i=0;i<16;i++){
        if(manualKeypadKey[i].justReleased()) manualKeypadKey[i].drawButton();
        if(manualKeypadKey[i].justPressed()) manualKeypadKey[i].drawButton(true);
    }
}

// manual field update
void update_field(char ch) {
    switch(selected_manual_field){
        case 1:
            if(manual_dry_temp_str.length() < 3 && (ch != '.' && ch != ':'))
                manual_dry_temp_str += ch;
            print_manual_dry_value();
            break;
        case 2:
            if(manual_wet_temp_str.length() < 3 && (ch != '.' && ch != ':'))
                manual_wet_temp_str += ch;
            print_manual_wet_value();
            break;
        case 3:
            if(manual_time_str.length() < 8 && (ch != '.'))
                manual_time_str += ch;
            print_manual_time_value();
            break;
    }
}

// field change update
void field_change_update() {
    print_manual_dry_value();
    print_manual_wet_value();
    // print_manual_time_value();
}


// phase touch control
void phase_touch() {
    bool pressed = tft.getTouch(&t_x, &t_y);
    // temp_phase_select = 0;
    // YELLOWING KEY
    if(pressed && phaseKey[0].contains(t_x, t_y)){
        Serial.println("home key pressed");
        phaseKey[0].press(true);

        // selected_phase_phase = 1;
        temp_phase_select = 1;
        update_phase_field();
        update_phase_time_field();
        delay(100);
    }
    else{
        phaseKey[0].press(false);
    }

    // LAMINA KEY
    if(pressed && phaseKey[1].contains(t_x, t_y)){
        Serial.println("save key pressed");
        phaseKey[1].press(true);

        // selected_phase_phase = 2;
        temp_phase_select = 2;
        update_phase_field();
        update_phase_time_field();
        delay(100);
    }
    else{
        phaseKey[1].press(false);
    }

    // COLOR FIXING KEY
    if(pressed && phaseKey[2].contains(t_x, t_y)){
        Serial.println("start key pressed");
        // selected_interface = 3;
        phaseKey[2].press(true);

        // selected_phase_phase = 3;
        temp_phase_select = 3;
        update_phase_field();
        update_phase_time_field();
        delay(100);
    }
    else{
        phaseKey[2].press(false);
    }

    // STEAM KEY
    if(pressed && phaseKey[3].contains(t_x, t_y)){
        Serial.println("reset key pressed");
        phaseKey[3].press(true);

        // selected_phase_phase = 4;
        temp_phase_select = 4;
        update_phase_field();
        update_phase_time_field();
        delay(100);
    }
    else{
        phaseKey[3].press(false);
    }

    // START KEY
    if(pressed && phaseKey[4].contains(t_x, t_y)){
        Serial.println("start key pressed");
        phaseKey[4].press(true);
        // Confirmation
        confirmation_screen();

        unsigned long startTime = millis();
        const unsigned long CONFIRM_TIMEOUT = 30000; // 30 seconds timeout

        while(millis() - startTime < CONFIRM_TIMEOUT){
            confirm_touch();
            if(confirm == 1){
                confirm = 0;
                break;
            }
            else if(confirm == 2){
                confirm = 0;
                pre_interface = -1;
                selected_interface = 1;
                return;
            }
            delay(10); // Small delay to prevent tight loop
        }

        // If we reach here, timeout occurred
        Serial.println("Confirmation timeout - returning to home");
        confirm = 0;
        pre_interface = -1;
        selected_interface = 1;
        return;
        selected_phase = temp_phase_select;
        selected_mode = 2;
        pre_interface = -1;
        selected_interface = 1;
        temp_phase_select = 0;
        cur_time_flag = true;
        hour = 0;
        minute = 0;
        second = 0;
        limit_hour = phase_duration_hour[selected_phase];
        limit_min = phase_duration_min[selected_phase];
        limit_sec = phase_duration_sec[selected_phase];

        writeIntToEEPROM(EEPROM, flag1, 0);
        writeIntToEEPROM(EEPROM, flag2, 0);
        delay(10);
    }
    else{
        phaseKey[4].press(false);
    }

    // HOME KEY
    if(pressed && phaseKey[5].contains(t_x, t_y)){
        Serial.println("reset key pressed");
        selected_interface = 1;
        temp_phase_select = 0;
        phaseKey[5].press(true);
    }
    else{
        phaseKey[5].press(false);
    }

    if(phaseKey[0].justReleased()) phaseKey[0].drawButton();
    if(phaseKey[0].justPressed()) phaseKey[0].drawButton(true);

    if(phaseKey[1].justReleased()) phaseKey[1].drawButton();
    if(phaseKey[1].justPressed()) phaseKey[1].drawButton(true);

    if(phaseKey[2].justReleased()) phaseKey[2].drawButton();
    if(phaseKey[2].justPressed()) phaseKey[2].drawButton(true);

    if(phaseKey[3].justReleased()) phaseKey[3].drawButton();
    if(phaseKey[3].justPressed()) phaseKey[3].drawButton(true);

    if(phaseKey[4].justReleased()) phaseKey[4].drawButton();
    if(phaseKey[4].justPressed()) phaseKey[4].drawButton(true);

    if(phaseKey[5].justReleased()) phaseKey[5].drawButton();
    if(phaseKey[5].justPressed()) phaseKey[5].drawButton(true);
}

// Update phase field
void update_phase_field() {
    tft.fillRect(offset_col + 80, offset_row, 160, textHeight*2+5, TFT_BLACK);
    print_phase_select_value();
}

// Update time field
void update_phase_time_field() {
    phase_time_value_str = "";
    String x = "";
    if(phase_duration_hour[temp_phase_select] <= 9)
        phase_time_value_str += '0';
    x = String(phase_duration_hour[temp_phase_select]);
    phase_time_value_str += x + ":";

    if(phase_duration_min[temp_phase_select] <= 9)
        phase_time_value_str += '0';
    x = String(phase_duration_min[temp_phase_select]);
    phase_time_value_str += x + ":";

    if(phase_duration_sec[temp_phase_select] <= 9)
        phase_time_value_str += '0';
    x = String(phase_duration_sec[temp_phase_select]);
    phase_time_value_str += x;
    print_phase_manual_time_value();
}



// Phase Screen Function
void phase_screen() {
    tft.fillScreen(TFT_BLACK);
    tft.fillRect(0, 0, tft.width(), tft.height(), TFT_BLACK);

    print_phase_select_label();
    print_phase_select_value();

    print_phase_manual_time();
    print_phase_manual_time_value();

    draw_phase_key();
}

// First row
// Phase label print
void print_phase_select_label() {
    tft.setFreeFont(FSB9);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("PHASE :", offset_col + 5, offset_row -5 + 7, GFXFF);
}

void print_phase_select_value() {
    tft.setFreeFont(FSB9);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(phase_label[temp_phase_select], offset_col + 80, offset_row -5 + 7, GFXFF);
}


// Second row
// Time label print
void print_phase_manual_time() {
    tft.setFreeFont(FSB9);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("TIME   :", offset_col + 5, offset_row + 7 + textHeight, GFXFF);
}

void print_phase_manual_time_value() {
    tft.setFreeFont(FSB9);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(phase_time_value_str, offset_col + 80, offset_row + 7 + textHeight, GFXFF);
}

// Phase button draw
void draw_phase_key() {
    int home_key_col_offset = 115;
    int home_key_row_offset = 100;
    int home_key_width = 180;
    int home_key_height = 26;
    int home_key_space_x = 6;
    int home_key_space_y = 12;
    for (uint8_t row = 0; row < 6; row++) {
        for (uint8_t col = 0; col < 1; col++) {
            uint8_t b = col + row * 1;

            tft.setFreeFont(FSB9);

            phaseKey[b].initButton(&tft, home_key_col_offset + col * (home_key_width + home_key_space_x),
                                home_key_row_offset + row * (home_key_height + home_key_space_y),
                                home_key_width, home_key_height, TFT_WHITE, phaseKeyColor[b], TFT_WHITE,
                                phaseKeyLabel[b], 1);
            phaseKey[b].drawButton();
        }
    }
}




// Manual Mode Screen function
void manual_screen() {
    tft.fillScreen(TFT_BLACK);

    tft.fillRect(0, 0, tft.width(), tft.height(), TFT_BLACK);

    print_manual_dry_label();
    print_manual_dry_value();

    print_manual_wet_label();
    print_manual_wet_value();

    // print_manual_time_label();
    // print_manual_time_value();

    draw_manual_keypad();

    draw_manual_key();

    draw_manual_phase_key();
}


// Manual first row
// Manual Dry label print
void print_manual_dry_label() {
    tft.drawRect(offset_col+2, offset_row - 5, textWidth, textHeight, TFT_WHITE);
    tft.setFreeFont(FF1);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("DRY", offset_col+5, offset_row -5 +7, GFXFF);
}

// Manual Dry Value print
void print_manual_dry_value() {
    tft.setFreeFont(FSB9);                 // Select the font
    if(selected_manual_field == 1) {
        tft.drawRect(offset_col + textWidth + 3 + 2, offset_row - 5, textWidth + 111, textHeight, TFT_GREEN);
        tft.fillRect(offset_col + textWidth + 3 + 3, offset_row - 4, textWidth + 109, textHeight-2, TFT_RED);
        tft.setTextColor(TFT_WHITE, TFT_RED);
    }
    else {
        tft.drawRect(offset_col + textWidth + 3 + 2, offset_row - 5, textWidth + 111, textHeight, TFT_WHITE);
        tft.fillRect(offset_col + textWidth + 3 + 3, offset_row - 4, textWidth + 109, textHeight-2, TFT_BLACK);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
    }
    tft.drawString(manual_dry_temp_str, offset_col + textWidth + 3 + 15, offset_row -5 +7, GFXFF);
}


// Manual second row
// Manual Wet label print
void print_manual_wet_label() {
    tft.drawRect(offset_col+2, offset_row - 5 + textHeight + 5, textWidth, textHeight, TFT_WHITE);
    tft.setFreeFont(FF1);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("WET", offset_col+5, offset_row - 5 + textHeight + 5 + 7, GFXFF);
}

// Manual Wet value print
void print_manual_wet_value() {
    tft.setFreeFont(FSB9);                 // Select the font
    if(selected_manual_field == 2) {
        tft.drawRect(offset_col + textWidth + 3 + 2, offset_row - 5 + textHeight + 5, textWidth + 111, textHeight, TFT_GREEN);
        tft.fillRect(offset_col + textWidth + 3 + 3, offset_row - 5 + textHeight + 6, textWidth + 109, textHeight-2, TFT_RED);
        tft.setTextColor(TFT_WHITE, TFT_RED);
    }
    else {
        tft.drawRect(offset_col + textWidth + 3 + 2, offset_row - 5 + textHeight + 5, textWidth + 111, textHeight, TFT_WHITE);
        tft.fillRect(offset_col + textWidth + 3 + 3, offset_row - 5 + textHeight + 6, textWidth + 109, textHeight-2, TFT_BLACK);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
    }
    tft.drawString(manual_wet_temp_str, offset_col + textWidth + 3 + 15, offset_row - 5 + textHeight + 5 + 7, GFXFF);
}


// Manual third row
// Manual Wet label print
void print_manual_time_label() {
    tft.drawRect(offset_col+2, offset_row -5 + textHeight*2 + 10, textWidth, textHeight, TFT_WHITE);
    tft.setFreeFont(FF1);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("TIME", offset_col+5, offset_row -5 + textHeight*2 + 10 + 7, GFXFF);
}

// Manual Time value print
void print_manual_time_value(){
    tft.setFreeFont(FSB9);                 // Select the font
    if(selected_manual_field == 3){
        tft.drawRect(offset_col + textWidth + 3 + 2, offset_row -5 + textHeight*2 + 10, textWidth + 111, textHeight, TFT_GREEN);
        tft.fillRect(offset_col + textWidth + 3 + 3, offset_row -5 + textHeight*2 + 11, textWidth + 109, textHeight-2, TFT_RED);
        tft.setTextColor(TFT_WHITE, TFT_RED);
    }
    else{
        tft.drawRect(offset_col + textWidth + 3 + 2, offset_row -5 + textHeight*2 + 10, textWidth + 111, textHeight, TFT_WHITE);
        tft.fillRect(offset_col + textWidth + 3 + 3, offset_row -5 + textHeight*2 + 11, textWidth + 109, textHeight-2, TFT_BLACK);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
    }
    tft.drawString(manual_time_str, offset_col + textWidth + 3 + 15, offset_row -5 + textHeight*2 + 10 + 7, GFXFF);
}


// Draw manual keypad
void draw_manual_keypad() {
    int home_key_col_offset = 18;
    int home_key_row_offset = 130;
    int home_key_width = 30;
    int home_key_height = 30;
    int home_key_space_x = 6;
    int home_key_space_y = 6;
    for (uint8_t row = 0; row < 4; row++) {
        for (uint8_t col = 0; col < 4; col++) {
            uint8_t b = col + row * 4;

            // if (b < 3) tft.setFreeFont(LABEL1_FONT);
            // else tft.setFreeFont(LABEL2_FONT);

            tft.setFreeFont(FSB9);

            manualKeypadKey[b].initButton(&tft, home_key_col_offset + col * (home_key_width + home_key_space_x),
                                home_key_row_offset + row * (home_key_height + home_key_space_y),
                                home_key_width, home_key_height, TFT_WHITE, TFT_WHITE, TFT_BLACK,
                                manualKeypadKeyLabel[b], 1);

            // key[b].initButton(&tft, KEY_X + col * (KEY_W + KEY_SPACING_X),
            //                     KEY_Y + row * (KEY_H + KEY_SPACING_Y), // x, y, w, h, outline, fill, text
            //                     KEY_W, KEY_H, TFT_WHITE, keyColor[b], TFT_WHITE,
            //                     keyLabel[b], KEY_TEXTSIZE);
            manualKeypadKey[b].drawButton();
        }
    }
}

// Draw manual key
void draw_manual_key() {
    int home_key_col_offset = 62;
    int home_key_row_offset = 270;
    int home_key_width = 100;
    int home_key_height = 23;
    int home_key_space_x = 10;
    int home_key_space_y = 12;
    for (uint8_t row = 0; row < 2; row++) {
        for (uint8_t col = 0; col < 2; col++) {
            uint8_t b = col + row * 2;

            tft.setFreeFont(FSB9);

            manualKey[b].initButton(&tft, home_key_col_offset + col * (home_key_width + home_key_space_x),
                                home_key_row_offset + row * (home_key_height + home_key_space_y),
                                home_key_width, home_key_height, TFT_WHITE, homeKeyColor[b], TFT_BLACK,
                                manualKeyLabel[b], 1);

            manualKey[b].drawButton();
        }
    }
}

// Draw manual phase key
void draw_manual_phase_key() {
    int home_key_col_offset = 192;
    int home_key_row_offset = 130;
    int home_key_width = 91;
    int home_key_height = 26;
    int home_key_space_x = 6;
    int home_key_space_y = 8;
    for (uint8_t row = 0; row < 4; row++) {
        for (uint8_t col = 0; col < 1; col++) {
            uint8_t b = col + row * 1;

            tft.setFreeFont(FSB9);

            manualPhaseKey[b].initButton(&tft, home_key_col_offset + col * (home_key_width + home_key_space_x),
                                home_key_row_offset + row * (home_key_height + home_key_space_y),
                                home_key_width, home_key_height, TFT_WHITE, tft.color565(255, 0, 255), TFT_WHITE,
                                manualPhaseKeyLabel[b], 1);

            manualPhaseKey[b].drawButton();
        }
    }
}




// COnfirmation Screen
void confirmation_screen() {
    tft.fillScreen(TFT_BLACK);
    tft.fillRect(0, 0, tft.width(), tft.height(), TFT_BLACK);

    confirmation_label();
    draw_confirm_key();
}

//Confirmation label print
void confirmation_label() {
    tft.setFreeFont(FSB12);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Confirm?", 70, 110, GFXFF);
}

// Draw COnfirm Keys
void draw_confirm_key() {
    int home_key_col_offset = 62;
    int home_key_row_offset = 250;
    int home_key_width = 100;
    int home_key_height = 23;
    int home_key_space_x = 10;
    int home_key_space_y = 12;

    tft.setFreeFont(FSB9);

    // YES
    confirmKey[0].initButton(&tft, 65,
                            170,
                            home_key_width, home_key_height, TFT_WHITE, homeKeyColor[3], TFT_BLACK,
                            "YES (*)", 1);

    // NO
    confirmKey[1].initButton(&tft, 65 + home_key_width + 10,
                            170,
                            home_key_width, home_key_height                            , TFT_WHITE, homeKeyColor[2], TFT_BLACK,
                            "NO (#)", 1);

    confirmKey[0].drawButton();
    confirmKey[1].drawButton();
}

// COnfirm touch
void confirm_touch() {
    static unsigned long lastTouchTime = 0;
    const unsigned long TOUCH_TIMEOUT = 30000; // 30 seconds timeout

    bool pressed = tft.getTouch(&t_x, &t_y);

    if (pressed) {
        lastTouchTime = millis();
    } else if (millis() - lastTouchTime > TOUCH_TIMEOUT) {
        // Timeout occurred, force NO selection
        confirm = 2;
        return;
    }

    // YES KEY
    if(pressed && confirmKey[0].contains(t_x, t_y)){
        Serial.println("YES");
        confirmKey[0].press(true);
        confirm = 1;
    }
    else{
        confirmKey[0].press(false);
    }

    // NO KEY
    if(pressed && confirmKey[1].contains(t_x, t_y)){
        Serial.println("NO");
        confirmKey[1].press(true);
        confirm = 2;
    }
    else{
        confirmKey[1].press(false);
    }

    if(confirmKey[0].justReleased()) confirmKey[0].drawButton();
    if(confirmKey[0].justPressed()) confirmKey[0].drawButton(true);
    if(confirmKey[1].justReleased()) confirmKey[1].drawButton();
    if(confirmKey[1].justPressed()) confirmKey[1].drawButton(true);
}


// return confirm
bool return_confirm() {
    unsigned long startTime = millis();
    const unsigned long CONFIRM_TIMEOUT = 30000; // 30 seconds timeout

    while(millis() - startTime < CONFIRM_TIMEOUT) {
        if(confirm == 1) return true;
        else if(confirm == 2) return false;
        delay(10); // Small delay to prevent tight loop
    }

    // If we reach here, timeout occurred
    Serial.println("Confirmation timeout - defaulting to NO");
    return false;
}






// Home Screen Function
void home_screen() {
    tft.fillScreen(TFT_BLACK);
    tft.fillRect(0, 0, tft.width(), tft.height(), TFT_BLACK);

    print_db_label();
    print_db_temp();
    print_wb_label();
    print_wb_temp();

    print_dbset_label();
    print_dbset_temp();
    print_wbset_label();
    print_wbset_temp();

    print_curtime_label();
    print_curtime();

    print_limittime_label();
    print_limittime();

    print_phase_label();
    print_phase();

    print_mode_label();
    print_mode();

    draw_home_keypad();
}


// First Row
// Print DB label
void print_db_label() {
    tft.drawRect(offset_col, offset_row, textWidth, textHeight, TFT_WHITE);
    tft.setFreeFont(FF1);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("DB", offset_col+3, offset_row+7, GFXFF);
}

// Print DB temp
void print_db_temp() {
    tft.drawRect(offset_col + textWidth + 1, offset_row, textWidth, textHeight, TFT_WHITE);
    tft.fillRect(offset_col + textWidth + 2, offset_row + 1, textWidth - 2, textHeight - 2, TFT_BLACK);
    tft.setFreeFont(FF1);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(dry_temp_str, offset_col + textWidth + 3+3, offset_row+7, GFXFF);
}

// Print WB label
void print_wb_label() {
    tft.drawRect(offset_col + textWidth * 2 + 2, offset_row, textWidth, textHeight, TFT_WHITE);
    tft.setFreeFont(FF1);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("WB", offset_col + textWidth * 2 + 3+3+3, offset_row+7, GFXFF);
}

// Print WB temp
void print_wb_temp() {
    tft.drawRect(offset_col + textWidth * 3 + 3, offset_row, textWidth, textHeight, TFT_WHITE);
    tft.fillRect(offset_col + textWidth * 3 + 4, offset_row + 1, textWidth - 2, textHeight - 2, TFT_BLACK);
    tft.setFreeFont(FF1);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(wet_temp_str, offset_col + textWidth * 3 + 3+3+3+3, offset_row+7, GFXFF);
}



// Second Row
// Print DB SET label
void print_dbset_label() {
    tft.drawRect(offset_col, offset_row + textHeight + 5, textWidth, textHeight, TFT_WHITE);
    tft.setFreeFont(FF1);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("DBSET", offset_col+3, offset_row + textHeight + 5 + 7, GFXFF);
}

// Print DB SET temp
void print_dbset_temp() {
    tft.drawRect(offset_col + textWidth + 1, offset_row + textHeight + 5, textWidth, textHeight, TFT_WHITE);
    tft.fillRect(offset_col + textWidth + 2, offset_row + textHeight + 6, textWidth-2, textHeight-2, TFT_BLACK);
    tft.setFreeFont(FF1);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(set_dry_temp_str + 'F', offset_col + textWidth + 3+3, offset_row + textHeight + 5 + 7, GFXFF);
}

// Print WB SET label
void print_wbset_label() {
    tft.drawRect(offset_col + textWidth * 2 + 2, offset_row + textHeight + 5, textWidth, textHeight, TFT_WHITE);
    tft.setFreeFont(FF1);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("WBSET", offset_col + textWidth * 2 + 3+2, offset_row + textHeight + 5 + 7, GFXFF);
}

// Print WB SET temp
void print_wbset_temp() {
    tft.drawRect(offset_col + textWidth * 3 + 3, offset_row + textHeight + 5, textWidth, textHeight, TFT_WHITE);
    tft.drawRect(offset_col + textWidth * 3 + 4, offset_row + textHeight + 6, textWidth-2, textHeight-2, TFT_BLACK);
    tft.setFreeFont(FF1);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(set_wet_temp_str + 'F', offset_col + textWidth * 3 + 3+3+3+3, offset_row + textHeight + 5 + 7, GFXFF);
}



// Third Row
// Print Current Time label
void print_curtime_label() {
    tft.drawRect(offset_col, offset_row + 70, textWidth + 60, textHeight, TFT_WHITE);
    tft.setFreeFont(FF1);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("CUR. TIME", offset_col+3, offset_row + 7 + 70, GFXFF);
}

// Print Current Time
void print_curtime() {
    home_cur_time_str = time_to_str(hour, minute, second);
    tft.fillRect(offset_col + textWidth*2 + 3, offset_row + 70, textWidth + 60, textHeight, TFT_BLACK);
    tft.setFreeFont(FSB12);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(home_cur_time_str, offset_col + textWidth*2 + 3 + 3, offset_row + 5 + 70, GFXFF);
}



// Fourth Row
// Print Limit Time label
void print_limittime_label() {
    tft.drawRect(offset_col, offset_row + 70 + textHeight + 5, textWidth + 60, textHeight, TFT_WHITE);
    tft.setFreeFont(FF1);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("LIMIT TIME", offset_col+3, offset_row + 7 + 70 + textHeight + 5, GFXFF);
}

void print_limittime() {
    home_limit_time_str = time_to_str(limit_hour, limit_min, limit_sec);
    tft.fillRect(offset_col + textWidth*2 + 3 + 3, offset_row + 70 + textHeight + 5, textWidth + 60, textHeight, TFT_BLACK);
    tft.setFreeFont(FSB12);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(home_limit_time_str, offset_col + textWidth*2 + 3 + 3, offset_row + 5 + 70 + textHeight + 5, GFXFF);
}



// Fifth Row
// Print Phase label
void print_phase_label() {
    tft.drawRect(offset_col, offset_row + 70 + textHeight*2 + 10, textWidth + 60, textHeight, TFT_WHITE);
    tft.setFreeFont(FF1);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("PHASE", offset_col+3, offset_row + 7 + 70 + textHeight*2 + 10, GFXFF);
}

void print_phase() {
    tft.fillRect(offset_col + textWidth*2 + 3+3, offset_row + 70 + textHeight*2 + 10, textWidth + 60, textHeight, TFT_BLACK);
    tft.setFreeFont(FF1);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(phase_label[selected_phase], offset_col + textWidth*2 + 3+3, offset_row + 7 + 70 + textHeight*2 + 10, GFXFF);
}



// Sixth Row
// Print Mode label
void print_mode_label() {
    tft.drawRect(offset_col, offset_row + 70 + textHeight*3 + 15, textWidth + 60, textHeight, TFT_WHITE);
    tft.setFreeFont(FF1);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("MODE", offset_col+3, offset_row + 7 + 70 + textHeight*3 + 15, GFXFF);
}

void print_mode() {
    tft.fillRect(offset_col + textWidth*2 + 3 + 3, offset_row + 70 + textHeight*3 + 15, textWidth + 60, textHeight, TFT_BLACK);
    tft.setFreeFont(FF1);                 // Select the font
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(mode_label[selected_mode], offset_col + textWidth*2 + 3 + 3, offset_row + 7 + 70 + textHeight*3 + 15, GFXFF);
}



// Draw home keypad
void draw_home_keypad() {
    int home_key_col_offset = 62;
    int home_key_row_offset = 250;
    int home_key_width = 100;
    int home_key_height = 23;
    int home_key_space_x = 10;
    int home_key_space_y = 12;
    for (uint8_t row = 0; row < 2; row++) {
        for (uint8_t col = 0; col < 2; col++) {
            uint8_t b = col + row * 2;

            tft.setFreeFont(FSB9);

            homeKey[b].initButton(&tft, home_key_col_offset + col * (home_key_width + home_key_space_x),
                                home_key_row_offset + row * (home_key_height + home_key_space_y),
                                home_key_width, home_key_height, TFT_WHITE, homeKeyColor[b], TFT_BLACK,
                                homeKeyLabel[b], 1);

            homeKey[b].drawButton();
        }
    }
}

// Interface control function
void interface_control() {
    if(pre_interface == selected_interface) return;
    switch(selected_interface){
        case 1:
            home_screen();
            break;
        case 2:
            manual_screen();
            break;
        case 3:
            phase_screen();
            break;
    }
    pre_interface = selected_interface;
}

// Touch control function
void touch_control() {
    switch(selected_interface){
        case 1:
            home_touch();
            break;
        case 2:
            manual_touch();
            break;
        case 3:
            phase_touch();
            break;
    }
}

// Helper function for EEPROM updates
void handleEEPROMUpdate() {
    pre_eeprom_time = cur_time;
    if(flip){
        if(selected_mode == 3){
            writeIntToEEPROM(EEPROM, manual_dry_temp1, limit_db);
            writeIntToEEPROM(EEPROM, manual_wet_temp1, limit_wb);
        }
        writeIntToEEPROM(EEPROM, selected_mode1, selected_mode);
        writeIntToEEPROM(EEPROM, selected_phase1, selected_phase);
        writeIntToEEPROM(EEPROM, hour1, hour);
        writeIntToEEPROM(EEPROM, min1, minute);
        writeIntToEEPROM(EEPROM, sec1, second);
        writeIntToEEPROM(EEPROM, final_limit_hour1, limit_hour);
        writeIntToEEPROM(EEPROM, final_limit_min1, limit_min);
        writeIntToEEPROM(EEPROM, final_limit_sec1, limit_sec);
        writeIntToEEPROM(EEPROM, flag1, 200);
    } else {
        if(selected_mode == 3){
            writeIntToEEPROM(EEPROM, manual_dry_temp2, limit_db);
            writeIntToEEPROM(EEPROM, manual_wet_temp2, limit_wb);
        }
        writeIntToEEPROM(EEPROM, selected_mode2, selected_mode);
        writeIntToEEPROM(EEPROM, selected_phase2, selected_phase);
        writeIntToEEPROM(EEPROM, hour2, hour);
        writeIntToEEPROM(EEPROM, min2, minute);
        writeIntToEEPROM(EEPROM, sec2, second);
        writeIntToEEPROM(EEPROM, final_limit_hour2, limit_hour);
        writeIntToEEPROM(EEPROM, final_limit_min2, limit_min);
        writeIntToEEPROM(EEPROM, final_limit_sec2, limit_sec);
        writeIntToEEPROM(EEPROM, flag2, 200);
    }
    flip = !flip;
}

// Helper function for temperature sensor handling
void handleTemperatureSensors() {
    if(tempF[0] < 10.0 || tempF[1] < 10.0){
        unsigned long sensorCheckStart = millis();
        const unsigned long SENSOR_CHECK_TIMEOUT = 30000;

        while(millis() - sensorCheckStart < SENSOR_CHECK_TIMEOUT){
            temp_measure();
            if(tempF[0] >= 10.0 && tempF[1] >= 10.0) break;
            sensor_warning();
            delay(100);
        }

        if(tempF[0] < 10.0 || tempF[1] < 10.0) {
            Serial.println("Warning: Temperature sensors not responding properly");
        }
    }
}

// Helper function for timer updates
void handleTimerUpdate() {
    pre_time = cur_time;
    second++;
    if(second == 60){ //60
        minute++;
        second = 0;
        print_dbset_temp();
        print_wbset_temp();
        if(minute == 60){ //60
            hour++;
            minute = 0;
        }
    }
    print_curtime();
}

// warining section
void warning_section(int ct) {
    for(int i=0;i<ct;i++){
        tone(buzzer, 1000);
        delay(1000);
        tone(buzzer, 0);        
        delay(1000);
    }
}


// Temp control
void temp_control() {
    int db = int(tempF[0]);
    int wb = int(tempF[1]);

    if(limit_db > db){
        // db relay on
        digitalWrite(relay1, HIGH);
    } else {
        // db relay off
        digitalWrite(relay1, LOW);
    }

    if(limit_wb > wb) {
        // wb relay on
        digitalWrite(relay2, HIGH);
    } else {
        // wb relay off
        digitalWrite(relay2, LOW);
    }
}







// Phase control
void phase_control() {
    static unsigned long phaseControlStart = 0;
    const unsigned long PHASE_CONTROL_TIMEOUT = 60000; // 1 minute timeout

    if (phaseControlStart == 0) {
        phaseControlStart = millis();
    }

    // Check for timeout
    if (millis() - phaseControlStart > PHASE_CONTROL_TIMEOUT) {
        Serial.println("Phase control timeout - resetting");
        phaseControlStart = 0;
        return;
    }

    // Improved time comparison logic
    bool timeReached = false;
    if (hour > limit_hour) {
        timeReached = true;
    } else if (hour == limit_hour) {
        if (minute > limit_min) {
            timeReached = true;
        } else if (minute == limit_min) {
            if (second >= limit_sec) {
                timeReached = true;
            }
        }
    }

    if(timeReached && alarmable && (selected_mode == 2)){
        delay(10);
        warning_section(5);
        alarmable = false;

        if(selected_mode == 2){ // ----------------------------------------------------------------------------
            if (hour < sizeof(phase_temp2[selected_phase-1])/sizeof(phase_temp2[selected_phase-1][0])) {
                limit_db = phase_temp2[selected_phase-1][hour][0];
                limit_wb = phase_temp2[selected_phase-1][hour][1];
                set_dry_temp_str = String(limit_db);
                set_wet_temp_str = String(limit_wb);
            }
        }
    }

    else if(timeReached && (selected_mode == 1)){
        selected_phase++;
        if(selected_phase >= 5){
            resetPhaseControl();
            warning_section(5);
        } else {
            warning_section(5);
            initializePhaseTime();
            print_limittime();
            print_phase();
        }
    }

    // Reset timeout counter if we're still in valid state
    if (!timeReached) {
        phaseControlStart = 0;
    }
}

// Helper function to reset phase control

void resetPhaseControl() {
    hour = 0; minute = 0; second = 0;
    limit_hour = 0; limit_min = 0; limit_sec = 0;
    selected_phase = 0;
    selected_pre_phase = -1;
    // Keep the final set points only if we were in manual mode
    if (selected_mode != 3) {
        set_dry_temp_str = " --";
        set_wet_temp_str = " --";
        limit_db = 0;
        limit_wb = 0;
    }
    selected_mode = 0;
    cur_time_flag = false;

    pre_interface = -1;
    selected_interface = 1;

    writeIntToEEPROM(EEPROM, flag1, 0);
    writeIntToEEPROM(EEPROM, flag2, 0);
}

// void resetPhaseControl() {
//     hour = 0;
//     minute = 0;
//     second = 0;
//     limit_hour = 0;
//     limit_min = 0;
//     limit_sec = 0;
//     selected_phase = 0;
//     selected_pre_phase = -1;
//     selected_mode = 0;
//     cur_time_flag = false;

//     set_dry_temp_str = " --";
//     set_wet_temp_str = " --";

//     limit_db = 0;
//     limit_wb = 0;

//     pre_interface = -1;
//     selected_interface = 1;

//     writeIntToEEPROM(EEPROM, flag1, 0);
//     writeIntToEEPROM(EEPROM, flag2, 0);
// }

// Helper function to initialize phase time
void initializePhaseTime() {
    hour = 0;
    minute = 0;
    second = 0;
    limit_hour = phase_duration_hour[selected_phase];
    limit_min = phase_duration_min[selected_phase];
    limit_sec = phase_duration_sec[selected_phase];

    pre_time = millis();
    cur_time_flag = true;
    print_curtime();
}



// sensor if not connected
void sensor_warning() {
    warning_section(1);
}


// temperature measurement function
void temp_measure() {
    static uint64_t temp_pre_time = 0;
    if(millis() - temp_pre_time >= 500 && selected_interface == 1){
        sensor1.requestTemperatures();
        sensor2.requestTemperatures();

        tempF[0] = sensor1.getTempFByIndex(0);
        tempF[1] = sensor2.getTempFByIndex(0);

        dry_temp_str = "";
        wet_temp_str = "";
        dry_temp_str = String(int(tempF[0]));
        wet_temp_str = String(int(tempF[1]));

        if(tempF[0] >= -10.0){
            dry_temp_str += 'F';
        }
        if(tempF[1] >= -10.0){
            wet_temp_str += 'F';
        }

        print_db_temp();
        print_wb_temp();

        temp_pre_time = millis();
    }
}


// Keypad event
void keypadEvent(KeypadEvent key) {
    switch (keypad.getState()){
        case PRESSED:
            if(selected_interface == 1){
                // MANUAL
                if(key == 'A'){
                    selected_interface = 2;
                }
                // AUTO
                else if(key == 'B'){
                    // Confirmation
                    confirmation_screen();

                    delay(10);
                    while(1){
                        char k = keypad.getKey();
                        if(k == '*'){ confirm = 1;}
                        if(k == '#'){ confirm = 2;}
                        if(confirm == 1){
                            confirm = 0;
                            break;
                        }
                        else if(confirm == 2){
                            confirm = 0;
                            pre_interface = -1;
                            selected_interface = 1;
                            return;
                        }
                    }

                    selected_mode = 1;
                    selected_phase = 1;
                    hour = 0;
                    minute = 0;
                    second = 0;
                    limit_sec = phase_duration_sec[selected_phase];
                    limit_min = phase_duration_min[selected_phase];
                    limit_hour = phase_duration_hour[selected_phase];

                    home_limit_time_str = time_to_str(limit_hour, limit_min, limit_sec);
                    cur_time_flag = true;

                    limit_db = phase_temp[selected_phase-1][hour][0];
                    limit_wb = phase_temp[selected_phase-1][hour][1];

                    set_dry_temp_str = String(limit_db);
                    set_wet_temp_str = String(limit_wb);
                    pre_interface = -1;
                    selected_interface = 1;

                    writeIntToEEPROM(EEPROM, flag1, 0);
                    writeIntToEEPROM(EEPROM, flag2, 0);
                    delay(10);
                }
                // PHASE
                else if(key == 'C'){
                    selected_interface = 3;
                }
                // RESET
                else if(key == 'D'){
                    // Confirmation
                    confirmation_screen();

                    delay(10);
                    while(1){
                        char k = keypad.getKey();
                        if(k == '*'){ confirm = 1;}
                        if(k == '#'){ confirm = 2;}
                        if(confirm == 1){
                            confirm = 0;
                            break;
                        }
                        else if(confirm == 2){
                            confirm = 0;
                            pre_interface = -1;
                            selected_interface = 1;
                            return;
                        }
                    }

                    writeIntToEEPROM(EEPROM, flag1, 0);
                    writeIntToEEPROM(EEPROM, flag2, 0);
                    delay(10);
                    ESP.restart();
                }
            }
            else if(selected_interface == 2){
                // Home
                if(key == 'A'){
                    selected_interface = 1;
                }

                // Start
                else if(key == 'B'){
                    // Confirmation
                    confirmation_screen();

                    delay(10);
                    while(1){
                        char k = keypad.getKey();
                        if(k == '*'){ confirm = 1;}
                        if(k == '#'){ confirm = 2;}
                        if(confirm == 1){
                            confirm = 0;
                            break;
                        }
                        else if(confirm == 2){
                            confirm = 0;
                            pre_interface = -1;
                            selected_interface = 1;
                            return;
                        }
                    }

                    // Start process

                    int t_hour = 0;
                    int t_min = 0;
                    int t_sec = 0;
                    int t_db = 0;
                    int t_wb = 0;
                    // working code
                    for(int i=0; i<manual_dry_temp_str.length(); i++){
                        int x = manual_dry_temp_str[i] - '0';
                        t_db = t_db * 10;
                        t_db += x;
                    }
                    for(int i=0; i<manual_wet_temp_str.length(); i++){
                        int x = manual_wet_temp_str[i] - '0';
                        t_wb = t_wb*10;
                        t_wb += x;
                    }

                    selected_interface = 1;
                    pre_interface = -1;
                    limit_db = t_db;
                    limit_wb = t_wb;
                    hour = 0;
                    minute = 0;
                    second = 0;
                    selected_mode = 3;
                    cur_time_flag = true;
                    selected_phase = 0;
                    set_dry_temp_str = String(limit_db);
                    set_wet_temp_str = String(limit_wb);

                    writeIntToEEPROM(EEPROM, manual_dry_temp1, limit_db);
                    writeIntToEEPROM(EEPROM, manual_wet_temp1, limit_wb);

                    writeIntToEEPROM(EEPROM, manual_dry_temp2, limit_db);
                    writeIntToEEPROM(EEPROM, manual_wet_temp2, limit_wb);

                    writeIntToEEPROM(EEPROM, flag1, 0);
                    writeIntToEEPROM(EEPROM, flag2, 0);
                    delay(10);
                    delay(300);
                }

                // Backspace
                else if(key == 'C'){
                    if(selected_manual_field == 1){
                        if(manual_dry_temp_str.length() > 0){
                            manual_dry_temp_str = manual_dry_temp_str.substring(0, manual_dry_temp_str.length() - 1);
                        }
                    }
                    if(selected_manual_field == 2){
                        if(manual_wet_temp_str.length() > 0){
                            manual_wet_temp_str = manual_wet_temp_str.substring(0, manual_wet_temp_str.length() - 1);
                        }
                    }
                    if(selected_manual_field == 3){
                        if(manual_time_str.length() > 0){
                            manual_time_str = manual_time_str.substring(0, manual_time_str.length() - 1);
                        }
                    }
                    field_change_update();

                    delay(300);
                }

                // Field change
                else if(key == 'D'){
                    selected_manual_field++;
                    if(selected_manual_field > 2) selected_manual_field = 1;
                    field_change_update();

                    delay(300);
                }

                if(key == '1') {
                    update_field('1');
                    delay(300);
                }
                if(key == '2') {
                    update_field('2');
                    delay(300);
                }
                if(key == '3') {
                    update_field('3');
                    delay(300);
                }
                if(key == '4') {
                    update_field('4');
                    delay(300);
                }
                if(key == '5') {
                    update_field('5');
                    delay(300);
                }
                if(key == '6') {
                    update_field('6');
                    delay(300);
                }
                if(key == '7') {
                    update_field('7');
                    delay(300);
                }
                if(key == '8') {
                    update_field('8');
                    delay(300);
                }
                if(key == '9') {
                    update_field('9');
                    delay(300);
                }
                if(key == '0') {
                    update_field('0');
                    delay(300);
                }
            }
            else if(selected_interface == 3){
                // Select phase 1
                if(key == '1'){
                    temp_phase_select = 1;
                    update_phase_field();
                    update_phase_time_field();
                    delay(300);
                }

                // Select phase 2
                else if(key == '2'){
                    temp_phase_select = 2;
                    update_phase_field();
                    update_phase_time_field();
                    delay(300);
                }

                // Select phase 3
                else if(key == '3'){
                    temp_phase_select = 3;
                    update_phase_field();
                    update_phase_time_field();
                    delay(300);
                }

                // Select phase 4
                else if(key == '4'){
                    temp_phase_select = 4;
                    update_phase_field();
                    update_phase_time_field();
                    delay(300);
                }

                // Start key
                else if(key == 'A'){
                    // do some start process
                    // Confirmation
                    confirmation_screen();

                    delay(10);
                    while(1){
                        char k = keypad.getKey();
                        if(k == '*'){ confirm = 1;}
                        if(k == '#'){ confirm = 2;}
                        if(confirm == 1){
                            confirm = 0;
                            break;
                        }
                        else if(confirm == 2){
                            confirm = 0;
                            pre_interface = -1;
                            selected_interface = 1;
                            return;
                        }
                    }

                    selected_phase = temp_phase_select;
                    selected_mode = 2;
                    pre_interface = -1;
                    selected_interface = 1;
                    temp_phase_select = 0;
                    cur_time_flag = true;
                    hour = 0;
                    minute = 0;
                    second = 0;
                    limit_hour = phase_duration_hour[selected_phase];
                    limit_min = phase_duration_min[selected_phase];
                    limit_sec = phase_duration_sec[selected_phase];

                    writeIntToEEPROM(EEPROM, flag1, 0);
                    writeIntToEEPROM(EEPROM, flag2, 0);
                    delay(10);

                    delay(300);
                }

                // Home key
                else if(key == 'B'){
                    selected_interface = 1;
                    delay(300);
                }
            }
            break;
    }
}

// Add this new function
void handleStateMachine() {
    const unsigned long TIMEOUT = 30000; // 30 second timeout

    switch(currentState) {
        case STATE_IDLE:
            // Normal operation
            break;

        case STATE_CONFIRM_WAIT:
            confirm_touch();
            if(confirm == 1) {
                confirm = 0;
                currentState = STATE_AUTO_PROCESS;
            }
            else if(confirm == 2) {
                confirm = 0;
                pre_interface = -1;
                selected_interface = 1;
                currentState = STATE_IDLE;
            }
            else if(millis() - stateStartTime > TIMEOUT) {
                // Timeout occurred
                confirm = 0;
                pre_interface = -1;
                selected_interface = 1;
                currentState = STATE_IDLE;
            }
            break;

        case STATE_AUTO_PROCESS:
            // Process auto mode
            Serial.println("Starting Auto Process...");
            hour = 0;
            minute = 0;
            second = 0;
            selected_mode = 1;
            selected_phase = 1;
            limit_sec = phase_duration_sec[selected_phase];
            limit_min = phase_duration_min[selected_phase];
            limit_hour = phase_duration_hour[selected_phase];

            home_limit_time_str = time_to_str(limit_hour, limit_min, limit_sec);
            cur_time_flag = true;

            limit_db = phase_temp[selected_phase-1][hour][0];
            limit_wb = phase_temp[selected_phase-1][hour][1];

            set_dry_temp_str = String(limit_db);
            set_wet_temp_str = String(limit_wb);

            writeIntToEEPROM(EEPROM, flag1, 0);
            writeIntToEEPROM(EEPROM, flag2, 0);

            currentState = STATE_IDLE; // Return to idle after auto process
            break;

        case STATE_RESET_PROCESS:
            confirm_touch();
            if(confirm == 1) {
                writeIntToEEPROM(EEPROM, flag1, 0);
                writeIntToEEPROM(EEPROM, flag2, 0);
                ESP.restart();
            }
            else if(confirm == 2 || millis() - stateStartTime > TIMEOUT) {
                confirm = 0;
                selected_interface = 1;
                currentState = STATE_IDLE;
            }
            break;
    }
}

void setup() {
    limit_db = 0;
    limit_wb = 0;

    delay(1000);
    Serial.begin(115200);
    Wire.begin();
    tft.init();
    tft.setRotation(0);

    tft.fillScreen(TFT_BLACK);

    check_eeprom();

    keypad.addEventListener(keypadEvent);

    touch_calibrate();

    pinMode(relay1, OUTPUT);
    pinMode(relay2, OUTPUT);
    pinMode(buzzer, OUTPUT);
    digitalWrite(buzzer, LOW);

    stored_selected_phase = readIntFromEEPROM(EEPROM, selected_phase1);
}

void loop() {
    cur_time = millis();

    //Handle temperature phase updates
    if(cur_time_flag && selected_interface == 1 && (selected_mode == 1 || selected_mode == 2) && alarmable){
        limit_db = phase_temp[selected_phase-1][hour][0];
        limit_wb = phase_temp[selected_phase-1][hour][1];
        set_dry_temp_str = String(limit_db);
        set_wet_temp_str = String(limit_wb);
    }

 

    // Core interface and control handling
    interface_control();
    touch_control();
    handleStateMachine();  // Added state machine handler

    // Handle EEPROM updates
    if(cur_time - pre_eeprom_time > 60000 && selected_interface == 1 && cur_time_flag){
        handleEEPROMUpdate();
    }

    // Handle keypad and phase control
    keypad.getKey();
    phase_control();

    // Temperature measurement and control
    temp_measure();
    handleTemperatureSensors();
    temp_control();

    // Timer updates
    if(cur_time - pre_time >= 900 && selected_interface == 1 && cur_time_flag){
        handleTimerUpdate();
    }
}


