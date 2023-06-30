
/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updates by chegewara
*/
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLESecurity.h>
#include <RtcDS1302.h>
std::string receivedData;
BLEAdvertising *pAdvertising;
BLESecurity *pSecurity;
// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
class MyCallback : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        receivedData = value;
    }
};

void printDateTime(const RtcDateTime& dt)
{
    char datestring[20];

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
            dt.Day(),
            dt.Month(),
            dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
    Serial.print(datestring);
}

String do_am;
MyCallback *callback;
BLECharacteristic *pCharacteristic;

ThreeWire myWire(18,19,17); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);
void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");
  BLEDevice::init("BLE Server");
  BLEServer *pServer = BLEDevice::createServer();
  pSecurity = new BLESecurity();
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_MITM_BOND);
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE |
                                         BLECharacteristic::PROPERTY_NOTIFY
                                       );

  pCharacteristic->setValue("Hello World says Neil");
  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  callback = new MyCallback();
  pCharacteristic->setCallbacks(callback);

  //RTC
  Rtc.Begin();
  RtcDateTime compiled = RtcDateTime(2023, 6, 29, 13, 51, 30);
  Rtc.SetDateTime(compiled);
  RtcDateTime now = Rtc.GetDateTime();
  if (now < compiled) 
    {
        Serial.println("RTC is older than compile time!  (Updating DateTime)");
        Rtc.SetDateTime(compiled);
    }
    else if (now > compiled) 
    {
        Serial.println("RTC is newer than compile time. (this is expected)");
    }
    else if (now == compiled) 
    {
        Serial.println("RTC is the same as compile time! (not expected but all is fine)");
    }
}

void loop() {
  // put your main code here, to run repeatedly:
  //if(Serial.available())
  //{
    //do_am = Serial.readString();
    //pCharacteristic->setValue(do_am.c_str());
    //pCharacteristic->notify();
  //}
  
  if (!receivedData.empty()) {
        // Xử lý dữ liệu nhận được ở đây
        // Ví dụ:
        String receive = receivedData.c_str();
        char first = receive.charAt(0);
        if (first == '1') Serial.println("1");
        else if (first == '0') Serial.println("0");
        else if (first == 'a') Serial.println(receivedData.c_str());
        else Serial.println(receivedData.c_str());
                
        
        // Đặt lại giá trị receivedData về rỗng để sẵn sàng nhận dữ liệu tiếp theo
        receivedData.clear();
    }
    pAdvertising->start();

    delay(20);

    RtcDateTime now = Rtc.GetDateTime();

    printDateTime(now);
    Serial.println();
    delay(2000);
}
