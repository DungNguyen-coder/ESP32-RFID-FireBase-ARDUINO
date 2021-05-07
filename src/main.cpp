#include <SPI.h>
#include <Arduino.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include "time.h"

#define FB_HOST "rfid-3c0f9-default-rtdb.firebaseio.com" // ten project
#define FB_Authen "Ov7vAkDeMpW7Dw2WFZ9G43Me9rQyHS9E170CyQgR"   // authen
#define SS_PIN 21
#define RST_PIN 22

const char *ntpServer = "asia.pool.ntp.org";
const long gmtOffset_sec = 7 * 3600;
const int daylightOffset_sec = 0;

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key;

FirebaseData data;
FirebaseJson json;

String date, month;
int t;

void getTIME()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return;
  }

  month = (timeinfo.tm_mon + 1) >= 10 ? String(timeinfo.tm_mon + 1) : ('0' + String(timeinfo.tm_mon + 1));
  date = String(timeinfo.tm_mday) + " : " + month + " : " + String(timeinfo.tm_year + 1900);
  Serial.println(date);

  t = timeinfo.tm_hour * 60 + timeinfo.tm_min;
  Serial.println(t);
}

void FireBasePushData(String ID)
{
  String path = "/Deviot/" + ID + "/Work-Time/" + date + "/" + String(t);
  // String s = "{\"" + String(t) + "\": \"oke\"}";
  // json.setJsonData(s);
  // Serial.println(s);
  Firebase.set(data, path, "done");

  path = "/Deviot-Date/" + date + "/" + ID;
  Firebase.set(data, path, "done");
  path = "/Deviot-Month/" + month + "/" + ID;
  Firebase.set(data, path, "done");

  // if (Firebase.set(data, path, "oke"))
  // {
  //   Serial.println("SET JSON --------------------");
  //   Serial.println("PASSED");
  //   Serial.println("PATH: " + data.dataPath());
  //   Serial.println("TYPE: " + data.dataType());
  //   Serial.println("ETag: " + data.ETag());
  //   Serial.println();
  // }
  // else
  // {
  //   Serial.println("FAILED");
  //   Serial.println("REASON: " + data.errorReason());
  //   Serial.println("------------------------------------");
  //   Serial.println();
  // }
}
void LED2_FLASH();

void setup()
{
  Serial.begin(115200);

  // cau hinh chan D2 bao hieu th da duoc doc
  pinMode(2, OUTPUT);
  LED2_FLASH();

  WiFi.begin("P401", "thaonghiadung");
  while (WiFi.status() != WL_CONNECTED)
    ;
  Serial.println(WiFi.localIP());

  Firebase.begin(FB_HOST, FB_Authen);

  SPI.begin();     // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522

  for (byte i = 0; i < 6; i++)
  {
    key.keyByte[i] = 0xFF;
  }

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop()
{
  // kiem tra xem co the nao duoc quet khong
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial())
  {
    LED2_FLASH();
    getTIME();
    Serial.print(F("PICC type: "));
    MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
    Serial.println(rfid.PICC_GetTypeName(piccType));

    // Check is the PICC of Classic MIFARE type
    if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
        piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
        piccType != MFRC522::PICC_TYPE_MIFARE_4K)
    {
      Serial.println(F("Your tag is not of type MIFARE Classic."));
      return;
    }

    String UID = String(rfid.uid.uidByte[0]);
    for (int i = 1; i < rfid.uid.size; i++)
    {
      UID += ":";
      UID += String(rfid.uid.uidByte[i]);
    }
    Serial.println(UID);
    Serial.println();
    FireBasePushData(UID);

    // Halt PICC  (tam dung PICC)
    rfid.PICC_HaltA();

    // Stop encryption on PCD (dung ma hoa PICC)
    rfid.PCD_StopCrypto1();
  }
}

void LED2_FLASH()
{
  digitalWrite(2, 1);
  delay(50);
  digitalWrite(2, 0);
}