#include <SPI.h>
#include <TFT_eSPI.h>
#include <Adafruit_GFX.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
// TFT_eSPI tft = TFT_eSPI(240, 240);
TFT_eSPI tft = TFT_eSPI(); 
const char* ssid = "HUAWEI-AA";                // 替换为你的Wi-Fi SSID
const char* password = "m511m511m511";
const char* url = "http://10.8.4.240:7575/api/almanac"; // 替换为你的API接口
void dis(){
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    WiFiClient client;
    http.begin(client,url);  // 指定API的URL
    int httpCode = http.GET(); // 发送GET请求

    if (httpCode > 0) { // 检查返回的HTTP状态码
      String payload = http.getString(); // 获取响应内容
      Serial.println("响应内容: " + payload);

      // 解析JSON数据
      DynamicJsonDocument doc(1024); // 创建一个动态JSON文档
      deserializeJson(doc, payload); // 反序列化JSON数据

      // 假设你的JSON数据中有一个叫 "key" 的字段
      const char* gregorian_date = doc["gregorian_date"]; // 根据实际的JSON结构修改
      const char* lunar_date = doc["lunar_date"];
      const char* yi = doc["yi"];
      const char* ji = doc["ji"];
      const char* shen_wei = doc["shen_wei"];
      tft.println(String(gregorian_date));
      tft.println(String(lunar_date));
      tft.println(String(yi));
      tft.println(String(ji));
      tft.println(String(shen_wei));

    } else {
      Serial.println("请求失败，状态码: " + String(httpCode));
    }
    http.end(); // 清理
  }
}
void setup() 
{
  Serial.begin(115200);
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.fillScreen(TFT_BLACK);
  delay(500);
  WiFi.begin(ssid, password);
  Serial.print("正在连接到WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("连接成功!");
}
void loop()
{
  dis();
  delay(1000);
  tft.fillScreen(TFT_BLACK);
}