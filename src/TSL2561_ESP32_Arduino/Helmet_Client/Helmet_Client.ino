#include <Wire.h>
#include "BLEDevice.h"


#define bleServerName "Motorcycle_server"
static BLEUUID MotorcycleServiceUUID("7107eb72-77e7-411b-8dcb-af365300a59c");

// BLE Characteristics
static BLEUUID BreakLightCharacteristicsUUID("46dfbfba-d66f-42dc-8128-1b636a527768");
static BLEUUID RightLightCharacteristicsUUID("f85e8339-026a-4e26-9dc4-cfecaa207113");
static BLEUUID LeftLightCharacteristicsUUID("2ad68c57-8b7f-48fa-ab97-d3ca82b4d6f1");

//Address of the peripheral device. Address will be found during scanning...
static boolean doConnect = false;
static boolean connected = false;

//Address of the peripheral device. Address will be found during scanning...
static BLEAddress *pServerAddress;
 
//Characteristicd that we want to read
static BLERemoteCharacteristic* BreakLightCharacteristic;
static BLERemoteCharacteristic* RightLightCharacteristic;
static BLERemoteCharacteristic* LeftLightCharacteristic;

//Variables to store lux values
char* Break_Char;
char* Right_Char;
char* Left_Char;

//Activate notify
const uint8_t notificationOn[] = {0x1, 0x0};
const uint8_t notificationOff[] = {0x0, 0x0};

//LED GPIO
#define LED_GPIO_BREAK 17
#define LED_GPIO_RIGHT 16
#define LED_GPIO_LEFT 15
#define LED_GPIO_BLUETOOTH 5
#define VOLTAGE_INPUT 13

//boolean for new light readings 
boolean New_break = false;
boolean New_right = false;
boolean New_left = false;

//Connect to the BLE Server that has the name, Service, and Characteristics
bool connectToServer(BLEAddress pAddress) {
  BLEClient* pClient = BLEDevice::createClient();
 
  // Connect to the remove BLE Server.
  pClient->connect(pAddress);
  Serial0.println(" - Connected to server");
 
  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService* pRemoteService = pClient->getService(MotorcycleServiceUUID);
  if (pRemoteService == nullptr) {
    Serial0.print("Failed to find our service UUID: ");
    Serial0.println(MotorcycleServiceUUID.toString().c_str());
    return (false);
  }
 
  // Obtain a reference to the characteristics in the service of the remote BLE server.
  BreakLightCharacteristic = pRemoteService->getCharacteristic(BreakLightCharacteristicsUUID);
  RightLightCharacteristic = pRemoteService->getCharacteristic(RightLightCharacteristicsUUID);
  LeftLightCharacteristic = pRemoteService->getCharacteristic(LeftLightCharacteristicsUUID);

  if (BreakLightCharacteristic == nullptr || RightLightCharacteristic == nullptr || LeftLightCharacteristic == nullptr) {
    Serial0.print("Failed to find our characteristic UUID");
    return false;
  }
  Serial0.println(" - Found our characteristics");
 
  //Assign callback functions for the Characteristics
  BreakLightCharacteristic->registerForNotify(BreakNotifyCallback);
  RightLightCharacteristic->registerForNotify(RightNotifyCallback);
  LeftLightCharacteristic->registerForNotify(LeftNotifyCallback);
  return true;
}

//Callback function that gets called, when another device's advertisement has been received
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.getName() == bleServerName) { //Check if the name of the advertiser matches
      advertisedDevice.getScan()->stop(); //Scan can be stopped, we found what we are looking for
      pServerAddress = new BLEAddress(advertisedDevice.getAddress()); //Address of advertiser is the one we need
      doConnect = true; //Set indicator, stating that we are ready to connect
      Serial0.println("Device found. Connecting!");
    }
  }
};

//When the BLE Server sends a new Break light reading with the notify property
static void BreakNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  //store temperature value
  Break_Char = (char*)pData;
  New_break = true;
}

//When the BLE Server sends a new Right light reading with the notify property
static void RightNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  //store humidity value
  Right_Char = (char*)pData;
  New_right = true;
}

//When the BLE Server sends a new Left light reading with the notify property
static void LeftNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  //store humidity value
  Left_Char = (char*)pData;
  New_left = true;
}

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_GPIO_BREAK, OUTPUT);
  pinMode(LED_GPIO_LEFT, OUTPUT);
  pinMode(LED_GPIO_RIGHT, OUTPUT);
  pinMode(LED_GPIO_BLUETOOTH, OUTPUT);
  pinMode(VOLTAGE_INPUT, INPUT);

  //Start serial0 communication
  Serial0.begin(115200);
  Serial0.println("Starting Arduino BLE Client application...");

  //Init BLE device
  BLEDevice::init("");

  // Retrieve a Scanner and set the callback we want to use to be informed when we have detected a new device. 
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(30);
}

void loop() {
  // put your main code here, to run repeatedly:
  // int volt = analogRead(13);
  // Serial0.println(volt);
  if (doConnect == true) {
    digitalWrite(LED_GPIO_BLUETOOTH, HIGH);
    if (connectToServer(*pServerAddress)) {
      Serial0.println("We are now connected to the BLE Server.");
      //Activate the Notify property of each Characteristic
      BreakLightCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2901))->writeValue((uint8_t*)notificationOn, 2, true);
      RightLightCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);
      LeftLightCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2903))->writeValue((uint8_t*)notificationOn, 2, true);
      connected = true;
    } else {
      Serial0.println("We have failed to connect to the server; Restart your device to scan for nearby BLE server again.");
      digitalWrite(LED_GPIO_BLUETOOTH, LOW);
    }
    doConnect = false;
  }
  //   if (doConnect == true && volt < 3000 ) {
  //     while(volt < 3000){
  //     Serial0.println("Battery 20% please charge");
  //     digitalWrite(LED_GPIO_BLUETOOTH, HIGH); // Set GPIO7 active high
  //     delay(100);  // delay of one second
  //     digitalWrite(LED_GPIO_BLUETOOTH, LOW); // Set GPIO7 active low
  //     delay(100); // delay of one second
  //     }
  // }
 if(New_break){
  digitalWrite(LED_GPIO_BREAK, HIGH);
  New_break = false;
 }
 else{
  digitalWrite(LED_GPIO_BREAK, LOW);
 }
 if(New_right){
  digitalWrite(LED_GPIO_RIGHT, HIGH);
  New_right = false;
 }
 else{
  digitalWrite(LED_GPIO_RIGHT, LOW);
 }
 if(New_left){
  digitalWrite(LED_GPIO_LEFT, HIGH);
  New_left = false;
 }
 else{
  digitalWrite(LED_GPIO_LEFT, LOW);
 }
  delay(100);
}
