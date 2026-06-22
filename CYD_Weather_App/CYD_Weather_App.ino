#include <lvgl.h>
#include "ui.h"  
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Skärmpinnar för touch
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

const char *ssid = "WIFI-NAME";
const char *password = "WIFI-PASSWORD";

String latitude = "59.3793";
String longitude = "13.5036";
// Enter your location
String location = "Karlstad";
// Type the timezone you want to get the time for
String timezone = "Europe/Stockholm";


String current_date;
String last_weather_update;
String temperature;
String humidity;
String datetime;
int is_day;
int weather_code = 0;
String weather_description;

String temperature_unit = "";
const char degree_symbol[] = "\u00B0C";

uint32_t last_weather_update_time = 0;
const uint32_t weather_interval = 600000;



SPIClass touchscreenSpi = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

uint16_t touchScreenMinimumX = 200, touchScreenMaximumX = 3700, touchScreenMinimumY = 240, touchScreenMaximumY = 3800;

/* Screen size */
#define TFT_HOR_RES 320
#define TFT_VER_RES 240

#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10 * (LV_COLOR_DEPTH / 8))

#if LV_USE_LOG != 0
void my_print(lv_log_level_t level, const char *buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}
#endif


void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);


  tft.setSwapBytes(true);
  tft.pushColors((uint16_t *)px_map, w * h, true);

  tft.endWrite();
  lv_disp_flush_ready(disp);
}

void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data) {
  if (touchscreen.touched()) {
    TS_Point p = touchscreen.getPoint();

    if (p.x < touchScreenMinimumX) touchScreenMinimumX = p.x;
    if (p.x > touchScreenMaximumX) touchScreenMaximumX = p.x;
    if (p.y < touchScreenMinimumY) touchScreenMinimumY = p.y;
    if (p.y > touchScreenMaximumY) touchScreenMaximumY = p.y;


    data->point.x = map(p.x, touchScreenMinimumX, touchScreenMaximumX, 0, TFT_HOR_RES);
    data->point.y = map(p.y, touchScreenMinimumY, touchScreenMaximumY, 0, TFT_VER_RES);
    data->state = LV_INDEV_STATE_PRESSED;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

lv_indev_t *indev;
uint8_t *draw_buf;
uint32_t lastTick = 0;


void setup() {
  Serial.begin(115200);

  String LVGL_Arduino = "LVGL demo V";
  LVGL_Arduino += String(lv_version_major()) + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.println(LVGL_Arduino);


  tft.init();
  tft.setRotation(1);         
  tft.fillScreen(TFT_BLACK);  


  touchscreenSpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSpi);
  touchscreen.setRotation(1);  

  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nConnected to Wi-Fi network with IP Address: ");
  Serial.println(WiFi.localIP());
  


  lv_init();


  draw_buf = new uint8_t[DRAW_BUF_SIZE];

  lv_display_t *disp = lv_display_create(TFT_HOR_RES, TFT_VER_RES);

  lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);


  lv_display_set_flush_cb(disp, my_disp_flush);
  lv_display_set_buffers(disp, draw_buf, NULL, DRAW_BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);


  indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, my_touchpad_read);

  Serial.println("LVGL Setup done");
  tft.invertDisplay(true);

  ui_init();

  lastTick = millis();
  get_weather_data();

  last_weather_update_time = millis();
}

void get_weather_data() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    // Construct the API endpoint
    String url = String("http://api.open-meteo.com/v1/forecast?latitude=" + latitude + "&longitude=" + longitude + "&current=temperature_2m,relative_humidity_2m,is_day,precipitation,rain,weather_code" + temperature_unit + "&timezone=" + timezone + "&forecast_days=1");
    http.begin(url);
    int httpCode = http.GET();  // Make the GET request

    if (httpCode > 0) {
      // Check for the response
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        //Serial.println("Request information:");
        //Serial.println(payload);
        // Parse the JSON to extract the time
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);
        if (!error) {
          const char *datetime = doc["current"]["time"];
          temperature = String(doc["current"]["temperature_2m"]);
          humidity = String(doc["current"]["relative_humidity_2m"]);
          is_day = String(doc["current"]["is_day"]).toInt();
          weather_code = String(doc["current"]["weather_code"]).toInt();
          /*Serial.println(temperature);
          Serial.println(humidity);
          Serial.println(is_day);
          Serial.println(weather_code);
          Serial.println(String(timezone));*/
          // Split the datetime into date and time
          String datetime_str = String(datetime);
          int splitIndex = datetime_str.indexOf('T');
          current_date = datetime_str.substring(0, splitIndex);
          last_weather_update = datetime_str.substring(splitIndex + 1, splitIndex + 9);  // Extract time portion
          set_weather_description();
        } else {
          Serial.print("deserializeJson() failed: ");
          Serial.println(error.c_str());
        }
      } else {
        Serial.println("Failed");
      }
    } else {
      Serial.printf("GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();  // Close connection
  } else {
    Serial.println("Not connected to Wi-Fi");
  }
}

void set_weather_description() {

  lv_label_set_text_fmt(objects.temp_text, "%s%s with %s%% humidity.", temperature.c_str(), degree_symbol, humidity.c_str());
  lv_label_set_text_fmt(objects.city_string, "Temperature in %s", location.c_str() );
  lv_label_set_text_fmt(objects.updated_text, "Updated: %s", last_weather_update.c_str() );

  Serial.println(last_weather_update.c_str());
}



void loop() {

  uint32_t current_time = millis();
  lv_tick_inc(current_time - lastTick);
  lastTick = current_time;

  //TIMER: Every 10 minute
  if (current_time - last_weather_update_time >= weather_interval) {
    last_weather_update_time = current_time; // reset timer
    Serial.println("10 minutes has passed, updating weather data.");
    get_weather_data();
  }


  lv_timer_handler();
  ui_tick();
  delay(5);
}

