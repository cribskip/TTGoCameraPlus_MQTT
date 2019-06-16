// TTGO Camera Plus (OV2640 + TFT ST7789)
// Compile as Board ESP32 Dev Module
// CPU MHz: 240 MHz
// Flash Frequency: 40 MHz
// Flash Mode: DIO
// Flash size: 4MB (32Mb)
// Partition: Huge App
// PSRAM: Enabled

// Used libraries:
// TFT_eSPI
// JPEG_Decoder

// Config for TFT_eSPI (User_Setup.h)
#define ST7789_DRIVER      // Full configuration option, define additional parameters below for this display
#define TFT_CS   12  // Chip select control pin D8
#define TFT_DC   15  // Data Command control pin
#define TFT_RST  -1  // Reset pin (could connect to NodeMCU RST, see next line)
#define TFT_BL 2  // LED back-light (only for ST7789 with backlight control pin)
#define TFT_MISO 22
#define TFT_MOSI 19
#define TFT_SCLK 21
#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
#define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:-.
#define LOAD_FONT8  // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
#define LOAD_GFXFF  // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts
#define SMOOTH_FONT
#define SPI_FREQUENCY  80000000
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY  2500000
#define USE_HSPI_PORT

// Current Performance
// HQVGA, JPEG Decode ~68ms, Total Render time per Frame ~86ms = 11,6 frames/second

#include "esp_camera.h"
#include <WiFi.h>

#include <TFT_eSPI.h>       // Hardware-specific library
TFT_eSPI tft = TFT_eSPI();  // Invoke custom library

// Select camera model
#define CAMERA_MODEL_TTGO

#include "camera_pins.h"

// WiFi Credentials
const char* ssid = "";
const char* password = "";

void startCameraServer();
camera_fb_t * fb = NULL;

void config_camera() {
  if (! psramFound()) {
    while (1) {
      Serial.println(F("YOUR BOARD DOES NOT HAVE PSRAM!"));
    }
  }

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //PIXFORMAT_YUV422;  // Colors incorrect, Picture ok
  //PIXFORMAT_JPEG; // Has to be decoded most of the time
  //PIXFORMAT_RGB565; // Colors incorrect, Picture ok
  //PIXFORMAT_GRAYSCALE; // Colors incorrect, Picture ok

  //FRAMESIZE_UXGA
  //FRAMESIZE_HQVGA; // 240x160
  //FRAMESIZE_QVGA;  // 320x240
  config.frame_size = FRAMESIZE_HQVGA;
  config.jpeg_quality = 10;
  config.fb_count = 2;

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    while (1) {
      Serial.println(F("Camera init failed"));
    }
  }

  // set window / digital zoom
  //s->set_window_size(s, 1600 / 8 - resolution[CAM_FRAMESIZE][0] / 2, 1200 / 8 - resolution[CAM_FRAMESIZE][1] / 2, resolution[CAM_FRAMESIZE][0], resolution[CAM_FRAMESIZE][1]);

  sensor_t * s = esp_camera_sensor_get();
  s->set_hmirror(s, true);
  s->set_vflip(s, true);
  s->set_gainceiling(s, GAINCEILING_128X);
  //s->set_framesize(s, (framesize_t)7);
}

void setup() {
  WiFi.mode(WIFI_OFF);
  pinMode(TFT_BACKLIGHT, OUTPUT);
  digitalWrite(TFT_BACKLIGHT, LOW); // Backlight off

  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  Serial.println(F("BOOT"));
  config_camera();

  tft.begin();
  //tft.setFont();
  //tft.setRotation(2);
  tft.fillScreen(TFT_DARKGREY);
  //tft.setTextColor(0xFFFF);
  //tft.setTextSize(2);

  WiFi.begin(ssid, password);
  //while (WiFi.status() != WL_CONNECTED) {
  delay(250);
  Serial.print(".");
  //}
  Serial.print(F("WiFi"));

  startCameraServer();
  Serial.print(F("WWW"));
  digitalWrite(TFT_BACKLIGHT, HIGH); // Backlight on
  delay(1000);
  
  Serial.print(F("Camera Ready! Use 'http://"));
  //Serial.print(WiFi.localIP());
}

void localstream() {
  fb = esp_camera_fb_get();
  esp_err_t res = ESP_OK;
  
  if (!fb) {
    Serial.println(F("Camera capture failed"));
  }

  if (fb->format == PIXFORMAT_JPEG) {
    drawArrayJpeg(fb->buf, fb->len, 0, 0);
  }
  esp_camera_fb_return(fb);
}

void loop() {
  // put your main code here, to run repeatedly:
  //delay(10000);
  yield();
  //if (!fb) fb = esp_camera_fb_get();
  localstream();
}
