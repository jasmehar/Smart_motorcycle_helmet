
#include <Wire.h>
#include <Adafruit_TSL2561_U.h>
#include <Adafruit_Sensor.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define I2C_SDA 37
#define I2C_SCL 38

#define LED_GPIO_BREAK 17
#define LED_GPIO_RIGHT 16
#define LED_GPIO_LEFT 15

#define bleServerName "Motorcycle_server"
#define Motorcycle_UUID "7107eb72-77e7-411b-8dcb-af365300a59c"
bool deviceConnected = false;
//all this code be dumb dumb repetition since it is 2 am and i am not bout to optimize

// Luminosity Characteristic and Descriptor

BLECharacteristic BreakLightCharacteristics("46dfbfba-d66f-42dc-8128-1b636a527768", BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor BreakLightDescriptor(BLEUUID((uint16_t)0x2901));

BLECharacteristic RightLightCharacteristics("f85e8339-026a-4e26-9dc4-cfecaa207113", BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor RightLightDescriptor(BLEUUID((uint16_t)0x2902));

BLECharacteristic LeftLightCharacteristics("2ad68c57-8b7f-48fa-ab97-d3ca82b4d6f1", BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor LeftLightDescriptor(BLEUUID((uint16_t)0x2903));

//Setup callbacks onConnect and onDisconnect
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
  };
  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
  }
};

//----------------------------------------------------------------------------------------------------------------- sensor code

TwoWire I2C_TSL2561 = TwoWire(0);

float luminosity_Break = 0;
float luminosity_Right = 0;
float luminosity_Left = 0;

//define TSL2561 objects
Adafruit_TSL2561_Unified Break_light = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 1);  //address and sensor ID
Adafruit_TSL2561_Unified Left_light = Adafruit_TSL2561_Unified(TSL2561_ADDR_LOW, 2);
Adafruit_TSL2561_Unified Right_light = Adafruit_TSL2561_Unified(TSL2561_ADDR_HIGH, 3);

void TSL2561_configureSensor() {
  //third is the clock frequency?? what will that be for TSL2561
  I2C_TSL2561.begin(I2C_SDA, I2C_SCL, 100000);

  // Unfortunately we will need to change the I2C address since I am not sure the address will be the same on dev board
  Break_light.begin(&I2C_TSL2561);
  Left_light.begin(&I2C_TSL2561);
  Right_light.begin(&I2C_TSL2561);

  //automatic gain (will need to test)
  Break_light.enableAutoRange(true);
  Left_light.enableAutoRange(true);
  Right_light.enableAutoRange(true);

  // fast but low resolution since there is a big difference between off and on and we need fast
  Break_light.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);
  Left_light.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);
  Right_light.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);
}

void setup() {
  // put your setup code here, to run once:
  Serial0.begin(115200);  // Initializes Serial0 Monitor for viewing data via USB on the computer
  TSL2561_configureSensor();
  pinMode(LED_GPIO_BREAK, OUTPUT);
  pinMode(LED_GPIO_LEFT, OUTPUT);
  pinMode(LED_GPIO_RIGHT, OUTPUT);
  //----------------------------------------------------------------------------------------------------------------- bluetooth code
  BLEDevice::init(bleServerName);                       //create the device
  BLEServer *motor_server = BLEDevice::createServer();  // Create the BLE Server
  motor_server->setCallbacks(new MyServerCallbacks());
  BLEService *MotorService = motor_server->createService(Motorcycle_UUID);  // Create the BLE Service

  // Create BLE Characteristics and Create a BLE Descriptor
  // Break Light
  MotorService->addCharacteristic(&BreakLightCharacteristics);
  BreakLightDescriptor.setValue("Break Light Luminosity");
  BreakLightCharacteristics.addDescriptor(&BreakLightDescriptor);
  // Right Light
  MotorService->addCharacteristic(&RightLightCharacteristics);
  RightLightDescriptor.setValue("Right Light Luminosity");
  RightLightCharacteristics.addDescriptor(&RightLightDescriptor);
  // Left Light
  MotorService->addCharacteristic(&LeftLightCharacteristics);
  LeftLightDescriptor.setValue("Left Light Luminosity");
  LeftLightCharacteristics.addDescriptor(&LeftLightDescriptor);
  // Start the service
  MotorService->start();
  // Start advertising
  BLEAdvertising *motor_advertising = BLEDevice::getAdvertising();
  motor_advertising->addServiceUUID(Motorcycle_UUID);
  motor_server->getAdvertising()->start();
  Serial0.println("Waiting a client connection to notify...");
}

void loop() {
  // put your main code here, to run repeatedly:
  if (deviceConnected) {
    // Receives the most recent Event from the sensor
    sensors_event_t Break_event;
    sensors_event_t Right_event;
    sensors_event_t Left_event;

    Break_light.getEvent(&Break_event);
    Right_light.getEvent(&Right_event);
    Left_light.getEvent(&Left_event);
    //if any is over 800 lux then notify
    if (Break_event.light > 800) {
      luminosity_Break = Break_event.light;
      static char Break_light_string[4];
      dtostrf(luminosity_Break, 3, 0, Break_light_string);

      BreakLightCharacteristics.setValue(Break_light_string);
      BreakLightCharacteristics.notify();
      digitalWrite(LED_GPIO_BREAK, HIGH);
      Serial0.println("Break light is on!");
      Serial0.print("luminosity_Break From Break Light: ");
      Serial0.print(Break_light_string);
      Serial0.println(" lux");
    } else {
      digitalWrite(LED_GPIO_BREAK, LOW);
    }
    if (Right_event.light > 800) {
      luminosity_Right = Right_event.light;
      static char Right_light_string[4];
      dtostrf(luminosity_Right, 3, 0, Right_light_string);

      RightLightCharacteristics.setValue(Right_light_string);
      RightLightCharacteristics.notify();
      digitalWrite(LED_GPIO_RIGHT, HIGH);
      Serial0.println("Right light is on!");
      Serial0.print("luminosity_Right From Right Light: ");
      Serial0.print(Right_light_string);
      Serial0.println(" lux");
    }
    else{
      digitalWrite(LED_GPIO_RIGHT, LOW);
    }
    if (Left_event.light > 800) {
      luminosity_Left = Left_event.light;
      static char Left_light_string[4];
      dtostrf(luminosity_Left, 3, 0, Left_light_string);

      LeftLightCharacteristics.setValue(Left_light_string);
      LeftLightCharacteristics.notify();
      digitalWrite(LED_GPIO_LEFT, HIGH);
      Serial0.println("Left light is on!");
      Serial0.print("luminosity_Left From Left Light: ");
      Serial0.print(Left_light_string);
      Serial0.println(" lux");
    }
    else{
      digitalWrite(LED_GPIO_LEFT, LOW);
    }
    delay(10);
  }
}
