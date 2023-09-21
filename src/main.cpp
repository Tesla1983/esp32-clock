#include <Wire.h>
#include <RTClib.h>
#include <TFT_eSPI.h>
#include <NTPClient.h>
#include <WiFi.h>
#include <simkai22.h>
#include <HTTPClient.h>
#include <Adafruit_AHTX0.h>
#define WIDTH  160
#define HEIGHT 128





Adafruit_AHTX0 aht;
RTC_DS3231 rtc;
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft); 
bool timeWritten = false;
const char* daysOfTheWeek[7] = {"周日", "周一", "周二", "周三", "周四", "周五", "周六"};
const char *ssid = "you ssid";
const char *password = "you password";
const char* ntpServer = "ntp1.aliyun.com";





float humidity; 
DateTime dateTime;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

IPAddress dnsserver(114, 114, 114, 114);

bool adjustTimeWithNTP = true;




void updateTimeDisplay(float temperature) {
  dateTime = rtc.now(); // 获取当前日期和时间

  int hour = dateTime.hour();
  int minute = dateTime.minute();
  int second = dateTime.second();

  spr.fillScreen(TFT_BLACK);
  spr.setCursor(0, 0);

  char timeStr[10];
  sprintf(timeStr, "%02d:%02d:%02d", hour, minute, second);

  char dateStr[20];
  sprintf(dateStr, "%04d/%02d/%02d", dateTime.year(), dateTime.month(), dateTime.day());

  char dayOfWeekStr[20];
  sprintf(dayOfWeekStr, "%s", daysOfTheWeek[dateTime.dayOfTheWeek()]);

  spr.drawString(timeStr, 0, 0, 4);
  spr.drawString("日期:", 0, 28, 4);
  spr.drawString(dateStr, 45, 28, 4);
  spr.drawString("温度: " + String(temperature, 1) + "℃", 0, 55, 4);
  
  spr.drawString(dayOfWeekStr, 90, 2, 4);
  spr.drawString("湿度: " + String(humidity, 1) + "%", 0, 80, 4);
  
  
  
  spr.pushSprite(0, 0);
  

  
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  WiFi.config(IPAddress(192, 168, 1, 5), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0), dnsserver);
  
  spr.createSprite(WIDTH, HEIGHT); // 创建一个 160x128 的画布
  tft.init();
  tft.setRotation(1);
  spr.fillScreen(TFT_BLACK);
  spr.loadFont(simkai22);
  spr.setTextSize(7);
  spr.setTextColor(TFT_GREEN);

  

  rtc.begin();
  bool isLostPower = rtc.lostPower(); //检测电池

  if (!rtc.begin()) {
    Serial.println("Could not find RTC!");
  }
  if (isLostPower) {
    Serial.println("The RTC module lost power");
  } else {
    Serial.println("The RTC module is running");
  }
  
  // 初始化AHT20传感器
  aht.begin();
}

void loop() {
  if (adjustTimeWithNTP) {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Connecting to WiFi...");
    }

    timeClient.begin();
    timeClient.setTimeOffset(28800);
    timeClient.forceUpdate();
    

    DateTime nowNTP = timeClient.getEpochTime();
    rtc.adjust(nowNTP);
    adjustTimeWithNTP = false;
    timeClient.end();
    WiFi.disconnect();
  }
  
  sensors_event_t humidityEvent;
  sensors_event_t tempEvent; 
  aht.getEvent(&humidityEvent,&tempEvent); 
  humidity = humidityEvent.relative_humidity; 


  updateTimeDisplay(tempEvent.temperature);
  
  spr.pushSprite(0,0);
  delay(1000);
}
