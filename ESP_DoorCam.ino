// TTGO Camera Plus (OV2640 + TFT ST7789)
// Compile as Board AI Thinker ESP32-CAM

#include "esp_camera.h"
#include <WiFi.h>

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#define TFT_CS        12
#define TFT_DC        15
#define TFT_BACKLIGHT  2
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, 19, 21, -1);
//
// WARNING!!! Make sure that you have either selected ESP32 Wrover Module,
//            or another board which has PSRAM enabled
//

// Select camera model
#define CAMERA_MODEL_TTGO

#include "camera_pins.h"

// WiFi Credentials
const char* ssid = "";
const char* password = "";

void startCameraServer();

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  Serial.println(F("BOOT"));
  WiFi.mode(WIFI_OFF);    //This also works

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
  //init with high specs to pre-allocate larger buffers
  if (psramFound()) {
    // FRAMESIZE_UXGA
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  pinMode(TFT_BACKLIGHT, OUTPUT);
  digitalWrite(TFT_BACKLIGHT, LOW); // Backlight on

  tft.init(240, 240);           // Init ST7789 240x240
  tft.setFont();
  tft.setRotation(2);
  tft.fillScreen(0x0000);
  tft.setTextColor(0xFFFF);
  tft.setTextSize(2);


  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  tft.print(F("WiFi"));

  startCameraServer();
  tft.print(F("WWW"));
  digitalWrite(TFT_BACKLIGHT, HIGH); // Backlight on

  Serial.print(F("Camera Ready! Use 'http://"));
  Serial.print(WiFi.localIP());

  // set window / digital zoom
  //s->set_window_size(s, 1600 / 8 - resolution[CAM_FRAMESIZE][0] / 2, 1200 / 8 - resolution[CAM_FRAMESIZE][1] / 2, resolution[CAM_FRAMESIZE][0], resolution[CAM_FRAMESIZE][1]);

  sensor_t * s = esp_camera_sensor_get();
  s->set_hmirror(s, true);
  s->set_vflip(s, true);
  s->set_gainceiling(s, (gainceiling_t)128);
  //s->set_framesize(s, (framesize_t)7);
}

void loop() {
  // put your main code here, to run repeatedly:
  //delay(10000);
  yield();
}
