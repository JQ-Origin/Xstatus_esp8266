#include <SPI.h>
#include <TFT_eSPI.h>
#include <Adafruit_GFX.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <TaskScheduler.h>

const char *ssid = "HUAWEI-AA"; // 替换为你的Wi-Fi SSID
const char *password = "m511m511m511";
const char *openwrt_url = "http://10.8.4.254:5000/network_speed/eth1"; // 替换为你的API接口
String bark_base_url = "http://60.204.209.208:40010/uzz9KNyakPoRGPoaQ6iGzY/网络速度异常!!!/";

unsigned long bark_previousTime = millis();
bool speed_status_tx = false;
bool speed_status_rx = false;
String rx_speed;
String tx_speed;

void display_openwrt();
void openwrt_ui();
void bark_send();

TFT_eSPI tft = TFT_eSPI();
Scheduler Sch;
Task t_display_openwrt(2000, TASK_FOREVER, &display_openwrt);
Task t_bark_send(1000, TASK_FOREVER, &bark_send);

void setup()
{
  Serial.begin(115200);
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextPadding(16);
  tft.setTextFont(1);
  delay(500);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  tft.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
    tft.print(".");
  }
  Serial.println(WiFi.localIP());
  Serial.println("Connection successful!");
  tft.print("\n");
  tft.println("Connection successful!\n");
  tft.print("IP: " + WiFi.localIP().toString());
  Sch.init();
  Sch.addTask(t_display_openwrt);
  Sch.addTask(t_bark_send);
  t_display_openwrt.enable();
  t_bark_send.enable();
  delay(3000);
  tft.fillScreen(TFT_BLACK);
  openwrt_ui();
}
void loop()
{
  Sch.execute();
}

void display_openwrt()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    WiFiClient client;
    http.begin(client, openwrt_url); // 指定API的openwrt_url
    int httpCode = http.GET();       // 发送GET请求

    if (httpCode > 0)
    {                                    // 检查返回的HTTP状态码
      String payload = http.getString(); // 获取响应内容
      Serial.println("响应内容: " + payload);

      // 解析JSON数据
      JsonDocument doc;              // 创建一个静态JSON文档
      deserializeJson(doc, payload); // 反序列化JSON数据

      // 获取速度字符串
      String receive_speed_str = doc["receive_speed"].as<String>();
      String transmit_speed_str = doc["transmit_speed"].as<String>();
      rx_speed = receive_speed_str;
      tx_speed = transmit_speed_str;

      // 去掉单位并转换为浮点数
      String receive_speed_units = receive_speed_str.substring(receive_speed_str.indexOf(" ") + 1);
      String transmit_speed_units = transmit_speed_str.substring(transmit_speed_str.indexOf(" ") + 1);
      float receive_speed = receive_speed_str.substring(0, receive_speed_str.indexOf(" ")).toFloat();
      float transmit_speed = transmit_speed_str.substring(0, transmit_speed_str.indexOf(" ")).toFloat();

      // 判断速度是否超过阈值
      if (receive_speed > 40.0 && receive_speed_units == "MB/s")
      {
        Serial.println("超速");
        tft.fillCircle(14, 37, 10, TFT_RED);
        speed_status_rx = true;
      }
      else if (receive_speed > 25.0 && receive_speed_units == "MB/s")
      {
        tft.fillCircle(14, 37, 10, TFT_YELLOW);
        speed_status_rx = true;
      }
      else
      {
        tft.fillCircle(14, 37, 10, TFT_GREEN);
        speed_status_rx = false;
      }

      if (transmit_speed > 3.5 && transmit_speed_units == "MB/s")
      {
        Serial.println("超速");
        tft.fillCircle(14, 63, 10, TFT_RED);
        speed_status_tx = true;
      }
      else if (transmit_speed > 2.0 && transmit_speed_units == "MB/s")
      {
        tft.fillCircle(14, 63, 10, TFT_YELLOW);
        speed_status_tx = true;
      }
      else
      {
        tft.fillCircle(14, 63, 10, TFT_GREEN);
        speed_status_tx = false;
      }
      // Serial.println("RX:"+String(receive_speed)+"\n"+"TX:"+String(transmit_speed));
      // Serial.println("RX:"+String(receive_speed_units)+"\n"+"TX:"+String(transmit_speed_units));

      tft.setCursor(27, 35);
      tft.fillRect(26, 26, 88, 23, TFT_BLACK);
      tft.print("RX:" + receive_speed_str);
      tft.setCursor(27, 60);
      tft.fillRect(26, 51, 88, 23, TFT_BLACK);
      tft.print("TX:" + transmit_speed_str);
    }
    else
    {
      Serial.println("HTTP请求失败");
    }
    http.end(); // 关闭HTTP连接
  }
}
void openwrt_ui()
{
  tft.drawRoundRect(2, 2, 158, 126, 10, TFT_WHITE);
  tft.drawFastHLine(2, 25, 158, TFT_WHITE);
  tft.drawFastVLine(117, 25, 126, TFT_WHITE);

  tft.drawFastHLine(2, 50, 117, TFT_WHITE);
  tft.drawFastHLine(2, 75, 117, TFT_WHITE);
  tft.drawFastHLine(2, 100, 117, TFT_WHITE);
  tft.fillCircle(14, 37, 10, TFT_LIGHTGREY);  // 1
  tft.fillCircle(14, 63, 10, TFT_LIGHTGREY);  // 2
  tft.fillCircle(14, 87, 10, TFT_LIGHTGREY);  // 3
  tft.fillCircle(14, 113, 10, TFT_LIGHTGREY); // 4

  tft.drawString("OpenWRT", 56, 7, 2);
}
void bark_send()
{
  if (bark_previousTime < millis() - 60000 && (speed_status_rx || speed_status_tx))
  {
    HTTPClient http;
    WiFiClient client;
    rx_speed.replace('/', 'p');
    tx_speed.replace('/', 'p');
    String bark_url_str = bark_base_url + "RX:" + rx_speed + "|" + "TX:" + tx_speed;
    const char *bark_url = bark_url_str.c_str();
    http.begin(client, bark_url); // 指定API的openwrt_url
    int httpCode = http.GET();    // 发送GET请求
    if (httpCode > 0)
    {
      Serial.println("警告已推送");
    }
    bark_previousTime = millis();
  }
}