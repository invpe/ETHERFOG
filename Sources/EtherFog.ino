/*
  ETHERFOG (WiFi Pollution Monitor) (c) invpe 2k24
*/
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <esp_task_wdt.h>
#include "esp_wifi.h"
/*--------------*/
#define ADA_PIN 27
#define ADA_NUMPIXELS 1
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(ADA_NUMPIXELS, ADA_PIN, NEO_GRB + NEO_KHZ800);
/*--------------*/
#define WIFI_CHANNEL_HOP_TIME 250
#define WIFI_CHANNELS 14
#define WINDOW_SIZE 10
uint32_t uiTotalCount = 0;
float calculateMovingAverage(float newValue) {
  static float values[WINDOW_SIZE];
  static int index = 0;
  static float sum = 0;
  static int count = 0;

  // Subtract the oldest value from the sum
  sum -= values[index];
  // Assign the new value to the current index position
  values[index] = newValue;
  // Add the new value to the sum
  sum += newValue;
  // Increment the index and wrap around if necessary
  index = (index + 1) % WINDOW_SIZE;

  // Keep track of how many values we have
  count = min(count + 1, WINDOW_SIZE);

  // Calculate the average based on the sum and the count of values
  return sum / count;
}
/*--------------*/
void PassiveScan(void* buf, wifi_promiscuous_pkt_type_t type) {

  wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
  wifi_pkt_rx_ctrl_t ctrl = (wifi_pkt_rx_ctrl_t)pkt->rx_ctrl;
  uiTotalCount++;
}
/*--------------*/
void setup() {
  Serial.begin(115200);
  WiFi.begin("", "");
  pixels.begin();
  pixels.setPixelColor(0, 255, 255, 255);
  pixels.setBrightness(255);
  pixels.show();
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous_rx_cb(&PassiveScan);
  esp_wifi_set_promiscuous(true);
}

void loop() {
  uiTotalCount = 0;
  for (int a = 1; a <= WIFI_CHANNELS; a++) {
    if (esp_wifi_set_channel(a, WIFI_SECOND_CHAN_NONE) == ESP_OK) {
      yield();
      esp_task_wdt_reset();
    }
    delay(WIFI_CHANNEL_HOP_TIME);
  }

  float average = calculateMovingAverage(uiTotalCount);

  String strScale = "?";
  if (average <= 50) {
    strScale = "Low";
    pixels.setPixelColor(0, 0, 255, 0);
  } else if (average <= 150) {
    strScale = "Moderate";
    pixels.setPixelColor(0, 255, 255, 0);
  } else if (average <= 300) {
    strScale = "High";
    pixels.setPixelColor(0, 255, 0, 0);
  } else {
    strScale = "Very High";
    pixels.setPixelColor(0, 255, 0, 255);
  }
  Serial.println(String(uiTotalCount) + "," + String(average));
  pixels.show();
}
