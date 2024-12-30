/*
esp32 Coding for Three Phase Power Meter
use 74ls126 to multiplexing Rx-Tx port for PZEM 3 modules
separate into 9 tabs: defines.h,ble.h,data,h,lcd.h,time.h,voids.h,wifi.h
*/

//define.h
#include <PZEM004Tv30.h>
#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h> 
#include <Ticker.h>
#include <esp_task_wdt.h>
#define WDT_TIMEOUT 60
#include <NimBLEDevice.h> 
#include <Preferences.h>
LiquidCrystal_I2C lcd(0x27,20,4);
Preferences FLASHSTOR;
Ticker counting;
WiFiClient client;
HTTPClient http;

#define PZEM_SERIAL Serial2
#define PZEM_RX_PIN 16
#define PZEM_TX_PIN 17
PZEM004Tv30 pzem(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN);

#define pzem1  18
#define pzem2  5
#define pzem3  19
#define led 12
float vo,am,fr,pof,en;
float v1,a1,f1,pf1,e1;
float v2,a2,f2,pf2,e2;
float v3,a3,f3,pf3,e3;
int posterr_count=0;
int lcd_loop=0;


int time2server = 60;
int oneSec=0;int twoSec=0;
char serverName[500];
char buf[500];
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "asia.pool.ntp.org", 20700+4500, 60000);
char Time[ ] = "TIME:00:00:00";
char Date[ ] = "DATE:01/01/2024";
// A UDP instance to let us send and receive packets over UDP
String myTime;
byte last_second, second_, minute_, hour_, day_, month_;
int year_;
String formattedDate;
String dayStamp = "01-09-24";
String timeStamp = "01:00:00";
bool holiday_flag =0;
bool hour_flag=0;
String ipassign="192.168.1.120";
String gatewayStr = "192.168.1.1";
String subnetStr = "255.255.255.0";
String serverip = "137.184.86.182";
String devid = "e001";
String path = "iot2025";
String mcid = "E3001";//Model ID
String ssid = "TP-Link_5B9A";
String pass = "97180937";
String payload;String macStr;
String fileName = "pzemx3_oled_ble_v3.ino";
String txt0 ;
String txt1 = "********************";
String txt2 = "multi-sensor Monitor";
String txt3 = "  by ps@evolution   ";
String txt4 = "       DIPROM       ";
String txt5 = "********************";
bool post_flag=0;
bool post_err =0;
int loop_count =1;
//ble
String lan_ip = "l:....";
String prodorderno = "24PD-038228";
String mcno = "HC04";
String jobno = "PAHC";
int layer =1;String qrNumStr="1";
String emp1L,emp2L,emp3L,emp4L,emp5L;
String ordernoL,planqtyLmsg,durationLmsg,resetCLmsg;
bool clear_flag=0;bool finish_flag=0;
String workno = "work123";
String bleNameStr = "E3001-Ble";
//-------------------------------------

//main tab
#include "defines.h"
#include "lcd.h"
#include "wifi.h"
#include "pzem.h"
#include "data.h"
#include "time.h"
#include "ble.h"
#include "voids.h"

void setup() {
  Serial.begin(9600);
  Serial2.begin(9600);
  lcd.init();
  lcd.clear();
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print(" 3ph. power monitor ");
  lcd.setCursor(0,1);
  lcd.print("  **** DIPROM ****  ");
  lcd.setCursor(0,2);
  lcd.print("    @psevolution    ");
  lcd.setCursor(0,3);
  lcd.print("===================");
  pinMode(pzem1,OUTPUT);
  pinMode(pzem2,OUTPUT);
  pinMode(pzem3,OUTPUT);
  pinMode(led,OUTPUT);
  digitalWrite(pzem1,LOW);
  digitalWrite(pzem2,LOW);
  digitalWrite(pzem3,LOW);
  for(int a=0;a<3;a++){
    digitalWrite(led,HIGH);
    delay(500);
    digitalWrite(led,LOW);
    delay(500);
  }
  delay(250); // wait for the OLED to power up
  InitStorage();         Tekstprintln("Setting loaded");                                       // Load settings from storage and check validity 
  StartBLEService();     Tekstprintln("BLE started");                                           // Start BLE service                                                                                             // Print the tekst time in the display 
  init_read();
  SWversion();
  Serial.print("Starting BLE: ");
  Serial.println(bleNameStr);

  Serial.println("Connecting Wifi...");
  wifi_Connect();
  delay(1000);
  macStr.toCharArray(Mem.McAdd,macStr.length()+1);
  Serial.print("MAC address: ");
  Serial.println(String(Mem.McAdd));

  counting.attach(1,EverySecondCheck); //EverySecondCheck
  esp_task_wdt_init(WDT_TIMEOUT, true);
  esp_task_wdt_add(NULL);
  lcd.clear();
}
void loop() {
    esp_task_wdt_reset();
    SerialCheck();
    CheckBLE(); 
    Serial.print("loop_count");
    Serial.println(loop_count);
    switch (loop_count){
      case 1:
        //lcd.backlight();
        Serial.println("Read pzem #1");
        digitalWrite(pzem1,HIGH);
        digitalWrite(pzem2,LOW);
        digitalWrite(pzem3,LOW);
        pzemRead();
        flash_led(2);
        v1=vo;a1=am;f1=fr;pf1=pof;e1=en;
        Serial.print("V1:");Serial.println(v1);
        lcd.clear();
        txt1 = "A1: "+String(a1)+"  ";
        txt1 = txt1.substring(0,txt1.length());
        txt2 = "V1: "+String(v1)+"  ";;
        txt2 = txt2.substring(0,txt2.length());
        txt3 = "pf1: "+String(pf1)+"  ";;
        txt3 = txt3.substring(0,txt3.length());
        lcd_displayTXT(1);
        loop_count++;
        break;
      case 2:
        Serial.println("Read pzem #2");
        digitalWrite(pzem1,LOW);
        digitalWrite(pzem2,HIGH);
        digitalWrite(pzem3,LOW);
        pzemRead();
        flash_led(3);
        v2=vo;a2=am;f2=fr;pf2=pof;e2=en;
        Serial.print("V2:");Serial.println(v2);
        lcd.clear();
        txt1 = "A2: "+String(a2)+"  ";
        txt1 = txt1.substring(0,txt1.length());
        txt2 = "V2: "+String(v2)+"  ";;
        txt2 = txt2.substring(0,txt2.length());
        txt3 = "pf2: "+String(pf2)+"  ";;
        txt3 = txt3.substring(0,txt3.length());
        lcd_displayTXT(1);
        loop_count++;
        break;
      case 3:
        Serial.println("Read pzem #3");
        digitalWrite(pzem1,LOW);
        digitalWrite(pzem2,LOW);
        digitalWrite(pzem3,HIGH);
        pzemRead();
        flash_led(4);
        v3=vo;a3=am;f3=fr;pf3=pof;e3=en;
        Serial.print("V3:");Serial.println(v3);
        lcd.clear();
        txt1 = "A3: "+String(a3)+"  ";
        txt1 = txt1.substring(0,txt1.length());
        txt2 = "V3: "+String(v3)+"  ";;
        txt2 = txt2.substring(0,txt2.length());
        txt3 = "pf3: "+String(pf3)+"  ";;
        txt3 = txt3.substring(0,txt3.length());
        lcd_displayTXT(1);
        loop_count++;
        break;
      case 10:
        loop_count=1;
        break;
      default:
        Serial.print(".......... case:");Serial.println(oneSec);
        digitalWrite(led,!digitalRead(led));
        loop_count++;
        break;
    }
    Serial.print("Time Left:");
    Serial.println(time2server-twoSec);
    if(twoSec>=time2server){
      lcd.clear();
      txt1="Send to Cloud Server";
      txt2="********************";
      lcd_displayTXT(5);
      postData3E();
      flash_led(5);
      twoSec = twoSec-time2server;
    }
    timeClient.update();
    timetime();
    lcd_displayTXT(1);
    if(post_err==1)posterr_count += 1;
    else posterr_count=0;
    if(posterr_count>=5){  //5 times post to server failed -> re connect wifi
      lcd.setCursor(10,3);
      lcd.print("wifi-recon");
      wifi_Connect();
    }
    Serial.print("net Error# ");
    Serial.println(posterr_count);
    lcd.setCursor(11,3);
    lcd.print("netErr#");
    lcd.setCursor(19,3);
    lcd.print(posterr_count);
    if(lcd_loop<=29)lcd.backlight();
    else if(lcd_loop>=30)lcd.noBacklight();
    if(lcd_loop>=60)lcd_loop=0;
    lcd_loop++;
    delay(1000);
}

void flash_led(int b){
  for(int a=0;a<=b;a++){
    digitalWrite(led,!digitalRead(led));
    delay(100);
  }
}


//lcd.h
void lcd_displayTXT(int a){
  lcd.setCursor(0, 0);
  if(post_err==0)lcd.print("#");
  else lcd.print("?");
  lcd.print(time2server-twoSec);
  lcd.print(":");
  lcd.print(dayStamp);
  lcd.print(" ");
  lcd.setCursor(15, 0);
  lcd.print(timeStamp.substring(0,5));
  lcd.setCursor(0, 1);
  lcd.print("                ");
  if(a==1){
    lcd.setCursor(0, 1);
    lcd.print(txt1.substring(0,19));
    lcd.setCursor(0, 2);
    lcd.print(txt2.substring(0,19));
    lcd.setCursor(0, 3);
    lcd.print(txt3.substring(0,19));
  }
  else if(a==5){
    lcd.setCursor(0, 1);
    lcd.print(txt1);
    lcd.setCursor(0, 2);
    lcd.print(txt2);
    lcd.setCursor(0, 3);
    lcd.print(txt3);
  }
}

//wifi.h
//--------------------------------------------
// CLOCK Update routine to run something every second
//--------------------------------------------
void EverySecondCheck(void)
{
  oneSec++;twoSec++;
  if(oneSec>10)oneSec=1;
 }

void wifi_Connect(){
   WiFi.begin(ssid, pass);
  int wifi_time=0;
  while (WiFi.status() != WL_CONNECTED && wifi_time<=30) {
    delay(500);
    Serial.print(".");
    wifi_time++;
  }
  lcd.clear();
  if(WiFi.status() == WL_CONNECTED){
    txt1="***********";
    txt2="Wi-Fi Connected";
    txt3="=> to Loop Program";
    txt4="in 3 Seconds";
  }
  else {
    txt1="";
    txt2="Wi-Fi Not Connected";
    txt3="=> to Restart";
    txt4="in 5 Seconds";
  }
  Serial.println(WiFi.localIP());
  macStr = WiFi.macAddress();

  Serial.print("Current ESP32 IP:");
  Serial.println(WiFi.localIP());
  Serial.print("Gateway (router) IP:");
  Serial.println(WiFi.gatewayIP());
  Serial.print("Subnet Mask:" );
  Serial.println(WiFi.subnetMask());
  ipassign=WiFi.localIP().toString().c_str();
  gatewayStr=WiFi.gatewayIP().toString().c_str();
  subnetStr=WiFi.subnetMask().toString().c_str();
  txt1=macStr;
  txt2=ipassign;
  txt3=gatewayStr;
  txt4=subnetStr;
  lcd_displayTXT(5);
  delay(2000);
}

//pzem.h
void pzemRead(){
  vo = pzem.voltage();
    if(isnan(vo)){
        vo=0;fr=0;pof=0;en=0;
        Serial.println("Error reading voltage");
    } else {
        Serial.print("Voltage: "); Serial.print(vo); Serial.println("V.");
        fr = pzem.frequency();
        pof = pzem.pf();
        en = pzem.energy();
    }
    am = pzem.current();
    if(isnan(am)){
        am=0;
        Serial.println("Error reading current");
        
    } else {
        Serial.print("Current: "); Serial.print(am); Serial.println("A.");
    }
    Serial.println("*******");
}

//data.h
//temperature T1 & T2
void dataConcat() {
  String datat = "http://";
  if(mcid=="")mcid="MC-001";
  datat.concat(serverip);
  datat.concat("/");
  datat.concat(path);
  datat.concat("/");
  datat.concat(devid);
  datat.concat("/insert3E.php?");
  datat.concat("devid=");
  datat.concat(devid);
  datat.concat("&mcid=");
  datat.concat(mcid);
  datat.concat("&a1=");
  datat.concat(a1);
  datat.concat("&v1=");
  datat.concat(v1);
  datat.concat("&pf1=");
  datat.concat(pf1); //Tin - Tout
  datat.concat("&e1=");
  datat.concat(e1);

  datat.concat("&a2=");
  datat.concat(a2);
  datat.concat("&v2=");
  datat.concat(v2);
  datat.concat("&pf2=");
  datat.concat(pf2); //Tin - Tout
  datat.concat("&e2=");
  datat.concat(e2);

  datat.concat("&a3=");
  datat.concat(a3);
  datat.concat("&v3=");
  datat.concat(v3);
  datat.concat("&pf3=");
  datat.concat(pf3); //Tin - Tout
  datat.concat("&e3=");
  datat.concat(e3);

  Serial.println(datat);
  int len = datat.length()+1;
  datat.toCharArray(serverName,len);
  Serial.println(serverName);
}
//temperature PT100
void postData3E(){
      esp_task_wdt_reset();
      dataConcat();
      HTTPClient http;
      http.begin(client, serverName);
      String rxMsg="";
      // Specify content-type header
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      int httpResponseCode = http.POST(rxMsg);
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      if(httpResponseCode<0 || httpResponseCode == 404){
        txt3="Failed ???";
        post_err=1;
      }
      else {
        txt3="Success ***";
        post_err=0;
      }
      lcd_displayTXT(5);
      delay(1000);
      http.end();
}

//time.h
void timetime(){
  /*
  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }*/
  day_ = timeClient.getDay();
  String dayStr = "";
  if(day_==0)dayStr="Sun";
  else if(day_==1)dayStr="Mon.";
  else if(day_==2)dayStr="Tue.";
  else if(day_==3)dayStr="Wed.";
  else if(day_==4)dayStr="Thu.";
  else if(day_==5)dayStr="Fri.";
  else if(day_==6)dayStr="Sat.";
  if(day_ ==0 || day_ ==6)holiday_flag=1;
  else holiday_flag=0;
  Serial.print("holiday: ");
  Serial.println(holiday_flag);
  // The formattedDate comes with the following format:
  // 2018-05-28T16:00:13Z
  // We need to extract date and time
  formattedDate = timeClient.getFormattedDate();
  Serial.println(formattedDate);
    // Extract date
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  Serial.print("DATE: ");
  Serial.println(dayStamp);
  // Extract time
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
  Serial.print("HOUR: ");
  Serial.println(timeStamp);
  int hour_offp = timeStamp.substring(0,2).toInt();
  Serial.print("Hour: ");
  Serial.print(hour_offp);
  if(hour_offp <=6 || hour_offp >=22)hour_flag=1;
  else hour_flag=0;
  Serial.print(" flag: ");
  Serial.println(hour_flag);
}

//ble.h
struct EEPROMstorage {                   // Data storage in EEPROM to maintain them after power loss
  char SsidChr[30];
  char PassChr[40];
  char ServerChr[20];
  char PathChr[15];
  char DeviceChr[15];
  char Wifi[2];
  char McChr[15]; //machine
  char McAdd[20]; //machine
  char Time2serverChr[4];
  char BLEbroadcastName[30] ; 
  byte UseBLELongString = 0;  //UseBLELongString 
  int  Checksum         = 0;
}  Mem; 

char menu[][40] = 
 {
 "A SSID", 
 "B Pass",
 "C Server",
 "D Path",
 "E Device",
 "F WiFi",
 "G t2server",
 "I Print this menu",
 "N BLE beacon name",
 "M Machine name",
 "R Reset settings",
 "Z Reset MCU"
 };
//--------------------------------------------
// BLE   //#include <NimBLEDevice.h>
//--------------------------------------------
BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
bool deviceConnected    = false;
bool oldDeviceConnected = false;
std::string ReceivedMessageBLE;

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"        // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
char sptext[200];   

void SendMessageBLE(std::string Message)
{
 if(deviceConnected) 
   {
    if (Mem.UseBLELongString)                                                     // If Fast transmission is possible
     {
      pTxCharacteristic->setValue(Message); 
      pTxCharacteristic->notify();
      delay(10);                                                                              // Bluetooth stack will go into congestion, if too many packets are sent
     } 
   else                                                                                      // Packets of max 20 bytes
     {   
      int parts = (Message.length()/20) + 1;
      for(int n=0;n<parts;n++)
        {   
         pTxCharacteristic->setValue(Message.substr(n*20, 20)); 
         pTxCharacteristic->notify();
         delay(40);                                                                           // Bluetooth stack will go into congestion, if too many packets are sent
        }
     }
   } 
}

//--------------------------------------------
// Common common print routines
//--------------------------------------------
void Tekstprint(char const *tekst)    { if(Serial) Serial.print(tekst);  SendMessageBLE(tekst);sptext[0]=0;   } 
void Tekstprintln(char const *tekst)  { sprintf(sptext,"%s\n",tekst); Tekstprint(sptext);  }
void TekstSprint(char const *tekst)   { printf(tekst); sptext[0]=0;}                          // printing for Debugging purposes in serial monitor 
void TekstSprintln(char const *tekst) { sprintf(sptext,"%s\n",tekst); TekstSprint(sptext); }


void GetStructFromFlashMemory(void)
{
 FLASHSTOR.begin("Mem", false);
 FLASHSTOR.getBytes("Mem", &Mem, sizeof(Mem) );
 FLASHSTOR.end();       
 sprintf(sptext,"Mem.Checksum = %d",Mem.Checksum);Tekstprintln(sptext);   
 ssid =String(Mem.SsidChr);//ssid
 pass =String(Mem.PassChr);//pass
 serverip =String(Mem.ServerChr);//pass
 path =String(Mem.PathChr);//pass
 devid =String(Mem.DeviceChr);//pass
 mcid =String(Mem.McChr);//pass
 bleNameStr = String(Mem.BLEbroadcastName);//pass
 String time2serverStr =String(Mem.Time2serverChr);//pass
 time2server = time2serverStr.toInt();
 if(time2server<=30)time2server=60;

 String Wifi_str = String(Mem.Wifi);
 Serial.print("ssid:");Serial.println(ssid);
 Serial.print("pass:");Serial.println(pass);
 Serial.print("server:");Serial.println(serverip);
 Serial.print("path:");Serial.println(path);
 Serial.print("devid:");Serial.println(devid);
 Serial.print("mcid:");Serial.println(mcid);
 Serial.print("time2server:");Serial.println(time2server);
 Serial.print("bmeName:");Serial.println(bleNameStr);
}
void StoreStructInFlashMemory(void)
{
 FLASHSTOR.begin("Mem",false);       //  delay(100);
 FLASHSTOR.putBytes("Mem", &Mem , sizeof(Mem) );
 FLASHSTOR.end();          
 }
 void PrintLine(byte Lengte)
{
 for(int n=0; n<Lengte; n++) sptext[n]='_';
 sptext[Lengte] = 0;
 Tekstprintln(sptext);
}
void SWversion(void) 
{ 
 #define FILENAAM (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
 PrintLine(35);
 for (uint8_t i = 0; i < sizeof(menu) / sizeof(menu[0]); Tekstprintln(menu[i++]));
 PrintLine(35);
 sprintf(sptext,"with ending character= ';' ");Tekstprintln(sptext);
 sprintf(sptext,"ex: ATP-Link_5B9A; ");Tekstprintln(sptext);
 sprintf(sptext,"--------------------");Tekstprintln(sptext);
 sprintf(sptext,"mcAddress: %s", Mem.McAdd);Tekstprintln(sptext);
 sprintf(sptext,"A SSID: %s", Mem.SsidChr);Tekstprintln(sptext);
 sprintf(sptext,"B Pass: %s", Mem.PassChr);Tekstprintln(sptext);
 sprintf(sptext,"C Server IP: %s", Mem.ServerChr);Tekstprintln(sptext);
 sprintf(sptext,"D Path: %s", Mem.PathChr);Tekstprintln(sptext);
 sprintf(sptext,"E Device: %s", Mem.DeviceChr);Tekstprintln(sptext);
 sprintf(sptext,"F Wifi(0,1): %s", Mem.Wifi);Tekstprintln(sptext);
 sprintf(sptext,"G t2server: %s", Mem.Time2serverChr);Tekstprintln(sptext);
 sprintf(sptext,"I Print Info");Tekstprintln(sptext);
 sprintf(sptext,"N BLE name: %s", Mem.BLEbroadcastName);Tekstprintln(sptext);
 sprintf(sptext,"M Machine ID: %s" , Mem.McChr);Tekstprintln(sptext);
 sprintf(sptext,"r: Reset Ble", Mem.BLEbroadcastName);Tekstprintln(sptext);
 sprintf(sptext,"z: Restart MCU", Mem.BLEbroadcastName);Tekstprintln(sptext);
 sprintf(sptext,"%s",Mem.UseBLELongString? "FastBLE=On":"FastBLE=Off" );Tekstprintln(sptext);
 sprintf(sptext,"Software: %s",FILENAAM);Tekstprintln(sptext);  //VERSION); 
 PrintLine(35);
}
void Reset(void)
{
 Mem.Checksum         = 25065;                                                                 //
 Mem.UseBLELongString = 0;
 strcpy(Mem.SsidChr,"TP-Link_5B9A");                                                                         // Default SSID
 strcpy(Mem.PassChr,"97180937");                                                                     // Default password
 strcpy(Mem.ServerChr,"137.184.86.182");
 strcpy(Mem.PathChr,"iot2025");
 strcpy(Mem.DeviceChr,"e001");
 strcpy(Mem.Time2serverChr,"300");
 strcpy(Mem.Wifi,"1");
 strcpy(Mem.McChr,"Mc-001");
 strcpy(Mem.BLEbroadcastName,"E3001-Ble");
 Tekstprintln("**** Reset of preferences ****"); 
 StoreStructInFlashMemory();                                                                  // Update Mem struct       
 SWversion();                                                                                 // Display the version number of the software
}
void InitStorage(void)
{
 GetStructFromFlashMemory();
 if( Mem.Checksum != 25065)
   {
    sprintf(sptext,"Checksum (25065) invalid: %d\n Resetting to default values",Mem.Checksum); 
    Tekstprintln(sptext); 
    Reset();                // If the checksum is NOK the Settings were not set
   }
 StoreStructInFlashMemory();
}



void ReworkInputString(String InputString)
{
 char ff[50];  InputString.toCharArray(ff,InputString.length());                           // Convert a String to char array
 sprintf(sptext,"Inputstring: %s  Lengte : %d\n", ff,InputString.length()-1); 
 // Tekstprint(sptext);
 if(InputString.length()> 40){Serial.printf("Input string too long (max40)\n"); return;}                                                         // If garbage return
 sptext[0] = 0;                                                                               // Empty the sptext string
 if(InputString[0] > 31 && InputString[0] <127)                                               // Does the string start with a letter?
  { 
  switch (InputString[0])
   { 
    case 'A': //SSID
    case 'a': 
           if (InputString.length() >5 )
            {  
             int ll = InputString.indexOf(";");
             if(ll==0)break;
             InputString.substring(1).toCharArray(Mem.SsidChr,ll);
             sprintf(sptext,"mySSID: %s", Mem.SsidChr); 
            }
            else sprintf(sptext,"**** Length fault. Use between 4 and 30 characters ****");
            break;
    case 'B': //Pass
    case 'b': 
           if (InputString.length() >5 )
            {  
             int ll = InputString.indexOf(";");
             if(ll==0)break;
             InputString.substring(1).toCharArray(Mem.PassChr,ll);
             sprintf(sptext,"Pass set: %s", Mem.PassChr); 
            }
            else sprintf(sptext,"**** Length fault. Use between 4 and 30 characters ****");
            break;
    case 'C': //Server
    case 'c': 
           if (InputString.length() >5 )
            {  
             int ll = InputString.indexOf(";");
             if(ll==0)break;
             InputString.substring(1).toCharArray(Mem.ServerChr,ll);
             sprintf(sptext,"Server IP set: %s", Mem.ServerChr); 
            }
            else sprintf(sptext,"**** Length fault. Use between 4 and 30 characters ****");
            break;

    case 'D': //Path
    case 'd': 
           if (InputString.length() >5 )
            {  
             int ll = InputString.indexOf(";");
             if(ll==0)break;
             InputString.substring(1).toCharArray(Mem.PathChr,ll);
             sprintf(sptext,"Path set: %s", Mem.PathChr); 
             devid = String(Mem.PathChr);
             Serial.print("path:");Serial.println(devid);
            }
            else sprintf(sptext,"**** Length fault. Use between 4 and 30 characters ****");
            break;
    case 'E': //Device
    case 'e': 
           if (InputString.length() >5 )
            {  
             int ll = InputString.indexOf(";");
             if(ll==0)break;
             InputString.substring(1).toCharArray(Mem.DeviceChr,ll);
             sprintf(sptext,"Device: %s", Mem.DeviceChr); 
             mcid = String(Mem.DeviceChr);
            }
            else sprintf(sptext,"**** Length fault. Use between 4 and 30 characters ****");
            break;
    case 'F': //Wifi
    case 'f': 
           if (InputString.length() >0 )
            {  
             int ll = InputString.indexOf(";");
             if(ll==0)break;
             InputString.substring(1).toCharArray(Mem.Wifi,ll);
             sprintf(sptext,"WiFi set (0,1): %s", Mem.Wifi); 
            }
            else sprintf(sptext,"**** Length fault. Use between 4 and 30 characters ****");
            break;

    case 'G': //t2server
    case 'g': 
           if (InputString.length() >0 )
            {  
             int ll = InputString.indexOf(";");
             if(ll==0)break;
             InputString.substring(1).toCharArray(Mem.Time2serverChr,ll);
             sprintf(sptext,"t2server set: %s", Mem.Time2serverChr);
             time2server = String(Mem.Time2serverChr).toInt();
            }
            else sprintf(sptext,"**** Length fault. Use between 4 and 30 characters ****");
            break;

    case 'N': //Ble Name
    case 'n': 
           if (InputString.length() >5 )
            { 
             int ll = InputString.indexOf(";"); 
             if(ll==0)break;
             InputString.substring(1).toCharArray(Mem.BLEbroadcastName,ll);
             sprintf(sptext,"BLE broadcast name set: %s", Mem.BLEbroadcastName); 
            }
            else sprintf(sptext,"**** Length fault. Use between 4 and 30 characters ****");
            break;

    case 'M': //Machine Name
    case 'm': 
           if (InputString.length() >5 )
            { 
             int ll = InputString.indexOf(";"); 
             if(ll==0)break;
             InputString.substring(1).toCharArray(Mem.McChr,ll);
             sprintf(sptext,"Machine name set: %s", Mem.McChr); 
             mcid = String(Mem.McChr);
             Serial.print("mcid:");
             Serial.println(mcid);
            }
            else sprintf(sptext,"**** Length fault. Use between 4 and 30 characters ****");
            break;
    case 'I':
    case 'i': 
            SWversion();
            break;
    case 'R': //default value
    case 'r':
            Reset();
            sprintf(sptext,"\nReset to default values: Done");                              
             break;      
    case 'Z': //reset mcu
    case 'z':
            sprintf(sptext,"\nReset MCU: Done");
            delay(1000);
            ESP.restart();
            break;  

    default: break;
    }
  }  
 Tekstprintln(sptext);    
 StoreStructInFlashMemory();                         
 InputString = "";
}
// BLE Start BLE Classes
//------------------------------
class MyServerCallbacks: public BLEServerCallbacks 
{
 void onConnect(BLEServer* pServer) {deviceConnected = true; };
 void onDisconnect(BLEServer* pServer) {deviceConnected = false;}
};

class MyCallbacks: public BLECharacteristicCallbacks 
{
 void onWrite(BLECharacteristic *pCharacteristic) 
  {
   std::string rxValue = pCharacteristic->getValue();
   ReceivedMessageBLE = rxValue + "\n";
//   if (rxValue.length() > 0) {for (int i = 0; i < rxValue.length(); i++) printf("%c",rxValue[i]); }
//   printf("\n");
  }  
};

//-----------------------------
// BLE Start BLE Service
//------------------------------
void StartBLEService(void){

 BLEDevice::init(Mem.BLEbroadcastName);                       // Create the BLE Device
 pServer = BLEDevice::createServer();                         // Create the BLE Server
 pServer->setCallbacks(new MyServerCallbacks());
 BLEService *pService = pServer->createService(SERVICE_UUID);     // Create the BLE Service
 pTxCharacteristic                     =                          // Create a BLE Characteristic 
     pService->createCharacteristic(CHARACTERISTIC_UUID_TX, NIMBLE_PROPERTY::NOTIFY);                 
 BLECharacteristic * pRxCharacteristic = 
     pService->createCharacteristic(CHARACTERISTIC_UUID_RX, NIMBLE_PROPERTY::WRITE);
 pRxCharacteristic->setCallbacks(new MyCallbacks());
 pService->start(); 
 BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
 pAdvertising->addServiceUUID(SERVICE_UUID); 
 pServer->start();                                    // Start the server  
 pServer->getAdvertising()->start();                  // Start advertising
 TekstSprint("BLE Waiting a client connection to notify ...\n"); 
}
void passByte2(String aStr,byte aArray[4]){
    int a1 = aStr.indexOf(".");
    aArray[0] = aStr.substring(0,a1).toInt();
    String bStr = aStr.substring(a1+1);

    int a2 = bStr.indexOf(".");
    aArray[1] = bStr.substring(0,a2+1).toInt();
    String cStr = bStr.substring(a2+1);

    int a3 = cStr.indexOf(".");
    aArray[2] = cStr.substring(0,a3+1).toInt();

    aArray[3] = cStr.substring(a3+1).toInt();

    Serial.println(aStr);
    for(int x=0;x<4;x++){
      Serial.print(aArray[x]);
      Serial.print(",");
    }
    Serial.println();
}
void init_read(){
  String deviceStr = String(Mem.DeviceChr);
  int lenMem = deviceStr.indexOf(";");
  devid = deviceStr.substring(0,lenMem);
  Serial.print("New Device: ");
  Serial.println(devid); 
 
  String pathStr = String(Mem.PathChr);
  lenMem = pathStr.indexOf(";");
  path = pathStr.substring(0,lenMem);
  Serial.print("New Path: ");
  Serial.println(path); 

  String serverStr = String(Mem.ServerChr);
  lenMem = serverStr.indexOf(";");
  serverip = serverStr.substring(0,lenMem);
  Serial.print("New Server IP: ");
  Serial.println(serverip); 

  String bleStr = String(Mem.BLEbroadcastName);
  lenMem = bleStr.indexOf(";");
  String bleString = bleStr.substring(0,lenMem);
  Serial.print("BleName: ");
  Serial.println(bleString); 
  if(bleString=""){
    bleNameStr.toCharArray(Mem.BLEbroadcastName,bleNameStr.length()+1);
  }
  ssid = String(Mem.SsidChr);
  pass= String(Mem.PassChr);

  String wifiStr = String(Mem.Wifi);

  String t2serverStr = String(Mem.Time2serverChr);
  time2server = t2serverStr.toInt(); 
  Serial.print("t2server: ");
  Serial.println(time2server);  
}
void CheckBLE(void)
{
 if(!deviceConnected && oldDeviceConnected)                                                   // Disconnecting
   {
    delay(300);                                                                               // Give the bluetooth stack the chance to get things ready
    pServer->startAdvertising();                                                              // Restart advertising
    TekstSprint("Start advertising\n");
    oldDeviceConnected = deviceConnected;
   }
 if(deviceConnected && !oldDeviceConnected)                                                   // Connecting
   { 
    oldDeviceConnected = deviceConnected;
    SWversion();
   }
 if(ReceivedMessageBLE.length()>0)
   {
    SendMessageBLE(ReceivedMessageBLE);
    String BLEtext = ReceivedMessageBLE.c_str();
    Serial.print("BL MSG:");
    Serial.println(BLEtext);
    ReceivedMessageBLE = "";
    ReworkInputString(BLEtext); 
   }
}

//voids.h
//--------------------------------------------
// Common check for serial input
//--------------------------------------------
void SerialCheck(void)
{
  if(Serial.available()>0){
    String SerialString =""; 
    while (Serial.available())
      { 
      char c = Serial.read();                                                               // Serial.write(c);
      if (c>31 && c<128) SerialString += c;                                                    // Allow input from Space - Del
      else c = 0;                                                                              // Delete a CR
      }
    if(SerialString.indexOf("t2server=")>0){
      String timetxt = SerialString.substring(10);
      time2server = timetxt.toInt();
      Serial.print("Chang t2server to: ");
      Serial.println(time2server);
    }
    if(SerialString.indexOf("serialno=")>0){
      String serialtxt = SerialString.substring(10);
      int lenEnd = serialtxt.indexOf(";");
      serialtxt.toCharArray(Mem.BLEbroadcastName,lenEnd+1);
      Serial.print("Change ble to serial no: ");
      Serial.println(serialtxt);
    }
    else if(SerialString.indexOf("reset")>0){
      ESP.restart();
    }
  }
}
