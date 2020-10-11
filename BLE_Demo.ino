#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <time.h>

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;

// See the following for generating UUIDs:
//从下列网址生成UUIDs
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"


class MyServerCallbacks: public BLEServerCallbacks {  
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;            //如果有设备连接
      Serial.println("Connected");
      //BLEDevice::startAdvertising();     //开始广播
    };

    void onDisconnect(BLEServer* pServer) {    //如果无设备
      deviceConnected = false;  
      Serial.println("Disconnected");            
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {   //创建MyCallbacks类，其继承自BLECharacteristicCallbacks
    void onWrite(BLECharacteristic *pCharacteristic) {
      Serial.println("onWrite");
    }
    void onRead(BLECharacteristic *pCharacteristic){
      Serial.println("onRead");
    }
    void onNotify(BLECharacteristic *pCharacteristic){
      //Serial.println("onNotify");
    }
};


void setup() {
  Serial.begin(115200);      //串口初始化

  // Create the BLE Device
  BLEDevice::init("ESP32");     //BLE 设备初始化，名称ESP32

  // Create the BLE Server
  pServer = BLEDevice::createServer();    //创建Server
  pServer->setCallbacks(new MyServerCallbacks());    //设置匿名回调函数（实例化MyServerCallbacks）

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);   //创建BLE Service

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(       //创建BLE 特征（Characterristic_UUID,长度）
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |                                        //这些属性值常量可在上文查看
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY 
                     //BLECharacteristic::PROPERTY_INDICATE
                    );

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor

  pCharacteristic->addDescriptor(new BLE2902());     //创建BLE 描述
  pCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();                              //服务启动

  // Start advertising                                       
  /*BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();    //定义了一个BLEAdvertising类指针pAdvertising，它指向BLEDevice::getAdvertising()
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  //设置值为0x00以不广播该参数  set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();                //开始广播*/
  pServer->getAdvertising()->start();
  Serial.println("等待一个客户端连接至notify...");
}

void loop() {
    // notify changed value
    if (deviceConnected) {        //如果设备已连接
        pCharacteristic->setValue((uint8_t*)&value, 4);       //设置值为value
        pCharacteristic->notify();                               //发送通知
        value++;                                         //value自加
        delay(2000); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
    }
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {  //如果断联&&旧设备连接（？）
        delay(500); // 延时，为程序做缓冲  give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // 重新广播
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {          //如果连接&&不是旧设备
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
}
