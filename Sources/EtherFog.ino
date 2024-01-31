/*
  ETHERFOG (WiFi Pollution Monitor) (c) invpe 2k24
*/
#include <unordered_set>
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <esp_task_wdt.h>
#include "esp_wifi.h"
/*--------------*/
/////////////////////////////////
// GridShell (optional)
/////////////////////////////////
// #define GRIDSHELL "your_grid_id" 
#ifdef GRIDSHELL
#include <ctime>
#include "SPIFFS.h"
#include "CGridShell.h"
#define WIFI "YOUR_WIFI"
#define WIFIP "YOUR_WIFI_PASSWORD"
#define LAT "LATITUDE"
#define LON "LONGITUDE"
#define TELEMETRY_MINUTES 60000ULL * 30
uint64_t uiTelemetrySend = 0;
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;
#endif
/*--------------*/
#define ADA_PIN 27
#define ADA_NUMPIXELS 1
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(ADA_NUMPIXELS, ADA_PIN, NEO_GRB + NEO_KHZ800);
/*--------------*/
#define WIFI_CHANNEL_HOP_TIME 250
#define WIFI_CHANNELS 14
#define WINDOW_SIZE 10
uint32_t uiTotalCount = 0;
int iRSSIMAX = -100;
int iRSSIMIN = 100;
std::unordered_set<std::string> sAccessPoints;
/*--------------*/
#define TYPE_MANAGEMENT 0x00
#define TYPE_CONTROL 0x01
#define TYPE_DATA 0x02
#define SUBTYPE_BEACONS 0x08
struct frame_control {
  uint8_t protocol_version : 2;
  uint8_t type : 2;
  uint8_t subtype : 4;
  uint8_t to_ds : 1;
  uint8_t from_ds : 1;
  uint8_t more_frag : 1;
  uint8_t retry : 1;
  uint8_t pwr_mgt : 1;
  uint8_t more_data : 1;
  uint8_t protected_frame : 1;
  uint8_t order : 1;
};
struct tbeacon {
  struct frame_control fc;
  uint16_t duration;
  uint8_t destination[6];
  uint8_t source[6];
  uint8_t bssid[6];
  uint16_t number;

  uint8_t uiTimestamp[8];
  uint8_t uiBeaconInterval[2];
  uint8_t uiCapability[2];
  // Tag ID = 1b
  // Tag Len = 1b
  // ...
};
/*--------------*/
String GetMACAddress(const int& iType) {
  String strMac;

  uint8_t marray[6] = { 0, 0, 0, 0, 0, 0 };
  char MAC_char[18] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  if (iType == 0)
    WiFi.macAddress(marray);
  else
    WiFi.softAPmacAddress(marray);

  for (int i = 0; i < sizeof(marray); ++i)
    sprintf(MAC_char, "%s%02X", MAC_char, marray[i]);

  return String(MAC_char);
}
/*--------------*/
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
  if (pkt->rx_ctrl.rssi > iRSSIMAX) iRSSIMAX = pkt->rx_ctrl.rssi;
  if (pkt->rx_ctrl.rssi < iRSSIMIN) iRSSIMIN = pkt->rx_ctrl.rssi;
  uiTotalCount++;

  // Get AP count
  uint32_t packetLength = ctrl.sig_len - 4;
  struct ieee80211_hdr* pHeader = (struct ieee80211_hdr*)pkt->payload;

  if (type == WIFI_PKT_MGMT) {

    unsigned int frameControl = ((unsigned int)pkt->payload[1] << 8) + pkt->payload[0];

    uint8_t version = (frameControl & 0b0000000000000011) >> 0;
    uint8_t frameType = (frameControl & 0b0000000000001100) >> 2;
    uint8_t frameSubType = (frameControl & 0b0000000011110000) >> 4;

    if ((frameSubType == SUBTYPE_BEACONS) && (version == 0)) {
      struct tbeacon* pbeacon = (struct tbeacon*)pkt->payload;

      char cMac[32];
      sprintf(cMac, "%02x%02x%02x%02x%02x%02x",
              pbeacon->source[0],
              pbeacon->source[1],
              pbeacon->source[2],
              pbeacon->source[3],
              pbeacon->source[4],
              pbeacon->source[5]);
      sAccessPoints.insert(std::string(cMac));
    }
  }
}
/*--------------*/
void setup() {
  Serial.begin(115200);
#ifdef GRIDSHELL
  while (!SPIFFS.begin()) {
    Serial.println("Formatting");
    SPIFFS.format();
    delay(1000);
  }
#endif

  WiFi.begin("", "");
  pixels.begin();
  pixels.setPixelColor(0, 255, 255, 255);
  pixels.setBrightness(255);
  pixels.show();
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous_rx_cb(&PassiveScan);
  esp_wifi_set_promiscuous(true);
}
/*--------------*/
void loop() {

  // Reset Counters
  uiTotalCount = 0;
  iRSSIMAX = -100;
  iRSSIMIN = 100;
  sAccessPoints.clear();


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
  Serial.println(String(uiTotalCount) + "," + String(average) + "," + String(iRSSIMAX) + "," + String(iRSSIMIN) + "," + String(sAccessPoints.size()));
  pixels.show();

/////////////////////////////////
// GridShell Telemetry submission
/////////////////////////////////
#ifdef GRIDSHELL
  if (millis() - uiTelemetrySend >= TELEMETRY_MINUTES) {
    unsigned long startTime = millis();
    WiFi.begin(WIFI, WIFIP);

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);

      // Check if 10 seconds have passed
      if (millis() - startTime >= 10000) {
        break;
      }
    }

    if (WiFi.status() == WL_CONNECTED) {
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
      tm local_tm;
      getLocalTime(&local_tm);
      time_t timeSinceEpoch = mktime(&local_tm);

      CGridShell::GetInstance().Init(GRIDSHELL, false);
      CGridShell::GetInstance().Tick();
      String strFileSettings = "ETHERFOG" + String(local_tm.tm_year + 1900) + String(local_tm.tm_mon + 1) + String(local_tm.tm_mday) + ",1,";
      String strPayload = String(timeSinceEpoch) + "," + GetMACAddress(0) + "," LAT "," LON "," + String(average) + "," + String(iRSSIMAX) + "," + String(iRSSIMIN) + "," + String(sAccessPoints.size()) + "\n";
      String strTaskPayload = strFileSettings + CGridShell::GetInstance().EncodeBase64(strPayload) + ",";
      CGridShell::GetInstance().AddTask("writedfs", strTaskPayload);
      CGridShell::GetInstance().Stop();

      //
      WiFi.disconnect();
    }

    uiTelemetrySend = millis();
  }
#endif
}
