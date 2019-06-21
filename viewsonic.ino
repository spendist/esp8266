#include <NTPClient.h>

/*
  Start auto wifi configure page
  display index.html (webserver)
  do action for ViewSonic control by software serial.
  前一段时间， 家里采购了一个 ViewSonic 的家用投影仪， 型号：PX727-4K/PX747-4K, 这是一个4K的家用投影，看电影效果还可以， 唯一遗憾的是不支持网络。
  阅读说明书以后，发现里面有串口控制的信息。于是突发奇想，用esp8266 实现远程控制，岂不是很好。
说干就干。硬件框架： esp8266 nodemcu 一块，串口调试器一块。arduino 开发环境一个。
其中 Nodemcu +5V，Gnd 管脚连接一个usb用于模块供电，未来直接插投影仪背面的usb口，软件实现附件串口， 自带的usb用于调试。
实现思路：
1：使用 Wifimanager 库，通过配置SSID/Password连接家庭网络
2：连接成功后提供web服务，通过不同的url发送请求给8266。
3：根据请求，打开8266相应的数据文件（其实就是控制指令）
4：把读到的数据发送给虚拟串口至投影仪，实现自动化控制。
5: After
6: 使用家里的 domoticz IOT 软件集成， 实现基于互联网的物联网控制。
废话不多说了， 直接上源码： 有兴趣的朋友可以交流一下：

*/

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ESP8266mDNS.h>
#include <SoftwareSerial.h>
#include <FS.h>   // file system

#define BAUD_RATE 9600
#define WifiLedPin 0
#define PowerLedPin 2     // D4 on NodeMCU and WeMos. Controls the onboard LED.

ESP8266WebServer server ( 80 );         //https://links2004.github.io/Arduino/d3/d58/class_e_s_p8266_web_server.html
SoftwareSerial swSer(14, 12, false, 256); // softwareSerial, for NodeMCU, GPIO 14&15, pin D5,D6

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

void setup() {
  Serial.begin(9600);
  swSer.begin(9600);
  delay(50);
  pinMode(PowerLedPin, OUTPUT);
  pinMode(WifiLedPin, OUTPUT);
  digitalWrite(PowerLedPin, LOW);
  Serial.setDebugOutput(true);
  // list files:
  SPIFFS.begin();
  Serial.println("Root filesystem file list:");
  String str = "file system\r\nfn=";
  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) {
    str += dir.fileName();
    str += "\tsize=";
    str += dir.fileSize();
    str += "\r\n";
  }
  Serial.print(str);

  WiFiManager wifiManager;
  wifiManager.setAPCallback(configModeCallback);

  if (!wifiManager.autoConnect()) {
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  }

  //if you get here you have connected to the WiFi
  digitalWrite(WifiLedPin, LOW);
  Serial.println("connected...yeey :)");

  // define webserver struct.
  server.on ( "/", handleRoot );
  server.on("/logo.jpg", []() {
    LoadImage("/logo.jpg", "application/x-jpg");
  });
  server.on ( "/hello", []() {
    server.send ( 200, "text/plain", "Hello world, Just test command" );
  } );
  server.on("/pwn_on", command);
  server.on("/pwn_off", command);
  server.on("/source_comp", command);
  server.on("/source_hdmi1", command);
  server.on("/source_hdmi2", command);
  server.on("/reset", command);
  server.on("/view_1",command);
  server.on("/view_2",command);
  server.on("/vier_3",command);
  server.on("/view_4",command);
  server.on("/msg_on",command);
  server.on("/msg_off",command);
  server.on("/autoscan_on",command);
  server.on("/autoscan_off",command);
  server.on("/mute_on",command);
  server.on("/mute_off",command);
  server.on("/volume_up",command);
  server.on("/volume_down",command);
  server.on("/lang_eng",command);
  server.on("/lang_cn",command);
  server.on("/cec_on",command);
  server.on("/cec_off",command);
  server.onNotFound ( handleNotFound );

  server.begin();  // start listen
  MDNS.addService("http", "tcp", 80);
  Serial.println ( "HTTP server started." );
  // Print the IP address
  Serial.println(WiFi.localIP());

}

void loop() {
  // put your main code here, to run repeatedly:
  // Check if a client has connected
  server.handleClient();
  yield(); // wait until client end;

}

void LoadImage(String path, String Type) {
  if ( SPIFFS.exists(path)) {
    File file = SPIFFS.open(path, "r");
    server.streamFile(file, Type);
    file.close();
  }
  yield();
}
void command() {
  /* General request analynes */
  // Document @ https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WebServer
  // https://links2004.github.io/Arduino/d3/d58/class_e_s_p8266_web_server.html
  String Req="";
  char c;
  Req=server.uri();
  
  Serial.println("Req=" + Req + ", Filename=" + Req + ".dat");
  if ( SPIFFS.exists(Req+".dat")) {
    File file = SPIFFS.open(Req+".dat", "r");  
    while (file.available()) {
      c=char(file.read());
      swSer.write(c);
    }
    file.close();
    Serial.println("-->End write data");
    server.send(200, "text/plain", "Okey, command "+Req+" had been send to control!");
  } else {
    Serial.println("XXX Error Invalid command! " + Req );
    handleNotFound();
  } 
   yield();
}
/*
void pwn_on() {
  Serial.println("-->power on");
  char cmd[10] = {0x06, 0x14, 0x00, 0x04, 0x00, 0x34, 0x11, 0x00, 0x00, 0x5D};
  char ch,ret[16];
  swSer.write(cmd, 10);
  server.send(200, "text/plain", "Ok send [Power ON] command to softwareSerial.");
  delay(500);
  
  while (swSer.available()>0) {
    ch=swSer.read();
  }
  Serial.println(ret);
  Serial.println("-->Done.");
  yield();
}

*/
void handleRoot() {
  // char temp[4096]; // index.html max size=4K
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  String xhtml;
  File fp = SPIFFS.open("/index.html", "r");
  if (!fp) {
    Serial.println("Error! open index.html.");
  }
  else {
    while (fp.available()) {
      xhtml += char(fp.read());
    }
    fp.close();
    //Serial.print(xhtml);
  }
  server.send(200, "text/html", xhtml.c_str());

  //server.send(200,"text/html",html);
  //snprintf ( temp, 4096, html.c_str(), hr, min % 60, sec % 60 );
  //server.send ( 200, "text/html", temp );
  yield();
  return;
}

void handleNotFound()
{
  // print error information DEBUG
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }
  server.send ( 404, "text/plain", message );
   yield();
}
