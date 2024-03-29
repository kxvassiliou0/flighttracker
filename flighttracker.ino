#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>  // For portability, meaning the chosen network can be changed

const char* rapidApiKey = "XXX KEY XXX";
const char* rapidApiHost = "flight-radar1.p.rapidapi.com";

const char* flightRadarApiUrl = "/flights/list-in-boundary"; // Retrieves the relevant data

// Creating a boundary based on co-ordinates for Airbus Headquarters, Blagnac
const float blLat = 43.575328;  // Bottom left latitude
const float blLng = 1.313557;  // Bottom left longitude
const float trLat = 43.659844;  // Top right latitude
const float trLng = 1.435969;  // Top right longitude
const int limit = 1;            // Number of flights to retrieve at a time


const size_t JSON_BUFFER_SIZE = 2048;  // Define the JSON buffer size

// Setting up libraries for the OLED display
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128     // OLED display width, in pixels
#define SCREEN_HEIGHT 64     // OLED display height, in pixels
#define OLED_RESET -1        // Reset pin
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int buttonPin = 15;  // Pin for the momentary push button
const int ledPin = 4;
bool buttonState = false;    // State of the button
bool buttonPressed = false;  // Flag indicating if the button is pressed

String accumulatedResponse = "";  // Accumulate the complete JSON response

// 'A320N', 128x64px - A320 NEO Graphic
const unsigned char bmpA320N[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x01, 0xc0, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x30, 0x00, 0x20, 0x00, 0x00,
  0x00, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x01, 0xff, 0x03, 0xfe, 0x00, 0x00,
  0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0x40, 0xe0, 0x00, 0x00, 0x03, 0xff, 0x87, 0xff, 0x00, 0x00,
  0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x40, 0xe0, 0x00, 0x00, 0x07, 0xff, 0xdf, 0xff, 0x80, 0x00,
  0x00, 0x00, 0x3c, 0x1c, 0x00, 0x00, 0x60, 0xe0, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xc0, 0x00,
  0x00, 0x00, 0x3f, 0xfc, 0x00, 0x00, 0x7f, 0xf0, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xe0, 0x00,
  0x00, 0x00, 0x3f, 0xfc, 0x00, 0x00, 0x7f, 0xf0, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xe0, 0x00,
  0x00, 0x18, 0xff, 0xf0, 0x00, 0x00, 0x3f, 0xfc, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xe0, 0x00,
  0x00, 0x0f, 0xff, 0x80, 0x00, 0x00, 0x03, 0xff, 0x80, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xf0, 0x00,
  0x00, 0x07, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xc0, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xf0, 0x00,
  0x00, 0x07, 0x80, 0x00, 0x00, 0x00, 0x00, 0x07, 0xc0, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xf0, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xe0, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xe0, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xe0, 0x00,
  0x00, 0x0f, 0xff, 0xf8, 0x00, 0x00, 0x7f, 0xff, 0xc0, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xc0, 0x00,
  0x00, 0x0f, 0xff, 0xf8, 0x00, 0x00, 0x7f, 0xff, 0xc0, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xc0, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xfe, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xfc, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xf8, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xf0, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xe0, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xc0, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0x80, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xfe, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x30, 0x60, 0xe0, 0x00, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x38, 0xf9, 0xf1, 0xf3, 0x98, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x3c, 0x59, 0xbb, 0xb3, 0x90, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x6c, 0x19, 0x33, 0x33, 0x90, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x6c, 0x78, 0x33, 0x33, 0xd0, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x7c, 0x18, 0xe3, 0x32, 0xf0, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xfe, 0x19, 0xc3, 0x33, 0x70, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xcf, 0xfb, 0xf3, 0xf3, 0x70, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xc6, 0xf1, 0xf1, 0xe3, 0x30, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
// 'ANY', 128x66px
const unsigned char bmpANY[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xc0,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xe0,
  0x00, 0x01, 0xc0, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xc0,
  0x00, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xc0,
  0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0x40, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xe0, 0x07, 0xc0,
  0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x40, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xc0, 0x03, 0xc0,
  0x00, 0x00, 0x3c, 0x1c, 0x00, 0x00, 0x60, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x80, 0x07, 0xc0,
  0x00, 0x00, 0x3f, 0xfc, 0x00, 0x00, 0x7f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x07, 0x80,
  0x00, 0x00, 0x3f, 0xfc, 0x00, 0x00, 0x7f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x07, 0x80,
  0x00, 0x18, 0xff, 0xf0, 0x00, 0x00, 0x3f, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x07, 0x80,
  0x00, 0x0f, 0xff, 0x80, 0x00, 0x00, 0x03, 0xff, 0x80, 0x00, 0x00, 0x00, 0x7c, 0x00, 0x07, 0x80,
  0x00, 0x07, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xc0, 0x00, 0x00, 0x00, 0xf8, 0x7c, 0x07, 0x80,
  0x00, 0x07, 0x80, 0x00, 0x00, 0x00, 0x00, 0x07, 0xc0, 0x00, 0x00, 0x00, 0xf9, 0xfe, 0x0f, 0x80,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xf1, 0xff, 0x0f, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xe3, 0xff, 0x0f, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xe3, 0xff, 0x0f, 0x00,
  0x00, 0x0f, 0xff, 0xf8, 0x00, 0x00, 0x7f, 0xff, 0xc0, 0x00, 0x00, 0x07, 0xc3, 0xff, 0x0f, 0x00,
  0x00, 0x0f, 0xff, 0xf8, 0x00, 0x00, 0x7f, 0xff, 0xc0, 0x00, 0x00, 0x0f, 0x83, 0xff, 0x1f, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x81, 0xff, 0x1e, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x01, 0xfe, 0x1e, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x7c, 0x1e, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x1e, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x3e, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x3c, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x3c, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xf0, 0x00, 0x00, 0x3c, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xe0, 0x00, 0x00, 0x3c, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xc0, 0x00, 0x00, 0x3c, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xc0, 0x00, 0x00, 0x7c, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xc0, 0x00, 0x00, 0x78, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xf8, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xf8, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xf8, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xf0, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
// 'A350', 128x64px - A350 Graphic
const unsigned char bmpA350[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x30, 0x00, 0x20, 0x00, 0x00,
  0x00, 0x01, 0xc0, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x01, 0xff, 0x03, 0xfe, 0x00, 0x00,
  0x00, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x03, 0xff, 0x87, 0xff, 0x00, 0x00,
  0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0x40, 0xe0, 0x00, 0x00, 0x07, 0xff, 0xdf, 0xff, 0x80, 0x00,
  0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x40, 0xe0, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xc0, 0x00,
  0x00, 0x00, 0x3c, 0x1c, 0x00, 0x00, 0x60, 0xe0, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xe0, 0x00,
  0x00, 0x00, 0x3f, 0xfc, 0x00, 0x00, 0x7f, 0xf0, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xe0, 0x00,
  0x00, 0x00, 0x3f, 0xfc, 0x00, 0x00, 0x7f, 0xf0, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xe0, 0x00,
  0x00, 0x18, 0xff, 0xf0, 0x00, 0x00, 0x3f, 0xfc, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xf0, 0x00,
  0x00, 0x0f, 0xff, 0x80, 0x00, 0x00, 0x03, 0xff, 0x80, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xf0, 0x00,
  0x00, 0x07, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xc0, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xf0, 0x00,
  0x00, 0x07, 0x80, 0x00, 0x00, 0x00, 0x00, 0x07, 0xc0, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xe0, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xe0, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xe0, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xc0, 0x00,
  0x00, 0x0f, 0xff, 0xf8, 0x00, 0x00, 0x7f, 0xff, 0xc0, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xc0, 0x00,
  0x00, 0x0f, 0xff, 0xf8, 0x00, 0x00, 0x7f, 0xff, 0xc0, 0x00, 0x07, 0xff, 0xff, 0xff, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xfe, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xfc, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xf8, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xf0, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xe0, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xc0, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0x80, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xfe, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x1c, 0x1f, 0x8f, 0xc3, 0xf0, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x3e, 0x3f, 0x8f, 0xc7, 0xf8, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x3e, 0x11, 0x8c, 0x07, 0x38, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x76, 0x01, 0x9c, 0x0e, 0x38, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x76, 0x0f, 0x1f, 0x8e, 0x38, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x67, 0x1f, 0x9f, 0xce, 0x38, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0x03, 0x80, 0xee, 0x38, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0x03, 0x80, 0xee, 0x38, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xe7, 0x33, 0x99, 0xcf, 0x70, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0xc3, 0xff, 0x1f, 0xc7, 0xf0, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xc3, 0xbe, 0x1f, 0x83, 0xc0, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
// 'A380', 128x64px - A380 Graphic
const unsigned char bmpA380[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x30, 0x00, 0x20, 0x00, 0x00,
  0x00, 0x01, 0xc0, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x01, 0xff, 0x03, 0xfe, 0x00, 0x00,
  0x00, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x03, 0xff, 0x87, 0xff, 0x00, 0x00,
  0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0x40, 0xe0, 0x00, 0x00, 0x07, 0xff, 0xdf, 0xff, 0x80, 0x00,
  0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x40, 0xe0, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xc0, 0x00,
  0x00, 0x00, 0x3c, 0x1c, 0x00, 0x00, 0x60, 0xe0, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xe0, 0x00,
  0x00, 0x00, 0x3f, 0xfc, 0x00, 0x00, 0x7f, 0xf0, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xe0, 0x00,
  0x00, 0x00, 0x3f, 0xfc, 0x00, 0x00, 0x7f, 0xf0, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xe0, 0x00,
  0x00, 0x18, 0xff, 0xf0, 0x00, 0x00, 0x3f, 0xfc, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xf0, 0x00,
  0x00, 0x0f, 0xff, 0x80, 0x00, 0x00, 0x03, 0xff, 0x80, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xf0, 0x00,
  0x00, 0x07, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xc0, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xf0, 0x00,
  0x00, 0x07, 0x80, 0x00, 0x00, 0x00, 0x00, 0x07, 0xc0, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xe0, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xe0, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xe0, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xc0, 0x00,
  0x00, 0x0f, 0xff, 0xf8, 0x00, 0x00, 0x7f, 0xff, 0xc0, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xc0, 0x00,
  0x00, 0x0f, 0xff, 0xf8, 0x00, 0x00, 0x7f, 0xff, 0xc0, 0x00, 0x07, 0xff, 0xff, 0xff, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xfe, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xfc, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xf8, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xf0, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xe0, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xc0, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0x80, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xfe, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x3c, 0x1f, 0x0f, 0xc3, 0xf0, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x3c, 0x3f, 0x9f, 0xe7, 0xf8, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x3e, 0x13, 0x9c, 0xe7, 0x38, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x7e, 0x03, 0x9c, 0xc7, 0x38, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x6e, 0x1f, 0x0f, 0xce, 0x18, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xe6, 0x1f, 0x8f, 0xce, 0x38, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0x03, 0x9c, 0xee, 0x38, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0x03, 0xb8, 0xee, 0x38, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0xe7, 0x77, 0xbd, 0xc7, 0x78, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0xc7, 0x7f, 0x1f, 0xc7, 0xf0, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0xc3, 0x3e, 0x0f, 0x83, 0xe0, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void printAircraftData(const char* payload) {
  StaticJsonDocument<JSON_BUFFER_SIZE> doc;
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.print("JSON parsing failed: ");
    Serial.println(error.c_str());
    return;
  }

  // Check if the "aircraft" array is empty and continue if not
  if (!doc["aircraft"].isNull() && doc["aircraft"].size() > 0) {
    // Extract relevant data from the JSON
    const char* aircraftType = doc["aircraft"][0][9].as<const char*>();
    const char* flightNo = doc["aircraft"][0][14].as<const char*>();
    const char* source = doc["aircraft"][0][12].as<const char*>();
    const char* destination = doc["aircraft"][0][13].as<const char*>();

    Serial.println(aircraftType);
    Serial.println(flightNo);
    Serial.print(source);
    Serial.print(" --> ");
    Serial.println(destination);

    if (strncmp(aircraftType, "A20N", 4) == 0) {
      // Flash A320neo graphic on-screen
      Serial.println("A320n");
      display.clearDisplay();
      display.setTextColor(WHITE);
      display.setTextSize(1.5);
      display.drawBitmap(0, 0, bmpA320N, 128, 64, WHITE);
      display.setCursor(12, 29);
      display.print(source);
      display.display();
      display.setCursor(49, 29);
      display.print(destination);
      display.display();
      display.setCursor(4, 47);
      display.setTextColor(BLACK);
      display.print(flightNo);
      display.display();
    } else if (strncmp(aircraftType, "A38", 3) == 0) {
      // Flash A380 graphic on-screen
      Serial.println("A380");
      display.clearDisplay();
      display.setTextColor(WHITE);
      display.setTextSize(1.5);
      display.drawBitmap(0, 0, bmpA380, 128, 64, WHITE);
      display.setCursor(12, 29);
      display.print(source);
      display.display();
      display.setCursor(49, 29);
      display.print(destination);
      display.display();
      display.setCursor(4, 47);
      display.setTextColor(BLACK);
      display.print(flightNo);
      display.display();
    } else if (strncmp(aircraftType, "A35", 3) == 0) {
      Serial.println("A350");
      display.clearDisplay();
      display.setTextColor(WHITE);
      display.setTextSize(1.5);
      display.drawBitmap(0, 0, bmpA350, 128, 64, WHITE);
      display.setCursor(12, 29);
      display.print(source);
      display.display();
      display.setCursor(49, 29);
      display.print(destination);
      display.display();
      display.setCursor(4, 47);
      display.setTextColor(BLACK);
      display.print(flightNo);
      display.display();
    } else {
      Serial.println("ANY");
      display.clearDisplay();
      display.setTextColor(WHITE);
      display.setTextSize(1.5);
      display.drawBitmap(0, 0, bmpANY, 128, 64, WHITE);
      display.setCursor(12, 29);
      display.print(source);
      display.display();
      display.setCursor(49, 29);
      display.print(destination);
      display.display();
      display.setCursor(87, 50);
      display.print(aircraftType);
      display.display();
      display.setCursor(4, 50);
      display.setTextColor(BLACK);
      display.print(flightNo);
      display.display();
    }

    Serial.println("Aircraft data displayed on OLED screen");

  } else {
    Serial.println("No aircraft data found");
    display.clearDisplay();
    display.setCursor(7, 30);
    display.setTextColor(WHITE);
    display.setTextSize(1.5);
    display.print("No flights overhead");
    display.display();
  }
}


void setup() {
  Serial.begin(230400);
  Wire.begin(21, 22);

  WiFiManager wm;

  bool res;
  res = wm.autoConnect("FlightRadar", "fuselage");  // password protected ap

  if (!res) {
    Serial.println("Failed to connect");
    // ESP.restart();
  } else {
    // Connected to wifi
    Serial.println("Connection success");
  }

  pinMode(buttonPin, INPUT_PULLUP);  // Set the button pin as INPUT_PULLUP
  pinMode(ledPin, OUTPUT);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  display.display();
  // Clear the buffer
  display.clearDisplay();
}

void loop() {
  for (int i = 0; i < 4; i++) {
    digitalWrite(ledPin, HIGH);  // Turn on the LED
    delay(100);                  // Delay to control the rotation speed
    digitalWrite(ledPin, LOW);   // Turn off the LED
    delay(100);                  // Delay between blinks
  }

  // Read the state of the button
  buttonState = digitalRead(buttonPin);

  // Check if the button is pressed
  if (buttonState == LOW && !buttonPressed) {
    Serial.println("button pressed");
    buttonPressed = true;

    WiFiClientSecure client;
    client.setInsecure();

    if (client.connect(rapidApiHost, 443)) {
      String url = String(flightRadarApiUrl) + "?bl_lat=" + String(blLat) + "&bl_lng=" + String(blLng) + "&tr_lat=" + String(trLat) + "&tr_lng=" + String(trLng) + "&limit=" + String(limit);

      // Make an HTTP request
      client.println("GET " + url + " HTTP/1.1");
      client.println("Host: " + String(rapidApiHost));
      client.println("X-RapidAPI-Key: " + String(rapidApiKey));
      client.println("Connection: close");
      client.println();

      // Read the response
      while (client.connected()) {
        if (client.available()) {
          char c = client.read();
          accumulatedResponse += c;
        }
      }

      // Print the HTTP response code
      Serial.print("HTTP Response Code: ");
      Serial.println(client.parseInt());

      // Find the start and end position of the JSON data
      int startPos = accumulatedResponse.indexOf('{');
      int endPos = accumulatedResponse.lastIndexOf('}');

      if (startPos != -1 && endPos != -1) {
        String response = accumulatedResponse.substring(startPos, endPos + 1);
        printAircraftData(response.c_str());
      } else {
        Serial.println("Invalid JSON response");
      }

      // Clear the accumulated response for the next iteration
      accumulatedResponse = "";
    } else {
      // Print the connection status
      Serial.print("Failed to connect to the server. Connection status: ");
      Serial.println(client.connected());

      delay(5000);  // Wait for 5 seconds before retrying the connection
    }

    delay(120000);  // Wait for 2 minutes between requests to comply with the rate limit
  }

  // Check if the button is released
  if (buttonState == HIGH && buttonPressed) {
    buttonPressed = false;
  }
}
