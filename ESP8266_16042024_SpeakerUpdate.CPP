#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <IRsend.h>
#include <Arduino.h>
#include <assert.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

const char* ssid = "-----------";
const char* password = "----------";

/* 
IR Code
pw 0x1FE48B7 (32 Bits) Tv On
md 0x1FE58A7 (32 Bits) Eco
mu 0x1FE7887 (32 Bits) TvExit
pl 0x1FE807F (32 Bits) Ac on
bw 0x1FE40BF (32 Bits) Turbo
fw 0x1FEC03F (32 Bits) Dim
e  0x1FE20DF (32 Bits) Socket
v+ 0x1FEA05F (32 Bits) Tv Vol+
v- 0x1FE609F (32 Bits) Tv Vol-
0  0x1FEE01F (32 Bits) Fan Off
rt 0x1FE10EF (32 Bits) Night
sd 0x1FE906F (32 Bits) Light
1  0x1FE50AF (32 Bits) Fan Speed 1
2  0x1FED827 (32 Bits) Fan Speed 2
3  0x1FEF807 (32 Bits) Fan Speed 3
4  0x1FE30CF (32 Bits) Fan Speed 4
5  0x1FEB04F (32 Bits) Fan Speed 5
6  0x1FE708F (32 Bits) Ac Temp 26
7  0x1FE00FF (32 Bits) Ac Temp 27
8  0x1FEF00F (32 Bits) Ac Temp 28
9  0x1FE9867 (32 Bits) Ac Temp 29
*/

// IR Remote Code for Lights
#define IR1            0x1FE48B7 //pw
#define IR2            0x1FE58A7 //mode
#define IR3            0x1FE7887 //mute
#define IR4            0x1FE807F //play
#define IR5            0x1FE40BF //backward
#define IR6            0x1FEC03F //forward
#define IR7            0x1FE20DF // eq
#define IR8            0x1FEA05F // v-
#define IR9            0x1FE609F // v+
#define IR10           0x1FE10EF // rpt
#define IR11           0x1FE906F // u/sd
#define IRF0           0x1FEE01F // 0
#define IRF1           0x1FE50AF // 1
#define IRF2           0x1FED827 // 2
#define IRF3           0x1FEF807 // 3
#define IRF4           0x1FE30CF // 4
#define IRF5           0x1FEB04F // 5
#define IRF6           0x1FE708F // 6
#define IRF7           0x1FE00FF // 7
#define IRF8           0x1FEF00F // 8
#define IRF9           0x1FE9867 // 9
/*
ADC0    A0 Fan Speed
GPIO-16 D0 Socket
GPIO -5 D1 Night
GPIO -4 D2 IR Tansmittor--------
GPIO -0 D3 Light
GPIO -2 D4 IR Reciver-----------
GPIO-14 D5 Fan
GPIO-12 D6 Night Switch
GPIO-13 D7 Fan Switch
GPIO-15 D8 Light Switch
GPIO-10 SK Socket Switch
GPIO -9 S3 
*/

int sensorValue = 0;

String switch1name = "Socket";
String switch1 = "OFF";
int     relay1 = 16; // D0
int switch1pin = 10; // SK

String switch2name = "Night";
String switch2 = "OFF";
int     relay2 = 15; // D8
int switch2pin = 12; // D6

//String switch3name = "Fan";
String Fan = "Speed5";
int     relay3 = 0; // D3
int switch3pin = 13; // D7

String switch4name = "Light";
String switch4 = "ON";
int     relay4 = 5; // D1
int switch4pin = 14; // D5

String All = "OFF";
String FL = "ON";

ESP8266WebServer server(80);

IRsend irsend(4);// IR LED  (D2)

//IR Recicer----------------------------------------------------------------------------

#ifdef ARDUINO_ESP32C3_DEV
const uint16_t kRecvPin = 2;
#else
const uint16_t kRecvPin = 2;
#endif
const uint32_t kBaudRate = 115200;
const uint16_t kCaptureBufferSize = 1024;
#if DECODE_AC
const uint8_t kTimeout = 50;
#else  
const uint8_t kTimeout = 15;
#endif  
const uint16_t kMinUnknownSize = 12;
const uint8_t kTolerancePercentage = kTolerance;  
#define LEGACY_TIMING_INFO false
IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
decode_results results;

//IR Recicer----------------------------------------------------------------------------

void handleRoot() {
 server.send(200, "text/html", 
"<!DOCTYPE html>" \
"<html>" \
"<head>" \
"<title>home</title>" \
"<style>"\
".container {background-color: grey; position: bottom; height: 614px; width: 600px; border-radius: 10px;}"\
"h1 {text-align: center;}"\
"h2 {text-align: center;}"\
"table.center {margin-left: auto; margin-right: auto;}"\
".btn {border: 1px solid 3498db; width: 173px; height: 110; background: none; padding: 10px 20px; font-size: 20px; margin:10px;border-radius: 10px;}"\
"</style>"\
"</head>" \
"<body>" \
"<div class=\"container\">"\
"<h1>Smart Home</h1>" \
"<h2>Master Bedroom | Fan: "+ Fan +"</h2>" \
"<table class=\"center\">" \
  "<tr>" \
    "<td><a href=\"/Switch1\"><button class=\"btn\">"+ switch1name +"<br />"+ switch1 +"</button></a></td>" \
    "<td><a href=\"/Switch2\"><button class=\"btn\">"+ switch2name +"<br />"+ switch2 +"</button></a></td>" \
    "<td><a href=\"/Switch4\"><button class=\"btn\">"+ switch4name +"<br />"+ switch4 +"</button></a></td>" \
  "</tr>" \
"</table>" \
"<table class=\"center\">" \
  "<tr>" \
    "<td><a href=\"FanOFF\"><button class=\"btn\">Fan<br />OFF</button></a></td>" \
    "<td><a href=\"/FanSpeed1\"><button class=\"btn\">Fan<br />Speed1</button></a></td>" \
    "<td><a href=\"/FanSpeed2\"><button class=\"btn\">Fan<br />Speed2</button></a></td>" \
  "</tr>" \
"</table>" \
"<table class=\"center\">" \
  "<tr>" \
    "<td><a href=\"/FanSpeed3\"><button class=\"btn\">Fan<br />Speed3</button></a></td>" \
    "<td><a href=\"/FanSpeed4\"><button class=\"btn\">Fan<br />Speed4</button></a></td>" \
    "<td><a href=\"/FanSpeed5\"><button class=\"btn\">Fan<br />Speed5</button></a></td>" \
  "</tr>" \
"</table>" \
"<table class=\"center\">" \
  "<tr>"
    "<td><a href=\"/All\"><button class=\"btn\">ALL Devices<br />" + All + "</button></a></td>" \
    "<td><a href=\"/AcON\"><button class=\"btn\">Air Conditioner<br />ON/OFF</button></a></td>" \
    "<td><a href=\"/FL\"><button class=\"btn\">Fan Light<br />" + FL + "</button></a></td>" \
  "</tr>" \
"</table>" \
"<table class=\"center\">" \
  "<tr>"
     "<td><a href=\"/Ac\"><button class=\"btn\">Ac<br/>Page</button></a></td>" \
    "<td><a href=\"/Tv\"><button class=\"btn\">Tv<br/>Page</button></a></td>" \
    "<td><a href=\"/\"><button class=\"btn\">Home<br/>Page</button></a></td>" \
  "</tr>" \
"</table>" \
"</div>"\
"</body>" \
"</html>" \
);
}
/* Only Html
<!DOCTYPE html>
<html>
<head>
<title>home</title>
<style>
.container {background-color: grey; position: bottom;height: 524px; width: 600px; border-radius: 10px;}
h1 {text-align: center;}
h2 {text-align: center;}
table.center {margin-left: auto; margin-right: auto;}
.btn {border: 1px solid 3498db; width: 173px; height: 110; background: none; padding: 10px 20px; font-size: 20px; margin:10px; border-radius: 10px;}
</style>
</head>
<body>
<div class="container"> 
<h1>Smart Home</h1>
<h2>Master Bedroom | Fan: Speed5</h2>
<table class="center">
  <tr>
    <td><a href="/Switch1"><button class="btn">Socket<br/>OFF</button></a></td>
    <td><a href="/Switch2"><button class="btn">Night<br/>OFF</button></a></td>
    <td><a href="/Switch4"><button class="btn">Light<br/>ON</button></a></td>
  </tr>
</table>
<table class="center">
  <tr>
    <td><a href="/FanOFF"><button class="btn">Fan<br/>OFF</button></a></td>
    <td><a href="/FanSpeed1"><button class="btn">Fan<br/>Speed1</button></a></td>
    <td><a href="/FanSpeed2"><button class="btn">Fan<br/>Speed2</button></a></td>
  </tr>
</table>
<table class="center">
  <tr>
    <td><a href="/FanSpeed3"><button class="btn">Fan<br/>Speed3</button></a></td>
    <td><a href="/FanSpeed4"><button class="btn">Fan<br/>Speed4</button></a></td>
    <td><a href="/FanSpeed5"><button class="btn">Fan<br/>Speed5</button></a></td>
  </tr>
</table>
<table class="center">
  <tr>
    <td><a href="/All"><button class="btn">ALL Devices<br/>ON</button></a></td>
    <td><a href="/AcON"><button class="btn">Air Conditioner<br/>ON/OFF</button></a></td>
    <td><a href="/FL"><button class="btn">Fan Light<br/>ON</button></a></td>
  </tr>
</table>
</div> 
</body>
</html>
*/

void AcPage() {
 server.send(200, "text/html", 
"<!DOCTYPE html>" \
"<html>" \
"<head>" \
"<title>home</title>" \
"<style>"\
".container {background-color: grey; position: bottom; height: 614px; width: 600px; border-radius: 10px;}"\
"h1 {text-align: center;}"\
"h2 {text-align: center;}"\
"table.center {margin-left: auto; margin-right: auto;}"\
".btn {border: 1px solid 3498db; width: 173px; height: 110; background: none; padding: 10px 20px; font-size: 20px; margin:10px;border-radius: 10px;}"\
"</style>"\
"</head>" \
"<body>" \
"<div class=\"container\">"\
"<h1>Smart Home</h1>" \
"<h2>Master Bedroom | Whirlpool AC</h2>" \
"<table class=\"center\">" \
  "<tr>" \
    "<td><a href=\"/Ac24\"><button class=\"btn\">AC<br/>On/Off</button></a></td>" \
    "<td><a href=\"/AcTurbo\"><button class=\"btn\">AC<br/>Turbo</button></a></td>" \
    "<td><a href=\"/AcDim\"><button class=\"btn\">AC<br/>Dim</button></a></td>" \
  "</tr>" \
"</table>" \
"<table class=\"center\">" \
  "<tr>" \
    "<td><a href=\"/AcPowerSaver\"><button class=\"btn\">AC<br/>Power Saver</button></a></td>" \
    "<td><a href=\"/AcWinter\"><button class=\"btn\">AC<br/>AcWinter</button></a></td>" \
    "<td><a href=\"/Ac30\"><button class=\"btn\">AC<br/>Ac30</button></a></td>" \
  "</tr>" \
"</table>" \
"<table class=\"center\">" \
  "<tr>" \
     "<td><a href=\"/Ac29\"><button class=\"btn\">AC<br/>Ac29</button></a></td>" \
    "<td><a href=\"/Ac28\"><button class=\"btn\">AC<br/>Ac28</button></a></td>" \
    "<td><a href=\"/Ac27\"><button class=\"btn\">AC<br/>Ac27</button></a></td>" \
  "</tr>" \
"</table>" \
"<table class=\"center\">" \
  "<tr>" \
     "<td><a href=\"/Ac26\"><button class=\"btn\">AC<br/>Ac26</button></a></td>" \
    "<td><a href=\"/Ac24\"><button class=\"btn\">AC<br/>Ac24</button></a></td>" \
    "<td><a href=\"/Ac25\"><button class=\"btn\">AC<br/>Ac25</button></a></td>" \
  "</tr>" \
"</table>" \
"<table class=\"center\">" \
  "<tr>"
     "<td><a href=\"/Ac\"><button class=\"btn\">AC<br/>Page</button></a></td>" \
    "<td><a href=\"/Tv\"><button class=\"btn\">Tv<br/>Page</button></a></td>" \
    "<td><a href=\"/\"><button class=\"btn\">Home<br/>Page</button></a></td>" \
  "</tr>" \
"</table>" \
"</div>"\
"</body>" \
"</html>" \
);
}


/* Ac Page Html
<!DOCTYPE html>
<html>
<head>
<title>home</title>
<style>
.container {background-color: grey; position: bottom;height: 614px; width: 600px; border-radius: 10px;}
h1 {text-align: center;}
h2 {text-align: center;}
table.center {margin-left: auto; margin-right: auto;}
.btn {border: 1px solid 3498db; width: 173px; height: 110; background: none; padding: 10px 20px; font-size: 20px; margin:10px; border-radius: 10px;}
</style>
</head>
<body>
<div class="container"> 
<h1>Smart Home</h1>
<h2>Master Bedroom | Whirlpool AC</h2>
<table class="center">
  <tr>
    <td><a href="/AcON"><button class="btn">AC<br/>On/Off</button></a></td>
    <td><a href="/ACTurbo"><button class="btn">AC<br/>Turbo</button></a></td>
    <td><a href="/AcDim"><button class="btn">AC<br/>Dim</button></a></td>
  </tr>
</table>
<table class="center">
  <tr>
    <td><a href="/AcPowerSaver"><button class="btn">AC<br/>Power Saver</button></a></td>
    <td><a href="/AcWinter"><button class="btn">AC<br/>Winter</button></a></td>
    <td><a href="/Ac30"><button class="btn">AC<br/>Temp 30</button></a></td>
  </tr>
</table>
<table class="center">
  <tr>
    <td><a href="/Ac29"><button class="btn">AC<br/>Temp 29</button></a></td>
    <td><a href="/Ac28"><button class="btn">AC<br/>Temp 28</button></a></td>
    <td><a href="/Ac27"><button class="btn">AC<br/>Temp 27</button></a></td>
  </tr>
</table>
<table class="center">
  <tr>
    <td><a href="/Ac26"><button class="btn">AC<br/>Temp 26</button></a></td>
    <td><a href="/Ac25"><button class="btn">AC<br/>Temp 25</button></a></td>
    <td><a href="/Ac24"><button class="btn">AC<br/>Temp 24</button></a></td>
  </tr>
</table>
<table class="center">
  <tr>
    <td><a href="/AcPage"><button class="btn">AC<br/>Page</button></a></td>
    <td><a href="/TvPage"><button class="btn">Tv<br/>Page</button></a></td>
    <td><a href="/"><button class="btn">Home<br/>Page</button></a></td>
  </tr>
</table>
</div> 
</body>
</html>
*/

void TvPage() {
 server.send(200, "text/html", 
"<!DOCTYPE html>" \
"<html>" \
"<head>" \
"<title>home</title>" \
"<style>"\
".container {background-color: grey; position: bottom; height: 614px; width: 600px; border-radius: 10px;}"\
"h1 {text-align: center;}"\
"h2 {text-align: center;}"\
"table.center {margin-left: auto; margin-right: auto;}"\
".btn {border: 1px solid 3498db; width: 173px; height: 110; background: none; padding: 10px 20px; font-size: 20px; margin:10px;border-radius: 10px;}"\
"</style>"\
"</head>" \
"<body>" \
"<div class=\"container\">"\
"<h1>Smart Home</h1>" \
"<h2>Master Bedroom | Croma Tv</h2>" \
"<table class=\"center\">" \
  "<tr>" \
    "<td><a href=\"/TvOn\"><button class=\"btn\">Tv<br/>On/Off</button></a></td>" \
    "<td><a href=\"/TvMenu\"><button class=\"btn\">Tv<br/>Menu</button></a></td>" \
    "<td><a href=\"/TvSource\"><button class=\"btn\">Tv<br/>Source</button></a></td>" \
  "</tr>" \
"</table>" \
"<table class=\"center\">" \
  "<tr>" \
    "<td><a href=\"/TvVolUp\"><button class=\"btn\">Tv<br />Vol+</button></a></td>" \
    "<td><a href=\"/TvUp\"><button class=\"btn\">Tv<br />Up</button></a></td>" \
    "<td><a href=\"/TvExit\"><button class=\"btn\">Tv<br />Exit</button></a></td>" \
  "</tr>" \
"</table>" \
"<table class=\"center\">" \
  "<tr>" \
     "<td><a href=\"/TvLeft\"><button class=\"btn\">Tv<br/>Left</button></a></td>" \
    "<td><a href=\"/TvOk\"><button class=\"btn\">Tv<br/>Ok</button></a></td>" \
    "<td><a href=\"/TvRight\"><button class=\"btn\">Tv<br/>Right</button></a></td>" \
  "</tr>" \
"</table>" \
"<table class=\"center\">" \
  "<tr>" \
     "<td><a href=\"/TvVolDown\"><button class=\"btn\">Tv<br/>Vol-</button></a></td>" \
    "<td><a href=\"/TvDown\"><button class=\"btn\">Tv<br/>Down</button></a></td>" \
    "<td><a href=\"/TvEco\"><button class=\"btn\">Tv<br/>Eco</button></a></td>" \
  "</tr>" \
"</table>" \
"<table class=\"center\">" \
  "<tr>"
     "<td><a href=\"/Ac\"><button class=\"btn\">Ac<br/>Page</button></a></td>" \
    "<td><a href=\"/Tv\"><button class=\"btn\">Tv<br/>Page</button></a></td>" \
    "<td><a href=\"/\"><button class=\"btn\">Home<br/>Page</button></a></td>" \
  "</tr>" \
"</table>" \
"</div>"\
"</body>" \
"</html>" \
);
}

/* Tv Page
<!DOCTYPE html>
<html>
<head>
<title>home</title>
<style>
.container {background-color: grey; position: bottom;height: 614px; width: 600px; border-radius: 10px;}
h1 {text-align: center;}
h2 {text-align: center;}
table.center {margin-left: auto; margin-right: auto;}
.btn {border: 1px solid 3498db; width: 173px; height: 110; background: none; padding: 10px 20px; font-size: 20px; margin:10px; border-radius: 10px;}
</style>
</head>
<body>
<div class="container"> 
<h1>Smart Home</h1>
<h2>Master Bedroom | Croma Tv</h2>
<table class="center">
  <tr>
    <td><a href="/TvON"><button class="btn">Tv<br/>On/Off</button></a></td>
    <td><a href="/TvMenu"><button class="btn">Tv<br/>Menu</button></a></td>
    <td><a href="/TvSource"><button class="btn">Tv<br/>Source</button></a></td>
  </tr>
</table>
<table class="center">
  <tr>
    <td><a href="/TvVolUp"><button class="btn">Tv<br/>Vol+</button></a></td>
    <td><a href="/TvUp"><button class="btn">Tv<br/>Up</button></a></td>
    <td><a href="/TvExit"><button class="btn">Tv<br/>Exit</button></a></td>
  </tr>
</table>
<table class="center">
  <tr>
    <td><a href="/TvLeft"><button class="btn">Tv<br/>Left</button></a></td>
    <td><a href="/TvOk"><button class="btn">Tv<br/>Ok</button></a></td>
    <td><a href="/TvRight"><button class="btn">Tv<br/>Right</button></a></td>
  </tr>
</table>
<table class="center">
  <tr>
    <td><a href="/TvVolDown"><button class="btn">Tv<br/>Vol-</button></a></td>
    <td><a href="/TvDown"><button class="btn">Tv<br/>Down</button></a></td>
    <td><a href="/TvEco"><button class="btn">Tv<br/>Eco</button></a></td>
  </tr>
</table>
<table class="center">
  <tr>
    <td><a href="/AcPage"><button class="btn">AC<br/>Page</button></a></td>
    <td><a href="/TvPage"><button class="btn">Tv<br/>Page</button></a></td>
    <td><a href="/"><button class="btn">Home<br/>Page</button></a></td>
  </tr>
</table>
</div> 
</body>
</html>
*/

void handleIr() {
  for (uint8_t i = 0; i < server.args(); i++) {
    if (server.argName(i) == "code") {
      uint32_t code = strtoul(server.arg(i).c_str(), NULL, 10);
#if SEND_NEC
      irsend.sendNEC(code, 166);
#endif  // http://192.168.29.249/ir?code=9226
    }
  }
  handleRoot();
}

void Allfunction(){
  if (All == "ON" || switch1 == "ON" || switch2 == "ON" || switch4 == "ON" || Fan != "OFF") { // All off
    All = "OFF";
    switch1 = "OFF";
    switch2 = "OFF";
    Fan = "OFF";
    switch4 = "OFF";

    digitalWrite(relay1, HIGH);
    digitalWrite(relay2, HIGH);
    digitalWrite(relay3, HIGH);
    digitalWrite(relay4, HIGH);
    handleRoot();
  }
  else { // All on
    All = "ON";
    switch1 = "ON";
    switch2 = "ON";
    Fan = "Speed5";
    switch4 = "ON";

    digitalWrite(relay1, LOW);
    digitalWrite(relay2, LOW);
    digitalWrite(relay3, LOW);
    digitalWrite(relay4, LOW);

    delay(2000);
    irsend.sendNEC(16476566, 32);
    handleRoot();
  }
}

void FLfunction(){
  if (FL == "ON" || switch4 == "ON" || Fan != "OFF") { // FL off
    FL = "OFF";
    switch4 = "OFF";
    Fan = "OFF";
    digitalWrite(relay3, HIGH);
    digitalWrite(relay4, HIGH);
    handleRoot();
  }
  else { // FL on
    FL = "ON";
    switch4 = "ON";
    Fan = "Speed5";
    digitalWrite(relay3, LOW);
    digitalWrite(relay4, LOW);

    delay(2000);
    irsend.sendNEC(16476566, 32);
    handleRoot();
  }
}

void switch1function(){
  if (switch1 == "ON") {
    switch1 = "OFF";
    digitalWrite(relay1, HIGH);
    handleRoot();
  }
  else {
    switch1 = "ON";
    digitalWrite(relay1, LOW);
    handleRoot();
  }
}

void switch2function(){
  if (switch2 == "ON") {
    switch2 = "OFF";
    digitalWrite(relay2, HIGH);
    handleRoot();
  }
  else {
    switch2 = "ON";
    digitalWrite(relay2, LOW);
    handleRoot();
  }
}

void switch3function(){
  if (Fan != "OFF") {
    FanOFF();
  }
  else {
    FanSpeed5();
  }
}

void switch4function(){
  if (switch4 == "ON") {
    switch4 = "OFF";
    digitalWrite(relay4, HIGH);
    handleRoot();
  }
  else {
    switch4 = "ON";
    digitalWrite(relay4, LOW);
    handleRoot();
  }
}

int IRFanDelay = 500;

void FanOFF(){
  Fan = "OFF";
  digitalWrite(relay3, HIGH);
  handleRoot();
}

void FanSpeed1(){
  Fan = "Speed1";
  digitalWrite(relay3, LOW);
  delay(IRFanDelay);
  irsend.sendNEC(16460246, 32);
  handleRoot();
}

void FanSpeed2(){
  Fan = "Speed2";
  digitalWrite(relay3, LOW);
  delay(IRFanDelay);
  irsend.sendNEC(16488806, 32);
  handleRoot();
}

void FanSpeed3(){
  Fan = "Speed3";
  digitalWrite(relay3, LOW);
  delay(IRFanDelay);
  irsend.sendNEC(16492886, 32);
  handleRoot();
}

void FanSpeed4(){
  Fan = "Speed4";
  digitalWrite(relay3, LOW);
  delay(IRFanDelay);
  irsend.sendNEC(16472486, 32);
  handleRoot();
}

void FanSpeed5(){
  Fan = "Speed5";
  digitalWrite(relay3, LOW);
  delay(IRFanDelay);
  irsend.sendNEC(16476566, 32);
  handleRoot();
}
//irsend.sendSony(9226, 15);
//irsend.sendNEC(0xFB38C7, 32); //croma tv on/off
void AcOn(){
  uint16_t rawData[343] = {9204, 4512,  678, 1612,  680, 1612,  680, 478,  678, 482,  680, 486,  676, 490,  678, 494,  678, 1624,  678, 472,  678, 1616,  676, 1616,  680, 480,  678, 484,  680, 488,  678, 492,  678, 486,  678, 472,  676, 474,  678, 1618,  680, 480,  678, 486,  678, 490,  678, 494,  676, 488,  678, 472,  678, 1616,  678, 480,  678, 482,  678, 488,  676, 490,  678, 492,  680, 1624,  678, 472,  676, 474,  678, 480,  676, 482,  678, 486,  678, 490,  676, 496,  676, 488,  676, 474,  678, 476,  676, 480,  676, 484,  678, 486,  678, 490,  678, 494,  676, 462,  678, 8012,  680, 470,  678, 1614,  680, 478,  678, 482,  678, 486,  678, 488,  676, 494,  678, 1624,  678, 472,  676, 476,  678, 480,  678, 482,  678, 1626,  676, 1630,  678, 494,  678, 488,  678, 472,  676, 478,  676, 482,  676, 484,  678, 486,  676, 490,  678, 494,  678, 488,  676, 474,  676, 476,  676, 480,  676, 486,  674, 488,  676, 492,  674, 496,  676, 490,  674, 474,  676, 476,  674, 482,  674, 484,  678, 488,  662, 506,  650, 520,  672, 490,  652, 498,  674, 476,  676, 482,  674, 486,  672, 490,  676, 492,  674, 496,  674, 490,  648, 500,  674, 478,  674, 484,  650, 510,  674, 488,  674, 494,  650, 520,  650, 514,  674, 476,  672, 480,  650, 1646,  674, 486,  648, 1654,  674, 1632,  674, 498,  672, 466,  676, 8016,  662, 486,  652, 500,  674, 482,  676, 486,  676, 486,  676, 490,  676, 496,  650, 514,  650, 1638,  650, 502,  652, 504,  672, 488,  650, 512,  652, 516,  674, 496,  674, 490,  676, 476,  674, 478,  650, 506,  650, 508,  672, 492,  648, 518,  650, 520,  650, 514,  650, 500,  650, 504,  648, 508,  674, 486,  650, 514,  672, 494,  648, 520,  650, 514,  650, 500,  648, 504,  650, 508,  648, 512,  668, 498,  644, 518,  648, 522,  648, 516,  648, 500,  648, 504,  648, 508,  650, 510,  648, 516,  650, 518,  648, 522,  648, 516,  648, 1640,  648, 506,  648, 508,  650, 510,  648, 514,  648, 520,  648, 522,  648, 490,  642};  // WHIRLPOOL_AC
  irsend.sendRaw(rawData, 343, 38000);
  handleRoot();
}


void AcTurbo(){
  uint16_t rawData[343] = {9088, 4620,  564, 1718,  566, 1720,  566, 584,  566, 588,  564, 592,  564, 596,  564, 598,  566, 1712,  582, 578,  564, 1722,  564, 1724,  566, 588,  564, 592,  564, 594,  566, 598,  564, 588,  564, 1718,  566, 580,  566, 586,  564, 588,  564, 592,  566, 596,  564, 598,  566, 588,  566, 578,  566, 1720,  564, 586,  564, 588,  564, 592,  564, 594,  566, 598,  566, 588,  566, 576,  564, 582,  566, 584,  564, 588,  564, 592,  564, 596,  564, 600,  564, 588,  564, 578,  564, 580,  566, 584,  564, 588,  566, 1730,  564, 596,  564, 598,  566, 1710,  566, 8120,  568, 578,  566, 582,  566, 586,  566, 588,  566, 592,  566, 596,  566, 600,  566, 1730,  566, 578,  566, 582,  566, 1724,  566, 1730,  566, 1734,  566, 596,  566, 600,  566, 588,  566, 578,  566, 582,  566, 586,  566, 590,  564, 594,  566, 596,  566, 600,  566, 588,  566, 578,  566, 582,  566, 586,  566, 590,  566, 594,  566, 596,  566, 600,  566, 590,  566, 578,  566, 582,  566, 586,  566, 588,  566, 594,  566, 596,  566, 598,  566, 590,  564, 578,  566, 580,  566, 586,  564, 590,  566, 574,  584, 596,  566, 600,  566, 588,  566, 578,  566, 582,  566, 586,  566, 588,  566, 592,  566, 596,  566, 600,  566, 590,  564, 1718,  566, 1722,  566, 1726,  566, 1730,  566, 592,  566, 596,  566, 600,  564, 572,  566, 8126,  566, 578,  566, 580,  566, 586,  564, 588,  566, 592,  566, 596,  566, 600,  566, 588,  566, 580,  566, 582,  566, 1726,  566, 588,  566, 592,  566, 596,  566, 600,  566, 588,  566, 578,  566, 582,  566, 586,  566, 590,  566, 592,  566, 596,  564, 600,  566, 588,  566, 578,  566, 582,  566, 586,  566, 590,  566, 592,  566, 596,  566, 600,  566, 588,  566, 578,  566, 582,  566, 586,  566, 1730,  566, 592,  564, 596,  566, 600,  566, 590,  566, 578,  566, 582,  566, 586,  566, 588,  566, 592,  566, 596,  566, 600,  566, 588,  566, 578,  566, 582,  566, 1728,  566, 1732,  566, 594,  566, 596,  566, 600,  566, 570,  558};  // WHIRLPOOL_AC
  irsend.sendRaw(rawData, 343, 38000);
  AcPage();
}

void AcDim(){
  uint16_t rawData[343] = {9092, 4614,  566, 1716,  564, 1718,  564, 584,  564, 588,  564, 592,  564, 596,  564, 598,  564, 1726,  564, 576,  564, 1718,  564, 1722,  564, 588,  564, 592,  564, 594,  564, 598,  564, 588,  564, 1716,  564, 580,  564, 586,  564, 588,  564, 592,  564, 594,  564, 598,  564, 588,  564, 578,  564, 1720,  564, 584,  564, 588,  564, 592,  564, 594,  564, 598,  564, 588,  564, 578,  564, 580,  564, 586,  564, 588,  564, 592,  564, 594,  564, 598,  564, 586,  564, 560,  582, 580,  564, 584,  564, 588,  564, 1730,  564, 594,  566, 598,  564, 570,  566, 8108,  566, 576,  564, 582,  564, 584,  564, 588,  564, 592,  564, 1732,  566, 598,  564, 1726,  564, 576,  564, 580,  566, 1722,  564, 1726,  564, 1730,  564, 596,  564, 598,  564, 588,  566, 576,  564, 580,  564, 584,  564, 588,  564, 592,  564, 596,  564, 598,  564, 588,  564, 576,  566, 580,  564, 584,  564, 588,  564, 592,  564, 596,  564, 598,  566, 586,  564, 578,  564, 580,  564, 584,  564, 588,  564, 592,  564, 596,  564, 598,  566, 588,  564, 578,  564, 580,  564, 586,  562, 588,  566, 592,  564, 594,  564, 598,  564, 588,  564, 578,  564, 580,  564, 584,  564, 588,  566, 592,  564, 596,  564, 598,  564, 588,  564, 1716,  564, 1720,  564, 1724,  566, 1728,  564, 590,  566, 1734,  566, 598,  564, 1710,  566, 8114,  566, 578,  564, 582,  564, 586,  564, 588,  566, 592,  564, 596,  564, 600,  564, 588,  566, 578,  566, 580,  566, 584,  564, 588,  564, 592,  566, 594,  566, 598,  566, 588,  564, 578,  564, 580,  564, 584,  564, 588,  564, 592,  564, 596,  564, 598,  564, 588,  566, 578,  566, 580,  564, 584,  566, 588,  566, 592,  564, 596,  564, 598,  564, 588,  564, 576,  564, 580,  564, 584,  564, 1726,  566, 592,  564, 596,  564, 600,  564, 588,  564, 576,  564, 582,  564, 584,  564, 588,  564, 592,  564, 596,  564, 600,  564, 588,  564, 578,  564, 580,  564, 584,  564, 1726,  566, 592,  564, 596,  564, 598,  564, 570,  558};  // WHIRLPOOL_AC
  irsend.sendRaw(rawData, 343, 38000);
  AcPage();
}

void AcPowerSaver(){
  uint16_t rawData[343] = {9150, 4562,  620, 1662,  618, 1666,  618, 530,  618, 534,  618, 536,  596, 564,  620, 544,  620, 1672,  618, 522,  620, 1664,  620, 1666,  620, 534,  618, 536,  618, 540,  618, 544,  620, 532,  618, 1662,  618, 1664,  620, 530,  618, 534,  618, 536,  620, 540,  620, 544,  620, 532,  618, 522,  618, 1666,  618, 530,  618, 534,  620, 1676,  618, 540,  618, 544,  620, 1670,  618, 524,  618, 526,  618, 530,  620, 532,  620, 536,  620, 540,  618, 544,  618, 534,  618, 524,  618, 526,  618, 530,  620, 532,  620, 536,  618, 542,  618, 544,  618, 516,  620, 8034,  612, 548,  618, 526,  618, 530,  620, 532,  620, 536,  618, 540,  618, 544,  618, 1672,  618, 1660,  618, 526,  618, 1668,  618, 534,  618, 1674,  618, 540,  620, 544,  618, 534,  618, 522,  620, 526,  616, 530,  618, 532,  618, 536,  620, 540,  618, 544,  618, 534,  618, 522,  592, 550,  618, 528,  618, 532,  618, 536,  620, 538,  618, 542,  618, 532,  618, 522,  620, 526,  618, 530,  618, 532,  618, 536,  618, 540,  620, 544,  618, 532,  618, 522,  618, 526,  618, 530,  618, 534,  618, 536,  618, 540,  618, 544,  620, 534,  618, 522,  620, 526,  594, 554,  618, 534,  618, 538,  618, 540,  618, 544,  620, 534,  618, 522,  618, 526,  618, 1666,  620, 532,  618, 536,  618, 540,  618, 544,  618, 514,  620, 8048,  620, 522,  620, 524,  620, 528,  618, 532,  618, 536,  620, 540,  618, 542,  618, 532,  618, 522,  618, 526,  618, 530,  618, 532,  618, 538,  618, 540,  620, 542,  618, 534,  618, 522,  618, 526,  620, 530,  620, 532,  618, 536,  618, 540,  620, 544,  618, 534,  618, 522,  618, 526,  618, 530,  620, 532,  618, 536,  620, 540,  620, 544,  620, 534,  618, 524,  618, 526,  618, 530,  618, 1672,  618, 536,  618, 540,  618, 544,  618, 532,  618, 522,  620, 524,  620, 530,  620, 532,  620, 536,  620, 540,  618, 544,  618, 532,  620, 522,  618, 524,  620, 530,  620, 1670,  602, 552,  618, 540,  618, 542,  620, 514,  612};  // WHIRLPOOL_AC
  irsend.sendRaw(rawData, 343, 38000);
  AcPage();
}

void AcWinter(){
  uint16_t rawData[343] = {9096, 4616,  564, 1716,  566, 1720,  564, 584,  564, 588,  564, 592,  564, 594,  566, 598,  564, 1728,  564, 576,  566, 1720,  566, 1724,  564, 588,  564, 592,  564, 594,  564, 598,  564, 588,  564, 576,  564, 580,  564, 586,  564, 586,  564, 590,  564, 594,  564, 598,  564, 586,  566, 1714,  566, 1718,  566, 584,  564, 588,  564, 1730,  564, 1734,  564, 1736,  564, 588,  564, 578,  564, 580,  564, 584,  564, 588,  564, 592,  564, 594,  566, 600,  564, 588,  566, 576,  566, 580,  564, 584,  564, 588,  564, 574,  582, 594,  564, 598,  564, 572,  564, 8112,  566, 576,  564, 580,  566, 584,  564, 588,  564, 592,  564, 594,  566, 598,  564, 1728,  564, 578,  564, 1720,  566, 584,  564, 1726,  564, 1730,  564, 594,  564, 600,  564, 588,  564, 578,  566, 580,  566, 584,  566, 588,  564, 590,  566, 594,  564, 598,  566, 588,  564, 576,  564, 580,  564, 584,  564, 588,  564, 592,  564, 594,  564, 598,  566, 588,  564, 576,  566, 580,  566, 584,  564, 588,  564, 590,  566, 594,  564, 598,  564, 588,  566, 576,  564, 580,  566, 584,  564, 572,  582, 590,  566, 594,  564, 600,  564, 588,  564, 576,  566, 580,  564, 584,  564, 588,  564, 592,  564, 596,  564, 598,  566, 588,  564, 1716,  564, 580,  564, 584,  566, 1726,  566, 592,  564, 1734,  564, 1736,  564, 1708,  564, 8092,  584, 576,  566, 580,  564, 584,  564, 588,  564, 592,  564, 594,  564, 580,  584, 588,  564, 576,  566, 1720,  564, 1722,  564, 588,  564, 592,  564, 596,  564, 598,  564, 588,  566, 576,  564, 582,  564, 584,  564, 588,  564, 592,  564, 596,  564, 600,  564, 588,  566, 576,  566, 580,  564, 584,  564, 588,  564, 592,  564, 594,  564, 598,  564, 588,  566, 578,  564, 580,  564, 584,  564, 1726,  564, 592,  564, 594,  566, 598,  564, 588,  564, 576,  566, 580,  564, 584,  564, 588,  564, 590,  566, 594,  566, 598,  564, 588,  564, 576,  564, 1720,  564, 1722,  564, 1726,  566, 590,  564, 594,  564, 598,  564, 570,  556};  // WHIRLPOOL_AC
  irsend.sendRaw(rawData, 343, 38000);
  AcPage();
}

void Ac30(){
  uint16_t rawData[343] = {9128, 4586,  596, 1684,  596, 1688,  566, 584,  594, 558,  596, 560,  596, 566,  594, 568,  596, 1696,  596, 546,  596, 1690,  594, 1694,  596, 558,  594, 562,  594, 564,  596, 568,  594, 558,  596, 548,  594, 550,  596, 554,  594, 558,  596, 560,  594, 566,  594, 568,  594, 540,  614, 548,  594, 1690,  596, 554,  594, 558,  564, 590,  596, 1702,  566, 1738,  594, 1696,  596, 548,  594, 550,  564, 584,  594, 558,  596, 560,  594, 564,  594, 568,  594, 558,  596, 546,  594, 550,  594, 554,  594, 558,  594, 562,  594, 564,  596, 568,  594, 540,  596, 8084,  596, 546,  594, 550,  594, 554,  594, 558,  594, 562,  594, 566,  594, 568,  594, 1696,  594, 1686,  594, 550,  596, 1694,  592, 1696,  594, 1700,  564, 600,  560, 598,  594, 558,  594, 548,  594, 550,  594, 554,  594, 558,  594, 560,  594, 564,  594, 568,  594, 556,  594, 546,  594, 552,  596, 554,  594, 558,  594, 560,  594, 564,  596, 568,  594, 558,  564, 576,  592, 552,  594, 554,  594, 558,  596, 562,  594, 564,  594, 568,  594, 558,  594, 546,  564, 580,  594, 554,  594, 560,  594, 560,  594, 564,  596, 568,  594, 558,  594, 548,  594, 552,  594, 554,  564, 588,  594, 560,  594, 564,  594, 568,  594, 558,  594, 1686,  596, 1690,  594, 1692,  594, 1698,  594, 1700,  596, 1702,  566, 1736,  596, 540,  594, 8080,  596, 546,  594, 550,  596, 554,  594, 558,  594, 562,  594, 564,  596, 568,  594, 558,  594, 546,  596, 1690,  594, 554,  594, 560,  594, 562,  594, 566,  594, 568,  594, 558,  594, 548,  596, 550,  594, 554,  594, 558,  594, 562,  564, 594,  594, 568,  594, 558,  594, 548,  596, 550,  594, 554,  594, 558,  594, 560,  596, 564,  594, 568,  594, 558,  594, 548,  594, 552,  594, 554,  594, 1698,  594, 562,  594, 564,  594, 568,  594, 558,  596, 546,  594, 552,  594, 554,  594, 558,  596, 560,  596, 564,  594, 568,  594, 556,  596, 546,  594, 1688,  594, 554,  596, 1696,  596, 560,  594, 564,  594, 568,  594, 540,  556};  // WHIRLPOOL_AC
  irsend.sendRaw(rawData, 343, 38000);
  AcPage();
}

void Ac29(){
  uint16_t rawData[343] = {9112, 4586,  596, 1686,  594, 1690,  596, 552,  596, 556,  596, 560,  594, 564,  594, 568,  596, 1696,  596, 546,  596, 1690,  596, 1692,  596, 558,  594, 560,  596, 564,  594, 568,  596, 556,  594, 548,  596, 550,  594, 554,  594, 558,  596, 560,  598, 562,  596, 568,  596, 556,  596, 546,  596, 1688,  594, 552,  596, 556,  594, 1700,  594, 564,  594, 1708,  594, 1696,  596, 546,  598, 548,  594, 554,  594, 558,  594, 562,  596, 564,  596, 568,  596, 558,  596, 546,  594, 552,  596, 552,  596, 556,  598, 560,  596, 564,  594, 570,  596, 540,  596, 8084,  596, 546,  596, 550,  596, 554,  596, 556,  596, 562,  594, 564,  594, 568,  594, 1698,  594, 1686,  596, 550,  594, 1692,  594, 1698,  596, 1698,  596, 564,  594, 568,  596, 558,  596, 546,  596, 550,  596, 554,  594, 556,  596, 562,  594, 564,  596, 568,  594, 558,  596, 546,  594, 552,  594, 534,  614, 558,  594, 562,  594, 564,  596, 568,  594, 558,  596, 546,  596, 552,  596, 554,  596, 556,  596, 562,  594, 564,  594, 568,  596, 558,  594, 546,  596, 550,  596, 554,  594, 556,  596, 562,  594, 564,  596, 566,  596, 556,  594, 546,  596, 550,  598, 552,  596, 556,  594, 560,  596, 564,  598, 566,  594, 558,  596, 1686,  594, 1690,  594, 1692,  594, 1696,  594, 562,  596, 564,  594, 1708,  594, 540,  596, 8080,  600, 544,  596, 550,  594, 554,  596, 556,  594, 562,  596, 564,  596, 568,  596, 556,  594, 548,  596, 1690,  594, 554,  596, 558,  596, 560,  596, 564,  596, 568,  596, 558,  596, 546,  598, 550,  596, 554,  596, 556,  596, 562,  596, 564,  596, 568,  596, 558,  592, 550,  596, 550,  596, 552,  596, 558,  596, 560,  594, 566,  594, 568,  596, 556,  594, 548,  594, 550,  594, 554,  596, 1696,  594, 560,  596, 562,  596, 568,  594, 558,  596, 546,  594, 552,  594, 554,  596, 556,  594, 562,  594, 566,  596, 568,  596, 556,  596, 548,  594, 1690,  596, 554,  594, 1698,  596, 560,  596, 564,  596, 568,  594, 542,  588};  // WHIRLPOOL_AC
  irsend.sendRaw(rawData, 343, 38000);
  AcPage();
}

void Ac28(){
  uint16_t rawData[343] = {9096, 4614,  564, 1714,  566, 1718,  564, 584,  564, 588,  564, 592,  564, 594,  564, 598,  564, 1726,  564, 576,  564, 1720,  562, 1722,  564, 586,  564, 592,  564, 596,  564, 598,  564, 588,  564, 576,  564, 580,  564, 584,  564, 588,  564, 592,  564, 594,  564, 598,  564, 588,  564, 576,  594, 1672,  612, 554,  564, 588,  564, 590,  594, 564,  594, 1706,  594, 1694,  594, 548,  594, 550,  594, 556,  594, 556,  596, 560,  564, 596,  564, 598,  594, 556,  596, 546,  594, 550,  594, 554,  594, 558,  596, 560,  594, 564,  594, 568,  596, 538,  594, 8076,  594, 546,  594, 550,  596, 552,  596, 556,  594, 560,  596, 564,  594, 568,  594, 1694,  596, 546,  596, 1688,  596, 1690,  594, 1696,  596, 1700,  594, 564,  596, 568,  594, 556,  596, 546,  594, 550,  594, 554,  596, 536,  614, 560,  620, 540,  594, 566,  620, 532,  594, 546,  594, 548,  596, 552,  596, 556,  596, 560,  596, 564,  620, 544,  594, 558,  594, 546,  620, 526,  618, 530,  620, 532,  620, 536,  596, 564,  594, 568,  594, 558,  594, 546,  620, 524,  594, 554,  618, 532,  594, 562,  594, 564,  618, 544,  616, 534,  594, 548,  612, 532,  594, 554,  594, 558,  596, 558,  616, 542,  594, 568,  594, 558,  618, 524,  594, 550,  592, 1694,  592, 1696,  592, 1700,  592, 566,  592, 1708,  592, 540,  592, 8074,  592, 550,  590, 552,  590, 556,  590, 560,  592, 564,  592, 568,  590, 572,  590, 560,  588, 552,  590, 1692,  588, 560,  588, 562,  590, 566,  590, 568,  590, 572,  590, 562,  586, 554,  588, 556,  586, 562,  588, 562,  588, 568,  540, 620,  538, 624,  588, 564,  538, 602,  540, 604,  540, 608,  540, 612,  540, 616,  540, 620,  538, 624,  538, 612,  538, 602,  540, 606,  538, 610,  540, 1750,  538, 616,  540, 620,  540, 622,  538, 612,  538, 602,  538, 606,  538, 608,  540, 612,  540, 616,  538, 620,  540, 624,  538, 614,  538, 602,  538, 1744,  538, 610,  538, 1750,  538, 618,  538, 620,  538, 624,  538, 594,  534};  // KELON168
  irsend.sendRaw(rawData, 343, 38000);
  AcPage();
}

void Ac27(){
  uint16_t rawData[343] = {9110, 4620,  566, 1718,  566, 1722,  566, 584,  566, 588,  566, 592,  566, 596,  566, 598,  566, 1730,  564, 578,  566, 1722,  564, 1706,  584, 588,  566, 592,  564, 596,  566, 598,  566, 586,  564, 578,  564, 582,  566, 584,  566, 586,  566, 592,  564, 596,  566, 598,  564, 588,  566, 578,  566, 1722,  566, 584,  566, 588,  564, 1732,  564, 1734,  564, 598,  564, 1726,  564, 576,  564, 580,  564, 584,  564, 588,  564, 592,  564, 576,  582, 598,  564, 588,  564, 576,  564, 580,  564, 584,  564, 588,  564, 592,  564, 594,  566, 598,  566, 570,  566, 8110,  568, 556,  582, 580,  564, 584,  564, 588,  564, 592,  564, 594,  566, 598,  564, 1726,  566, 578,  564, 1720,  564, 1724,  564, 1728,  564, 1732,  564, 594,  566, 600,  564, 588,  564, 576,  566, 582,  564, 584,  564, 588,  564, 592,  564, 596,  564, 598,  564, 588,  564, 578,  564, 582,  564, 586,  566, 588,  564, 592,  566, 596,  564, 598,  566, 588,  564, 578,  564, 580,  564, 584,  564, 588,  566, 592,  566, 594,  566, 598,  564, 588,  566, 576,  564, 582,  564, 584,  564, 588,  564, 592,  564, 594,  564, 598,  566, 588,  564, 576,  564, 580,  564, 584,  564, 588,  566, 590,  564, 596,  566, 598,  564, 588,  564, 578,  564, 582,  562, 1724,  564, 1726,  566, 592,  564, 1734,  564, 598,  564, 570,  564, 8112,  566, 576,  564, 580,  564, 584,  564, 588,  564, 592,  564, 596,  564, 598,  564, 588,  564, 576,  566, 1718,  566, 584,  564, 588,  564, 590,  566, 594,  564, 598,  564, 588,  566, 576,  564, 580,  564, 584,  564, 588,  564, 592,  564, 596,  564, 598,  566, 586,  566, 578,  564, 582,  564, 584,  564, 588,  564, 592,  564, 594,  564, 598,  564, 588,  564, 578,  564, 580,  566, 584,  564, 1728,  564, 592,  564, 594,  564, 598,  566, 586,  564, 576,  566, 580,  564, 584,  564, 588,  566, 594,  562, 594,  566, 600,  564, 588,  564, 578,  564, 1722,  562, 584,  566, 1728,  564, 592,  564, 596,  566, 598,  564, 570,  558};  // WHIRLPOOL_AC
  irsend.sendRaw(rawData, 343, 38000);
  AcPage();
}

void Ac26(){
  uint16_t rawData[343] = {9132, 4588,  596, 1684,  596, 1690,  596, 554,  594, 558,  596, 562,  596, 564,  596, 568,  594, 1696,  596, 546,  596, 1688,  596, 1692,  596, 558,  596, 560,  596, 564,  596, 568,  596, 558,  594, 548,  596, 548,  596, 556,  596, 556,  596, 560,  596, 564,  596, 568,  596, 556,  596, 548,  594, 1690,  594, 554,  596, 556,  596, 560,  594, 1704,  596, 568,  594, 1696,  596, 546,  596, 550,  596, 554,  594, 558,  596, 560,  594, 564,  596, 568,  596, 556,  596, 546,  596, 550,  596, 552,  596, 556,  596, 560,  594, 564,  596, 568,  594, 540,  594, 8078,  596, 546,  596, 550,  596, 554,  596, 558,  596, 560,  596, 564,  598, 566,  596, 1694,  596, 546,  596, 1690,  596, 1692,  596, 1696,  596, 1700,  596, 564,  596, 568,  598, 556,  596, 546,  596, 550,  596, 554,  596, 558,  596, 560,  596, 564,  596, 566,  596, 558,  596, 548,  594, 550,  596, 554,  596, 558,  596, 562,  596, 566,  594, 568,  596, 558,  596, 546,  596, 550,  596, 554,  596, 556,  596, 560,  596, 564,  596, 566,  596, 558,  596, 548,  594, 552,  596, 554,  596, 558,  596, 562,  596, 564,  594, 568,  596, 556,  594, 548,  596, 550,  596, 554,  596, 558,  596, 562,  596, 564,  596, 568,  596, 556,  594, 548,  596, 550,  596, 1692,  596, 1696,  596, 1700,  596, 1702,  596, 568,  594, 540,  596, 8082,  596, 546,  596, 550,  596, 552,  596, 558,  596, 560,  596, 564,  594, 568,  594, 558,  596, 546,  596, 1688,  596, 554,  594, 560,  594, 560,  596, 564,  596, 566,  596, 558,  596, 546,  594, 550,  596, 552,  594, 558,  596, 560,  596, 564,  596, 566,  596, 556,  596, 546,  596, 550,  594, 554,  594, 558,  596, 560,  596, 564,  596, 568,  596, 556,  594, 548,  594, 550,  596, 552,  594, 1678,  614, 560,  596, 564,  596, 568,  596, 556,  596, 546,  596, 550,  594, 554,  596, 556,  596, 562,  596, 564,  598, 566,  596, 556,  598, 546,  596, 1688,  596, 552,  596, 1696,  596, 560,  596, 564,  596, 566,  596, 538,  556};  // WHIRLPOOL_AC
  irsend.sendRaw(rawData, 343, 38000);
  AcPage();
}

void Ac25(){
  uint16_t rawData[343] = {9098, 4616,  564, 1718,  564, 1720,  564, 584,  564, 588,  566, 592,  564, 596,  564, 600,  564, 1728,  566, 576,  566, 1720,  566, 1724,  564, 588,  564, 592,  566, 596,  564, 600,  564, 588,  566, 578,  566, 580,  564, 584,  566, 588,  564, 592,  566, 594,  566, 598,  564, 588,  564, 578,  564, 1722,  564, 584,  564, 588,  564, 1730,  564, 596,  564, 598,  564, 1728,  564, 578,  564, 580,  566, 584,  566, 588,  564, 592,  564, 596,  564, 600,  564, 588,  566, 576,  564, 582,  566, 584,  564, 588,  566, 590,  566, 594,  566, 598,  564, 570,  566, 8114,  566, 578,  564, 582,  564, 584,  564, 588,  564, 592,  564, 596,  564, 598,  564, 1726,  566, 1714,  564, 1720,  564, 1722,  564, 1726,  564, 1732,  564, 596,  564, 600,  564, 588,  564, 578,  564, 582,  564, 584,  566, 588,  566, 592,  564, 596,  564, 600,  564, 588,  566, 578,  566, 580,  564, 584,  564, 588,  566, 592,  564, 594,  564, 598,  566, 588,  566, 560,  582, 580,  566, 586,  564, 588,  564, 592,  566, 596,  564, 598,  566, 586,  566, 578,  564, 582,  566, 584,  564, 588,  564, 592,  564, 596,  566, 598,  564, 588,  564, 578,  564, 582,  564, 584,  564, 586,  566, 592,  566, 594,  564, 598,  566, 588,  566, 1716,  564, 580,  566, 1724,  564, 1728,  566, 592,  564, 594,  566, 598,  564, 570,  566, 8114,  566, 576,  566, 580,  566, 584,  566, 588,  566, 592,  564, 596,  564, 598,  564, 588,  564, 576,  566, 1720,  564, 584,  568, 584,  564, 592,  564, 596,  564, 598,  564, 588,  564, 578,  564, 582,  566, 584,  564, 588,  564, 592,  564, 596,  566, 598,  566, 588,  566, 578,  564, 580,  564, 584,  564, 588,  564, 592,  564, 594,  564, 598,  564, 588,  564, 578,  564, 580,  564, 584,  564, 1728,  564, 592,  564, 594,  566, 598,  564, 588,  564, 576,  564, 580,  564, 584,  564, 588,  566, 592,  564, 596,  564, 598,  564, 588,  564, 576,  566, 1718,  564, 584,  566, 1726,  564, 592,  566, 594,  564, 598,  564, 570,  558};  // WHIRLPOOL_AC
  irsend.sendRaw(rawData, 343, 38000);
  AcPage();
}

void Ac24(){
  uint16_t rawData[343] = {9100, 4618,  596, 1686,  596, 1690,  566, 584,  564, 588,  564, 590,  594, 564,  566, 598,  594, 1696,  594, 548,  592, 1692,  596, 1694,  594, 560,  566, 590,  566, 594,  596, 568,  594, 558,  594, 548,  566, 580,  594, 1694,  594, 558,  564, 590,  596, 564,  596, 568,  594, 558,  564, 576,  594, 1692,  594, 554,  594, 558,  596, 562,  594, 564,  594, 570,  564, 1728,  564, 576,  564, 580,  596, 554,  594, 560,  564, 592,  564, 594,  596, 568,  594, 560,  592, 548,  596, 550,  566, 584,  564, 588,  564, 592,  564, 596,  594, 570,  594, 542,  566, 8114,  564, 578,  594, 550,  566, 584,  594, 558,  594, 562,  594, 564,  594, 570,  592, 1698,  566, 576,  594, 1690,  564, 584,  564, 586,  594, 1702,  566, 594,  566, 598,  594, 558,  566, 576,  594, 550,  596, 554,  594, 558,  594, 564,  594, 566,  594, 568,  570, 582,  566, 576,  596, 550,  594, 554,  596, 558,  594, 562,  596, 564,  564, 598,  594, 558,  594, 548,  566, 580,  596, 554,  564, 588,  566, 590,  594, 566,  564, 598,  594, 558,  594, 548,  594, 550,  594, 554,  594, 558,  594, 562,  594, 566,  594, 568,  596, 558,  564, 576,  594, 552,  594, 554,  594, 558,  594, 560,  594, 566,  594, 568,  596, 556,  594, 546,  596, 550,  596, 1692,  566, 588,  594, 1700,  594, 566,  566, 598,  564, 570,  566, 8108,  596, 546,  594, 552,  594, 554,  596, 558,  594, 562,  594, 564,  596, 568,  596, 558,  594, 1684,  596, 550,  594, 554,  566, 586,  566, 590,  594, 566,  594, 570,  594, 556,  566, 578,  564, 580,  594, 554,  566, 588,  564, 592,  596, 566,  592, 570,  594, 558,  594, 548,  564, 580,  596, 554,  594, 558,  596, 562,  594, 566,  594, 568,  594, 558,  594, 548,  596, 550,  594, 554,  596, 1698,  564, 592,  594, 568,  592, 570,  594, 558,  566, 578,  594, 550,  594, 554,  594, 558,  594, 562,  596, 566,  594, 568,  594, 558,  566, 1716,  596, 550,  594, 554,  596, 1696,  596, 560,  594, 564,  566, 598,  594, 542,  558};  // WHIRLPOOL_AC
  irsend.sendRaw(rawData, 343, 38000);
  AcPage();
}

//irsend.sendNEC(0xFB38C7, 32); //croma tv on/off
void TvOn(){
  //irsend.sendNEC(0xFB38C7, 32); croma 49" Power
  irsend.sendSony(0x540A, 15); // Sony Home theater Power
  TvPage();
}

void TvMenu(){
  irsend.sendNEC(0xFBE817, 32);
  TvPage();
}

void TvSource(){
  irsend.sendNEC(0xFB926D, 32);
  TvPage();
}

void TvVolUp(){
  // irsend.sendNEC(0xFBD22D, 32); croma 49" Vol+
  irsend.sendSony(0x240A, 15); // Sony Home theater Vol+
  TvPage();
}

void TvVolDown(){
  //irsend.sendNEC(0xFBF20D, 32); croma 49" Vol-
  irsend.sendSony(0x640A, 15); // Sony Home theater Vol-
  TvPage();
}

void TvUp(){
  irsend.sendNEC(0xFB58A7, 32);
  TvPage();
}

void TvExit(){
  irsend.sendNEC(0xFB9A65, 32);
  TvPage();
}

void TvLeft(){
  irsend.sendNEC(0xFBE21D, 32);
  TvPage();
}

void TvOk(){
  irsend.sendNEC(0xFB609F, 32);
  TvPage();
}

void TvRight(){
  irsend.sendNEC(0xFBE01F, 32);
  TvPage();
}

void TvDown(){
  irsend.sendNEC(0xFB12ED, 32);
  TvPage();
}

void TvEco(){
  irsend.sendNEC(0xFBEA15, 32);
  TvPage();
}

/* 
IR Code
pw 0x1FE48B7 (32 Bits) Tv On
md 0x1FE58A7 (32 Bits) Eco
mu 0x1FE7887 (32 Bits) TvExit
pl 0x1FE807F (32 Bits) Ac on
bw 0x1FE40BF (32 Bits) Turbo
fw 0x1FEC03F (32 Bits) Dim
e  0x1FE20DF (32 Bits) Socket
v- 0x1FEA05F (32 Bits) Tv Vol-
v+ 0x1FE609F (32 Bits) Tv Vol+
0  0x1FEE01F (32 Bits) Fan Off
rt 0x1FE10EF (32 Bits) Night
sd 0x1FE906F (32 Bits) Light
1  0x1FE50AF (32 Bits) Fan Speed 1
2  0x1FED827 (32 Bits) Fan Speed 2
3  0x1FEF807 (32 Bits) Fan Speed 3
4  0x1FE30CF (32 Bits) Fan Speed 4
5  0x1FEB04F (32 Bits) Fan Speed 5
6  0x1FE708F (32 Bits) Ac Temp 26
7  0x1FE00FF (32 Bits) Ac Temp 27
8  0x1FEF00F (32 Bits) Ac Temp 28
9  0x1FE9867 (32 Bits) Ac Temp 29
*/

void ir_remote() {
  if (irrecv.decode(&results)) {
    Serial.println(results.value, HEX);  
    if (results.value==IR1) {
      TvOn();
    }
    else if (results.value==IR2) {
      TvEco();
    }
    else if (results.value==IR3) {
      TvExit();
    }
    else if (results.value==IR4) {
      AcOn();
    }
    else if (results.value==IR5) {
      AcTurbo();
    }
    else if (results.value==IR6) {
      AcDim();
    }
    else if (results.value==IR7) {
      switch1function();
    }
    else if (results.value==IR8) {
      TvVolDown();
    }
    else if (results.value==IR9) {
      TvVolUp();
    }
    else if (results.value==IR10) {
      switch2function();
    }
    else if (results.value==IR11) {
      switch4function();
    }
    else if (results.value==IRF0) {
      FanOFF();
    }
    else if (results.value==IRF1) {
      FanSpeed1();
    }
    else if (results.value==IRF2) {
      FanSpeed2();
    }
    else if (results.value==IRF3) {
      FanSpeed3();
    }
    else if (results.value==IRF4) {
      FanSpeed4();
    }
    else if (results.value==IRF5) {
      FanSpeed5();
    }
    else if (results.value==IRF6) {
      Ac26();
    }
    else if (results.value==IRF7) {
      Ac27();
    }
    else if (results.value==IRF8) {
      Ac28();
    }
    else if (results.value==IRF9) {
      Ac29();
    }
    else {
      Serial.println("Wrong IR button pressed"); 
      delay(100);
    }
    irrecv.resume(); // receive the next value
  }
}

bool previous_sw1_val = 0;
bool previous_sw2_val = 0;
bool previous_sw3_val = 0;
bool previous_sw4_val = 0;

void switch_control() {
  bool sw1 = digitalRead(switch1pin); //SK
  bool sw2 = digitalRead(switch2pin); //D6
  bool sw3 = digitalRead(switch3pin); //D7
  bool sw4 = digitalRead(switch4pin); //D8

  if (sw1 != previous_sw1_val) {
    switch1function();
    previous_sw1_val = sw1;
  }
  if (sw2 != previous_sw2_val) {
    switch2function();
    previous_sw2_val = sw2;
  }
  if (sw3 != previous_sw3_val) {
    switch3function();
    previous_sw3_val = sw3;
  }
  if (sw4 != previous_sw4_val) {
    switch4function();
    previous_sw4_val = sw4;
  }
  delay(500);
}

void setup(){
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);

  pinMode(switch1pin, INPUT);
  pinMode(switch2pin, INPUT);
  pinMode(switch3pin, INPUT);
  pinMode(switch4pin, INPUT);

  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);
  digitalWrite(relay3, LOW);
  digitalWrite(relay4, LOW);

  previous_sw1_val = digitalRead(switch1pin);
  previous_sw2_val = digitalRead(switch2pin);
  previous_sw3_val = digitalRead(switch3pin);
  previous_sw4_val = digitalRead(switch4pin);  
  
  irsend.begin();

  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); 
 
  server.on("/", handleRoot);    
  server.on("/Ac", AcPage);
  server.on("/Tv", TvPage);
  server.on("/ir", handleIr);
  server.on("/All", Allfunction);
  server.on("/Switch1", switch1function); 
  server.on("/Switch2", switch2function);  
  server.on("/Switch4", switch4function);
  server.on("/FL", FLfunction);
  server.on("/FanOFF", FanOFF); 
  server.on("/FanSpeed1", FanSpeed1);   
  server.on("/FanSpeed2", FanSpeed2);
  server.on("/FanSpeed3", FanSpeed3);
  server.on("/FanSpeed4", FanSpeed4);  
  server.on("/FanSpeed5", FanSpeed5); 
  server.on("/AcON", AcOn);   
  server.on("/AcTurbo", AcTurbo);   
  server.on("/AcDim", AcDim);
  server.on("/AcPowerSaver", AcPowerSaver);
  server.on("/AcWinter", AcWinter);
  server.on("/Ac30", Ac30);
  server.on("/Ac29", Ac29);
  server.on("/Ac28", Ac28);
  server.on("/Ac27", Ac27);
  server.on("/Ac26", Ac26);
  server.on("/Ac25", Ac25);
  server.on("/Ac24", Ac24);
  server.on("/TvOn", TvOn); 
  server.on("/TvMenu", TvMenu);
  server.on("/TvSource", TvSource);
  server.on("/TvVolUp", TvVolUp);
  server.on("/TvUp", TvUp);
  server.on("/TvExit", TvExit);
  server.on("/TvLeft", TvLeft);
  server.on("/TvOk", TvOk);
  server.on("/TvRight", TvRight);
  server.on("/TvVolDown", TvVolDown);
  server.on("/TvDown", TvDown);
  server.on("/TvEco", TvEco);

  server.begin();     
  Serial.println("HTTP server started");

//----------------------------------------------------IR Reciver

  #if defined(ESP8266)
    Serial.begin(kBaudRate, SERIAL_8N1, SERIAL_TX_ONLY);
  #else 
    Serial.begin(kBaudRate, SERIAL_8N1);
  #endif  
    while (!Serial) 
      delay(50);
    assert(irutils::lowLevelSanityCheck() == 0);
    Serial.printf("\n" D_STR_IRRECVDUMP_STARTUP "\n", kRecvPin);
  #if DECODE_HASH
    irrecv.setUnknownThreshold(kMinUnknownSize);
  #endif 
    irrecv.setTolerance(kTolerancePercentage);  
    irrecv.enableIRIn();

//----------------------------------------------------IR Reciver

}

void loop(){   
  sensorValue = analogRead(A0);
  if (sensorValue > 900 && 1000 > sensorValue) {
    Serial.println(sensorValue);
    if (Fan == "OFF") {
      FanSpeed1();
      delay(1000);
    }
    if (Fan == "Speed1") {
      FanSpeed2();
      delay(1000);
    }
    if (Fan == "Speed2") {
      FanSpeed3();
      delay(1000);
    }
    if (Fan == "Speed3") {
      FanSpeed4();
      delay(1000);
    }
    if (Fan == "Speed4") {
      FanSpeed5();
      delay(1000);
    }
  }
  if (sensorValue > 1000 && 1024 > sensorValue) {
    Serial.println(sensorValue);
    if (Fan == "Speed5") {
      FanSpeed4();
      delay(1000);
    }
    if (Fan == "Speed4") {
      FanSpeed3();
      delay(1000);
    }
    if (Fan == "Speed3") {
      FanSpeed2();
      delay(1000);
    }
    if (Fan == "Speed2") {
      FanSpeed1();
      delay(1000);
    }
    if (Fan == "Speed1") {
      FanOFF();
      delay(1000);
    }
  }
  server.handleClient();  
  ir_remote();
  switch_control();
}
