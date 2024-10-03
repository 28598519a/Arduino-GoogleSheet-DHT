#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "HTTPSRedirect.h"
#include "DHT.h"
#include <Ticker.h>
Ticker timer1, timer2;

// DHT define
#define DHT_VCC 0
#define DHTPIN 5
#define DHTTYPE DHT22

//======WiFi AP setting========================================
const char Fssid[] = "WeMos D1";       // Ap mode SSID
const char Fpassword[] = "12345678";   // AP mode Password

//======WiFi STA setting=======================================
const char* ssid = "";          // STA mode default SSID
const char* password = "";   // STA mode default Password

//======DHT setting============================================
float h=-1,t;
DHT dht(DHTPIN, DHTTYPE);

//======HTTPClient=============================================
const char host[] = "script.google.com";
const char GScriptId[] = ""; // Google script ID
const char DeviceID[] = "";
const int httpsPort = 443;
HTTPSRedirect* client;

//======WebSever setting=======================================
String Inssid = "",Inpassword = "";
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

ESP8266WebServer server(80);
int State = 1;
bool ManualReconnect = false;
// 建立html頁面
const char htmlPage[]PROGMEM=R"=====(
<!DOCTYPE html>
<html>
<body>
<h3>WiFi Setting</h3>
<FORM METHOD="POST"action="/">
<label> SSID </label>
<input type="text" name="Wifissid" value="">
<label> Password </label>
<input type="text" name="Wifipasswords" value="">
<input type="submit" value="Send">
</form>
</body>
</html>
)=====";

/////////////////////===connect===////////////////////////////
bool WiFiConnect()
{
    int i = 0;
    bool status = false;
    
    Serial.print("Connecting to wifi: ");
    Serial.println(Inssid);
    WiFi.begin(Inssid.c_str(), Inpassword.c_str());
    
    while (WiFi.status() != WL_CONNECTED && i < 20)
    {
        // 每0.5秒確認一次，10秒未連線成功則退出
        delay(500);
        Serial.print(".");
        i++;
    }
    Serial.println(WiFi.status());
    
    if(i < 20)
    {
        // 設定斷線自動重連
        WiFi.setAutoReconnect(true);
        ManualReconnect = false;
        
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        status = true;
    }
    yield();
    
    return status;
}

bool CreateClient()
{
    client = new HTTPSRedirect(httpsPort);
    client->setPrintResponseBody(true);
    client->setContentTypeHeader("application/json");
    // Disable SSL certificate verification (esp8266開發版庫 2.4.2 版以上需要)
    client->setInsecure();
    return ClientConnect();
}

bool ClientConnect()
{
    for (int i=0; i<5; i++){
        int retval = client->connect(host, httpsPort);
        
        if (retval == 1)
        {
            Serial.print("Connecting to ");
            Serial.println(host);
            return true;
        }
        else Serial.println("Connection failed. Retrying...");
        yield();
    }
    return false;
}

///////////////////////===softAP===///////////////////////////
void softap_start()
{
    // Access Point + Station, AP用於臨時修改要連的WiFi，STA持續嘗試連線WiFi
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(Fssid, Fpassword);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    
    // handlePostForm僅會在外部裝置連上此AP並且在每次與 http://local_ip/ 頁面有交互時才會執行1次
    server.on("/", handlePostForm);
    server.begin();
    Serial.println("HTTP Server Started");
    yield();
}

// 取得網頁傳送資料
void handlePostForm()
{
    Inssid = "";
    Inpassword = "";
    Inssid = server.arg("Wifissid");
    Inpassword = server.arg("Wifipasswords");

    if (Inssid != "")
    {
        Serial.println("SSID Received, contents:");
        Serial.println(Inssid);
        Serial.println("Passwd Received, contents:");
        Serial.println(Inpassword);
        
        // 斷開WiFi連線 (自動重連也會關閉)，並清除之前連線的SSID
        WiFi.disconnect();        
        ManualReconnect = true;
    }
    server.send(200,"text/html",htmlPage);
    yield();
}

//////////////////////===sensor===////////////////////////////
String dhthum()
{
    h = dht.readHumidity();
    
    if (!isnan(h))
    {
        return String("data1=") + h + String("&");
    }
    else return String("");
}
String dhthic()
{
    // Read temperature as Celsius (the default)
    // 絕對溫度
    t = dht.readTemperature();
    
    // 相對溫度 (體感溫度)
    //float hic = dht.computeHeatIndex(t, h, false);
    
    if (!isnan(t))
    {
        return String("data2=") + t + String("&");
    }
    else return String("");
}

//////////////////////===other===/////////////////////////////
String CheckFreeRam()
{
    int ram = ESP.getFreeHeap();
    return String("Available: ") + ram + String(" bytes");
}

void DHT_Power(bool status)
{
    if (status)
    {
        digitalWrite(DHT_VCC, HIGH);
        // DHT供電後要等1秒啟動，保險設2秒
        delay(2000);
    }
    else digitalWrite(DHT_VCC, LOW);
    yield();
}

bool SensorSend()
{
    bool status = true;
    
    DHT_Power(true);
    String url3 = String("/macros/s/")+ GScriptId + String("/exec?ID=") + DeviceID + String("&");
    url3 += dhthum();
    url3 += dhthic();
    DHT_Power(false);
    
    Serial.println("ready to doGet....");
    if (client->GET(url3, host))
    {
        Serial.println("GET SUCCESS");
        // 斷開client (若不自斷，超時也是會被自動斷開，浪費資源而已)
        client->stop();
        status = true;
    }
    else
    {
        Serial.println("GET FAILED");
        status = false;
    }
    yield();
    
    return status;
}

//////////////////////===Timer===/////////////////////////////
/* 
* Ticker執行的callback func內會執行到的code不可以有yield()、delay()，會導致wdt reset。如果有則那部分要用改flag的方式弄到loop()做
* ESP自帶的預設ticker庫只能吃一個參數 (想超過的話要定義struct來傳)
*/
// 預設0秒先觸發做1次
bool TSS_flag = true;

void TimerSensorSend(bool main_ticker=false)
{
    // 當timer1 (正常的送資料時間) 觸發時若timer2還在運作就關掉timer2
    if (main_ticker && timer2.active())
    {
        timer2.detach();
    }
    
    TSS_flag = true;
}

//=============================================================

void setup()
{
    Serial.begin(115200);
    Serial.println();
    Inssid = ssid;
    Inpassword = password;
    
    // Station, 無AP
    WiFi.mode(WIFI_STA);
    WiFiConnect();
    if (WiFi.isConnected())
    {
        CreateClient();
    }
    else ManualReconnect = true;
    
    dht.begin();
    // DHT供電改由I/O腳(0V,5V)，避免DHT因長期通電發熱、損耗
    pinMode(DHT_VCC, OUTPUT);
    
    // 每1小時收集並上傳1次感測數據 (不包含0秒，這裡透過TSS_flag預設值來達成)
    timer1.attach(3600, TimerSensorSend, true);
}

void loop()
{
    /* Handle WiFi */
    if (State == 0)
    {
        // 未建立連線時維持網頁
        //Serial.println("WebPage...handling");
        server.handleClient();
        
        if (WiFi.isConnected())
        {
            Serial.println(WiFi.status());
            
            // WiFi連線成功，關閉AP及server
            server.stop();
            WiFi.softAPdisconnect(true);
            WiFi.mode(WIFI_STA);
            
            CreateClient();
            State = 1;
        }
        else if (ManualReconnect)
        {
            // Server跟AP此時比起之前自動重連期間會比較卡一點，因為要delay跟佔住CPU執行緒
            delay(500);
            WiFiConnect();
        }
    }
    else if (State == 1 && !WiFi.isConnected())
    {
        // WiFi狀態斷線，啟動AP並嘗試重連WiFi
        delete client;
        Serial.println("connection failed!");
        softap_start();
        
        delay(500);
        State = 0;
    }
    else if (TSS_flag)
    {
        if (!client->connected())
        {
            ClientConnect();
        }
        
        if (!SensorSend())
        {
            // Retry, 57 + 2 (DHT) + 1 (Web) = 60s. (Need check the client)
            timer2.once(57, TimerSensorSend, false);
        }
        TSS_flag = false;
    }
    
    //Serial.println(CheckFreeRam());
    delay(100);
}
