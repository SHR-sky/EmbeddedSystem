// ESP32数据网页

#include <WiFi.h>
#include <WiFiUdp.h>
#include <Arduino.h>
#include <NTPClient.h>
#include "ArduinoJson.h"
#include <string.h>


// RX TX 连接STM32

// 创建客户端
WiFiClient client;


HardwareSerial MySerial_stm32(1);

// 输入服务端，要求处于同一局域网

const IPAddress serverIP(192, 168, 184, 48);
uint16_t serverPort = 9999;

// 网页请求
void httpRequest(String reqRes);
void parseInfo(WiFiClient client);

// 接入的WIFI密码与名称
const char *ssid = "HEXER";
const char *password = "chafiprc";

// Define NTP Client to get time
const long utcOffsetInSeconds = 28800;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

// 星期
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// 定时获取
// int get_weather_cnt = 0;

const char *host = "api.seniverse.com"; // 将要连接的服务器地址
const int httpPort = 80;                // 将要连接的服务器端口

// 心知天气HTTP请求所需信息
String reqUserKey = "SjF1ffbgYJ8Smyg4v"; // 私钥
String reqLocation = "WuHan";            // 城市
String reqUnit = "c";                    // 摄氏/华氏

// 向心知天气服务器服务器请求信息并对信息进行解析
void httpRequest(String reqRes)
{
    WiFiClient client_weather;

    // 建立http请求信息
    String httpRequest = String("GET ") + reqRes + " HTTP/1.1\r\n" +
                         "Host: " + host + "\r\n" +
                         "Connection: close\r\n\r\n";
    Serial.println(" ");
    Serial.print("Connecting to ");
    Serial.print(host);

    // 尝试连接服务器
    if (client_weather.connect(host, 80))
    {
        Serial.println(" Success!");

        // 向服务器发送http请求信息
        client_weather.print(httpRequest);
        Serial.println("Sending request: ");
        Serial.println(httpRequest);

        // 获取并显示服务器响应状态行
        String status_response = client_weather.readStringUntil('\n');
        Serial.print("status_response: ");
        Serial.println(status_response);

        // 使用find跳过HTTP响应头
        if (client_weather.find("\r\n\r\n"))
        {
            Serial.println("Found Header End. Start Parsing.");
        }

        // 利用ArduinoJson库解析心知天气响应信息
        parseInfo(client_weather);
    }
    else
    {
        Serial.println(" connection failed!");
    }
    // 断开客户端与服务器连接工作
    client_weather.stop();
}

// 利用ArduinoJson库解析心知天气响应信息
void parseInfo(WiFiClient client_weather)
{
    const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + 2 * JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(6) + 230;
    DynamicJsonDocument doc(capacity);

    deserializeJson(doc, client_weather);

    JsonObject results_0 = doc["results"][0];

    JsonObject results_0_now = results_0["now"];
    const char *results_0_now_text = results_0_now["text"];               // "Sunny"
    const char *results_0_now_code = results_0_now["code"];               // "0"
    const char *results_0_now_temperature = results_0_now["temperature"]; // "32"

    const char *results_0_last_update = results_0["last_update"]; // "2020-06-02T14:40:00+08:00"

    // 显示以上信息
    String results_0_now_text_str = results_0_now["text"].as<String>();
    int results_0_now_code_int = results_0_now["code"].as<int>();
    int results_0_now_temperature_int = results_0_now["temperature"].as<int>();

    String results_0_last_update_str = results_0["last_update"].as<String>();

    client.println("WEA:");
    client.println(results_0_now_text_str);
    client.println("TEM:");
    client.println(results_0_now_temperature_int);
    client.println(results_0_last_update_str);
    
    MySerial_stm32.write(0x2);
    MySerial_stm32.print("WEA:" + results_0_now_text_str + "TEM:" + results_0_now_temperature_int);
    MySerial_stm32.write(0xff);
}


void setup()
{
    // 打开串口，传输数据
    Serial.begin(115200);
    MySerial_stm32.begin(9600,SERIAL_8N1,41,40);

    // WIFI接入局域网
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    timeClient.begin();

    Serial.println("Connected");
    Serial.print("IP Address:");
    // 有点问题？
    Serial.println(WiFi.localIP());
}


char rev;
String stm32_word = "";


void loop()
{
    Serial.println("try to connect the AP:");
    if (client.connect(serverIP, serverPort))
    {



        
        Serial.println("OK! Let say hello world");


        

        while (1)
        {


            Serial.write(0xff);

            String line = client.readStringUntil('\n');
            if(MySerial_stm32.available())
            {
                rev=MySerial_stm32.read();
                if(rev == 'A')
                {   
                    client.printf("STATE 1");
                    String states = client.readStringUntil('\n');
                    MySerial_stm32.print(0x01);
                    MySerial_stm32.printf("",states);
                    MySerial_stm32.print(0xff);
                    MySerial_stm32.print(0xff);
                    MySerial_stm32.print(0xff);
                }
                if(rev == 'B')
                {   
                    client.printf("STATE 1");
                    String states = client.readStringUntil('\n');
                    MySerial_stm32.print(0x01);
                    MySerial_stm32.printf("",states);
                    MySerial_stm32.print(0xff);
                    MySerial_stm32.print(0xff);
                    MySerial_stm32.print(0xff);
                }

            }

            if(line == "PC")
            {
                String MEM = client.readStringUntil('\n');

                String DISK = client.readStringUntil('\n');

                String MYPC = client.readStringUntil('\n');

            }

            // 时间更新
            if (line == "TIME")
            {
                timeClient.update();
                client.println(timeClient.getFormattedTime());
                MySerial_stm32.write(0x01);
                MySerial_stm32.print(daysOfTheWeek[timeClient.getDay()]);
                MySerial_stm32.print(", ");
                MySerial_stm32.print(timeClient.getHours());
                MySerial_stm32.print(":");
                MySerial_stm32.print(timeClient.getMinutes());
                MySerial_stm32.print(":");
                MySerial_stm32.print(timeClient.getSeconds());
                MySerial_stm32.write(0xff);
            }

            // 向心知天气服务器服务器请求信息并对信息进行解析
            if (line == "WEATHER")
            {
                // 建立心知天气API当前天气请求资源地址
                String reqRes = "/v3/weather/now.json?key=" + reqUserKey +
                                +"&location=" + reqLocation +
                                "&language=en&unit=" + reqUnit;
                httpRequest(reqRes);
            }

            if(line == "PC")
            {
                MySerial_stm32.print(0x3);
                MySerial_stm32.print(0xff);
                MySerial_stm32.print(0xff);
                MySerial_stm32.print(0xff);
            }

            if(line == "MUSIC")
            {

            }

            Serial.println(line);
        }
    }
}
