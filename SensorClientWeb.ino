#include <FS.h> 
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>

#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>
#include <IRutils.h>



//Send Pins
const uint16_t kIrLed = 4;  // ESP8266 GPIO pin to use. Recommended: 4 (D2).
IRsend irsend(kIrLed);  // Set the GPIO to be used to sending the message.

//Receive Pins
const uint16_t kRecvPin = 14;// ESP8266 GPIO pin to use. Recommended: 14 (D5).
IRrecv irrecv(kRecvPin);
decode_results results;

#include "WemoSwitch.h"
#include "WemoManager.h"
#include "CallbackFunction.h"

WemoManager wemoManager;
WemoSwitch *IR1 = NULL;
WemoSwitch *IR2 = NULL;
WemoSwitch *IR3 = NULL;
WemoSwitch *IR4 = NULL;

int FlashButtonState=-1;
int ByPassSetupMode=0;
int IsInSetupMode=0;
int IsInOperationalMode=0;
int SetupOperationalMode=1;

ESP8266WebServer server(80);

String LocalWifiSSID="";
String LocalWifiPass="";
//Sensor Names
String SensorName1;
String SensorName2;
String SensorName3;
String SensorName4;
//IR Values
String IRVal1;
String IRVal2;
String IRVal3;
String IRVal4;

String LastIRCode="000000001";


void setup() 
{
  
  
  pinMode(0, INPUT);//Setup Flash Button GPIO_0
  pinMode(2, OUTPUT);//Setup LED Button GPIO_2
  //RGB LEDS
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(15, OUTPUT);  
  
  
  digitalWrite(2, LOW);
  
  Serial.begin(115200);
  Serial.println("Booting Up...");
  delay(10);


}

void loop() 
{
  //Check if this is the first time startup
  if(ByPassSetupMode==0){WaitForSetupMode();ByPassSetupMode=1;digitalWrite(2, HIGH);}
  
  if(IsInSetupMode == 1)
  {
    server.handleClient();
    if (irrecv.decode(&results)) {
      unsigned long NewLastIRCode = results.value;
      LastIRCode = String(NewLastIRCode,HEX);
      //Write last received code to disk
      WriteFile("lastircode",LastIRCode);
      FlashLEDOnOff();
      Serial.println(LastIRCode);
      irrecv.resume();  // Receive the next value
    }    
   
   }

  //Check if Operational Mode may start
  if(SetupOperationalMode==1 && IsInSetupMode==0 ){StartOperationalMode();}
  
  //Debug
    //Serial.print("SetupOperationalMode:");Serial.println(SetupOperationalMode);  
    //Serial.print("IsInSetupMode:");Serial.println(IsInSetupMode);
  
  //Run Operation Code
  if(IsInOperationalMode==1)
  {
    //Handle WemoRequests
    wemoManager.serverLoop();
   
    
  }
  
}

void WaitForSetupMode()
{
  
    for (int i = 0; i <= 300; i++) {
      FlashButtonState =  digitalRead(0);
      if(FlashButtonState==0){StartSetupMode();break;}
      delay(10);
    }
     //WriteDefaultIR code to file.
     WriteFile("lastircode","00000000");

  
  
}

void StartSetupMode()
{
const char* ssid     = "LittleNodes_WemoEmulator";
const char* password = "";

//Flash LED to indicate setup mode
    for (int i = 0; i <= 10; i++) {
      digitalWrite(2, LOW);
      delay(50);
      digitalWrite(2, HIGH);
      delay(50);
    }
    digitalWrite(2, LOW);

    

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();

  Serial.println("");
  Serial.println("Starting Setup Mode...");
  Serial.println("WiFi started successfully");
  
  FlashLEDSlow();  
  Serial.print("IP address: ");
  Serial.println(myIP);
  //Indicate node is in setup mode in order to handle web requests.
  IsInSetupMode = 1; 

  //Start the web server
  server.begin(); 
  server.on("/", fileindex);
  server.on("/index.html", fileindex);
  server.on("/setup.html", filesetup);
  server.on("/savesetup.html", savesetup);
  server.on("/reboot.html", reboot);
  server.on("/bootstrap.min.css", bootstrap);
  server.on("/fSensorName1.conf", fSensorName1);
  server.on("/fSensorName2.conf", fSensorName2);
  server.on("/fSensorName3.conf", fSensorName3);
  server.on("/fSensorName4.conf", fSensorName4);
  server.on("/fSensorIR1.conf", fSensorIR1);
  server.on("/fSensorIR2.conf", fSensorIR2);
  server.on("/fSensorIR3.conf", fSensorIR3);
  server.on("/fSensorIR4.conf", fSensorIR4);
  server.on("/lastircode", lastircode);

  

  
  server.on("/fSSID.conf", fSSID);
  server.on("/fPassword.conf", fPassword);


  
  //Start the filesystem
  SPIFFS.begin(); 
  //Start IR Receiver
  irrecv.enableIRIn();  // Start the receiver
  Serial.print("IRrecvDemo is now running and waiting for IR message on Pin ");
  Serial.println(kRecvPin);

}



//File Handlers
void fileindex()
{
  Serial.println("loading index.html");
  File file = SPIFFS.open("/index.html.gz", "r"); 
  size_t sent = server.streamFile(file, "text/html");
}
void filesetup()
{
  Serial.println("loading setup.html");
  File file = SPIFFS.open("/setup.html.gz", "r"); 
  size_t sent = server.streamFile(file, "text/html");
}
void savesetup()
{
  File file = SPIFFS.open("/savesetup.html.gz", "r"); 
  size_t sent = server.streamFile(file, "text/html");

  //Get data from incomming POST request
  String fSensorName1;
  String fSensorName2;
  String fSensorName3;
  String fSensorName4;
  //IR codes
  String fSensorIR1;
  String fSensorIR2;
  String fSensorIR3;
  String fSensorIR4;


  
  String fSSID;
  String fPassword;
  if(server.hasArg("fSensorName1"))
  {
    Serial.println("Saving the following settings...");
    //Device Names
    fSensorName1 = server.arg("fSensorName1"); Serial.println("Sensor Name1: " + fSensorName1);
    fSensorName2 = server.arg("fSensorName2"); Serial.println("Sensor Name2: " + fSensorName2);
    fSensorName3 = server.arg("fSensorName3"); Serial.println("Sensor Name3: " + fSensorName3);
    fSensorName4 = server.arg("fSensorName4"); Serial.println("Sensor Name4: " + fSensorName4);
    //Device IR Values
    fSensorIR1 = server.arg("fSensorIR1"); Serial.println("Sensor Val IR1: " + fSensorIR1);
    fSensorIR2 = server.arg("fSensorIR2"); Serial.println("Sensor Val IR2: " + fSensorIR2);
    fSensorIR3 = server.arg("fSensorIR3"); Serial.println("Sensor Val IR3: " + fSensorIR3);
    fSensorIR4 = server.arg("fSensorIR4"); Serial.println("Sensor Val IR4: " + fSensorIR4);
    

    
    fSSID = server.arg("fSSID"); Serial.println("AP SSID: " + fSSID);
    fPassword = server.arg("fPassword"); Serial.println("AP Password: " + fPassword);


    //Write settings to file    
    //Device Names
    WriteFile("fSensorName1.conf",fSensorName1);
    WriteFile("fSensorName2.conf",fSensorName2);
    WriteFile("fSensorName3.conf",fSensorName3);
    WriteFile("fSensorName4.conf",fSensorName4);
    //IR Values
    WriteFile("fSensorIR1.conf",fSensorIR1);
    WriteFile("fSensorIR2.conf",fSensorIR2);
    WriteFile("fSensorIR3.conf",fSensorIR3);
    WriteFile("fSensorIR4.conf",fSensorIR4);
    
    WriteFile("fSSID.conf",fSSID);
    WriteFile("fPassword.conf",fPassword);
    Serial.println("Data written to local file system");
    
   
  }
}
void WriteFile(String FileName,String DataToWrite)
{
  File FileObjW = SPIFFS.open("/" + FileName, "w");
  DataToWrite.trim();
  FileObjW.print(DataToWrite);
  FileObjW.close(); 
}
void bootstrap()
{
  File file = SPIFFS.open("/bootstrap.min.css.gz", "r"); 
  size_t sent = server.streamFile(file, "text/css");
}
void fSensorName1()
{
  File file = SPIFFS.open("/fSensorName1.conf", "r"); 
  size_t sent = server.streamFile(file, "text/html");
}
void fSensorName2()
{
  File file = SPIFFS.open("/fSensorName2.conf", "r"); 
  size_t sent = server.streamFile(file, "text/html");
}
void fSensorName3()
{
  File file = SPIFFS.open("/fSensorName3.conf", "r"); 
  size_t sent = server.streamFile(file, "text/html");
}
void fSensorName4()
{
  File file = SPIFFS.open("/fSensorName4.conf", "r"); 
  size_t sent = server.streamFile(file, "text/html");
}
void fSensorIR1()
{
  File file = SPIFFS.open("/fSensorIR1.conf", "r"); 
  size_t sent = server.streamFile(file, "text/html");
}
void fSensorIR2()
{
  File file = SPIFFS.open("/fSensorIR2.conf", "r"); 
  size_t sent = server.streamFile(file, "text/html");
}
void fSensorIR3()
{
  File file = SPIFFS.open("/fSensorIR3.conf", "r"); 
  size_t sent = server.streamFile(file, "text/html");
}
void fSensorIR4()
{
  File file = SPIFFS.open("/fSensorIR4.conf", "r"); 
  size_t sent = server.streamFile(file, "text/html");
}
void lastircode()
{
 
  File file = SPIFFS.open("/lastircode", "r"); 
  size_t sent = server.streamFile(file, "text/html");
}


void fSSID()
{
  File file = SPIFFS.open("/fSSID.conf", "r"); 
  size_t sent = server.streamFile(file, "text/html");
}
void fPassword()
{
  File file = SPIFFS.open("/fPassword.conf", "r"); 
  size_t sent = server.streamFile(file, "text/html");
}

void reboot()
{
  //Restart the device
  ESP.restart();
}



void StartOperationalMode()
{
  SPIFFS.begin(); 
  SetupOperationalMode=0;
  Serial.println("Starting Operational Mode");
  Serial.println("Reading Configuration from filesystem...");

  //Load Config
  ReadConfig("fSSID.conf");
  ReadConfig("fPassword.conf");
  //Load Device Names
  ReadConfig("fSensorName1.conf");
  ReadConfig("fSensorName2.conf");
  ReadConfig("fSensorName3.conf");
  ReadConfig("fSensorName4.conf");
  //Load IR Values
  ReadConfig("fSensorIR1.conf");
  ReadConfig("fSensorIR2.conf");
  ReadConfig("fSensorIR3.conf");
  ReadConfig("fSensorIR4.conf");


  
  Serial.println("Local Wifi SSID:" + LocalWifiSSID);
  Serial.println("Local Wifi PASS:" + LocalWifiPass);
  Serial.println("SensorName1:" + SensorName1 + ":" + IRVal1 );
  Serial.println("SensorName2:" + SensorName2 + ":" + IRVal2 );
  Serial.println("SensorName3:" + SensorName3 + ":" + IRVal3 );
  Serial.println("SensorName4:" + SensorName4 + ":" + IRVal4 );

  

  //Start Wifi
  Serial.println("Setting Up Wifi");
  LocalWifiSSID.trim();
  LocalWifiPass.trim();
  WiFi.mode(WIFI_STA);
  WiFi.begin(LocalWifiSSID.c_str(), LocalWifiPass.c_str());
  Serial.println("Connecting to " + LocalWifiSSID);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("WiFi connected - ");  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Adding Wemo Virtual Devices ...");
  
 
   

  //Start Wemo Emulator and add virtual devices.
  wemoManager.begin();
  // Format: Alexa invocation name, local port no, on callback, off callback
  //IR1 = new WemoSwitch(SensorName1, 80, OnVoid, Off Void)
  IR1 = new WemoSwitch(SensorName1, 80, SendIR1, SendIR1);
  wemoManager.addDevice(*IR1);
  Serial.println("Adding:" + SensorName1);
  
  IR2 = new WemoSwitch(SensorName2, 81, SendIR2, SendIR2);
  wemoManager.addDevice(*IR2);
  Serial.println("Adding:" + SensorName2);
  
  IR3 = new WemoSwitch(SensorName3, 82, SendIR3, SendIR3);  
  wemoManager.addDevice(*IR3);
  Serial.println("Adding:" + SensorName3);
  
  IR4 = new WemoSwitch(SensorName4, 83, SendIR4, SendIR4);  
  wemoManager.addDevice(*IR4);
  Serial.println("Adding:" + SensorName4);
  


  Serial.println("Starting the Infrared routines...");
  irsend.begin();
 

  
  
  Serial.println("Now running in operational mode...");
  IsInOperationalMode=1;
  Serial.println("Flashing LED slowly to indicate that this node is now online");
  FlashLEDSlow();
}

void ReadConfig(String FileNameToRead)
{      
      Serial.println("Reading:" + FileNameToRead);
      String FileContent="";      
      FileNameToRead.trim();      
      File FileObjR = SPIFFS.open("/" + FileNameToRead , "r");
      if (!FileObjR) {
          Serial.println("/" + FileNameToRead + " File not found.");
      }      
      while (FileObjR.available()) {
          FileContent=FileContent += char(FileObjR.read());      
      } 
      FileObjR.close(); 
      if(FileNameToRead=="fSSID.conf"){LocalWifiSSID=FileContent;}
      if(FileNameToRead=="fPassword.conf"){LocalWifiPass=FileContent;}
      if(FileNameToRead=="fSensorName1.conf"){SensorName1=FileContent;}
      if(FileNameToRead=="fSensorName2.conf"){SensorName2=FileContent;}
      if(FileNameToRead=="fSensorName3.conf"){SensorName3=FileContent;}
      if(FileNameToRead=="fSensorName4.conf"){SensorName4=FileContent;}

      if(FileNameToRead=="fSensorIR1.conf"){IRVal1=FileContent;}
      if(FileNameToRead=="fSensorIR2.conf"){IRVal2=FileContent;}
      if(FileNameToRead=="fSensorIR3.conf"){IRVal3=FileContent;}
      if(FileNameToRead=="fSensorIR4.conf"){IRVal4=FileContent;}

      

}

void SendIR1(){
 
 
 unsigned long NewIrVal1 = strtoul(IRVal1.c_str(), NULL, 16);
  for (int i = 0; i <= 2; i++) {
    irsend.sendNEC(NewIrVal1);
    Serial.println("Sending:" + SensorName1 + ":" + NewIrVal1);
    delay(200);
  }  
 
 
 irsend.sendNEC(NewIrVal1);
 LEDDance();

}
void SendIR2(){
 unsigned long NewIrVal2 = strtoul(IRVal2.c_str(), NULL, 16);
  for (int i = 0; i <= 2; i++) {
    irsend.sendNEC(NewIrVal2);
    Serial.println("Sending:" + SensorName2 + ":" + NewIrVal2);
    delay(200);
  }
 LEDDance();

}
void SendIR3(){
 unsigned long NewIrVal3 = strtoul(IRVal3.c_str(), NULL, 16);
  for (int i = 0; i <= 2; i++) {
    irsend.sendNEC(NewIrVal3);
    Serial.println("Sending:" + SensorName3 + ":" + NewIrVal3);
    delay(200);
  }
 LEDDance();

}
void SendIR4(){
 unsigned long NewIrVal4 = strtoul(IRVal4.c_str(), NULL, 16);
  for (int i = 0; i <= 2; i++) {
    irsend.sendNEC(NewIrVal4);
    Serial.println("Sending:" + SensorName4 + ":" + NewIrVal4);
    delay(200);
  }
 LEDDance();

}
void SendNothing(){
  Serial.println("Sending:Nothing");
  LEDDance();
}

void FlashLED()
{

  for (int i = 0; i <= 3; i++) {
  digitalWrite(2,HIGH);
  delay(50);
  digitalWrite(2, LOW);
  delay(50);
  digitalWrite(2,HIGH);
  
  }  
  
 
}
void FlashLEDSlow()
{

  for (int i = 0; i <= 2; i++) {
  digitalWrite(2,HIGH);
  delay(200);
  digitalWrite(2, LOW);
  delay(200);
  digitalWrite(2,HIGH);
  
  }  
}

void FlashLEDOnOff()
{
  digitalWrite(2, LOW);
  delay(100);
  digitalWrite(2,HIGH);
}

void LEDDance()
{
  int randomPin = random(12, 15);
  digitalWrite(12, HIGH);
  delay(40);
  digitalWrite(12, LOW);
  delay(40);
  digitalWrite(13, HIGH);
  delay(40);
  digitalWrite(13, LOW);
  delay(40);  
    digitalWrite(15, HIGH);
  delay(40);
  digitalWrite(15, LOW);
  delay(40);
  
 
}
