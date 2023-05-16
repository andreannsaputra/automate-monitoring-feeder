#include <Arduino.h>
#include <HX711_ADC.h>
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <MFRC522.h>
#include <main.h>
#include <EEPROM.h>

void setup()
{
  pinMode(5, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);

  SIM800L.begin(9600);
  Serial.begin(57600);
  SPI.begin();
  lcd.begin(20, 4);
  scale.begin();
  mfrc522.PCD_Init();

  Serial.println("Mulai");
  lcd.setBacklight(255);
  lcd.setCursor(1, 0);
  lcd.print("Timbangan  Digital");
  lcd.setCursor(0, 3);
  lcd.print("Init...");
  delay(2000);

  Loadcell_Init();
  lcd.clear();
  delay(2000);
}

void loop()
{
  baca_btn_kal();
  baca_tare();
  if (bca_kal != 0)
  {
    Serial.println("Calibrate load cell");
    Calibrate();
  }
  else
  {
    Read_Weight();
    Show_RFID();
  }

  if (bca_tare != 0)
  {
    Serial.println("tare...");
    scale.tareNoDelay();
    lcd.setCursor(0, 3);
    lcd.print("Tare !!");
    delay(1000);
    lcd.setCursor(0, 3);
    lcd.print("              ");
    bca_tare = 0;
  }
  else
  {
    Read_Weight();
    Show_RFID();
  }

  if (tempRFID != 0)
  {
    baca_btn_kal();
    baca_btn();
    if (bca != 0)
    {
      Serial.println("tombol ok");
      SetupModule();
    }
    else
    {
      Read_Weight();
      Show_RFID();
    }
  }
  else
  {
    Read_Weight();
    Show_RFID();
  }
  if (bca == 0 && bca_kal == 0)
  {
    if (mfrc522.PICC_IsNewCardPresent())
    {
      Scan_RFID();
    }
  }
  if (mfrc522.PICC_IsNewCardPresent())
  {
    Scan_RFID();
  }

  delay(100);
}

void Show_RFID()
{
  lcd.setCursor(0, 0);
  lcd.print("ID: ");
  lcd.setCursor(4, 0);
  lcd.print(tempRFID);
}

void Scan_RFID()
{
  lcd.setCursor(0, 0);
  lcd.print("ID: ");
  lcd.setCursor(4, 0);
  lcd.print(tempRFID);
  Serial.println("Kartu terbaca");

  if (!mfrc522.PICC_ReadCardSerial())
  {
    return;
  }

  Read_RFID();
  Serial.print("Tersimpan: ");
  Serial.println(tempRFID);

  mfrc522.PICC_HaltA();
}

String Read_RFID()
{
  String content = "";
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();
  tempRFID = content;
  return tempRFID;
}

int baca_btn()
{
  if (digitalRead(7) == LOW)
  {
    if (bca == 1)
    {
      bca = 0;
    }
    else
    {
      bca = 1;
    }
    delay(1000);
  }
  return bca;
}

int baca_btn_kal()
{
  if (digitalRead(8) == LOW)
  {
    if (bca_kal == 1)
    {
      bca_kal = 0;
    }
    else
    {
      bca_kal = 1;
    }
    delay(1000);
  }
  return bca_kal;
}

int baca_tare()
{
  if (digitalRead(5) == LOW)
  {
    if (bca_tare == 1)
    {
      bca_tare = 0;
    }
    else
    {
      bca_tare = 1;
    }
    delay(1000);
  }
  return bca_tare;
}

void SetupModule()
{
  lcd.setCursor(0, 3);
  lcd.print("Mengirim");
  Serial.println("Mengirim");
  if (SIM800L.available())
    Serial.write(SIM800L.read());
  SIM800L.println("AT");
  delay(100);
  SIM800L.println("AT+CMEE=2");
  delay(1000);
  SIM800L.println("AT+CPIN?");
  delay(1000);
  SIM800L.println("AT+CREG?");
  delay(1000);
  SIM800L.println("AT+CGATT?");
  delay(1000);
  SIM800L.println("AT+CIPSHUT");
  delay(1000);
  SIM800L.println("AT+CIPSTATUS");
  delay(2000);
  SIM800L.println("AT+CIPMUX=0");
  delay(2000);

  // setting the APN,
  SIM800L.println("AT+CSTT=\"" + APN + "\"");
  delay(1000);
  ShowSerialData();
  SIM800L.println("AT+CIICR");
  delay(2000);
  ShowSerialData();

  // get local IP adress
  SIM800L.println("AT+CIFSR");
  delay(2000);
  ShowSerialData();

  SIM800L.println("AT+CIPSPRT=0");
  delay(2000);
  ShowSerialData();

  SIM800L.println("AT+CIPSTART=\"TCP\",\"app.jala.tech\",\"80\"");
  delay(3000);
  ShowSerialData();

  SIM800L.println("AT+CIPSEND");
  delay(2000);
  Serial.println();
  ShowSerialData();
  SendToServer();
}

void SendToServer()
{
  String str = "POST /api/device/rfid_ponds/" + tempRFID + "/feeds?api_token=" + API_Key_jala + "&quantity=" + brt_krm + " HTTP/1.1\r\n"
                                                                                                                         "Host: app.jala.tech\r\n\r\n";
  Serial.println(str);
  delay(2000);
  SIM800L.println(str);
  delay(4000);
  ShowSerialData();
  SIM800L.println((char)26);
  delay(4000);
  SIM800L.println();
  ShowSerialData();
  lcd.setCursor(0, 3);
  lcd.print("        ");
  lcd.setCursor(0, 3);
  lcd.print("Terkirim");
  SIM800L.println("AT+CIPSHUT");
  delay(2000);
  ShowSerialData();
  str = "";
  Serial.println("data terkirim!!!");
  lcd.setCursor(0, 3);
  lcd.print("        ");
  bca = 0;
}

void ShowSerialData()
{
  while (SIM800L.available() != 0)
    Serial.write(SIM800L.read());
  delay(2000);
}

void Read_Weight()
{
  static boolean newDataReady = 0;
  const int SerialPrintInterval = 0;
  EEPROM.get(calVal_addr, strd_data);
  scale.setCalFactor(strd_data);
  if (scale.update())
    newDataReady = true;

  if (newDataReady)
  {
    if (millis() > t + SerialPrintInterval)
    {
      float berat = scale.getData();
      brt_krm = berat / 1000;
      lcd.setCursor(0, 2);
      lcd.print("Brt:");

      if (berat < 0)
      {
        berat = 0;
      }
      else
      {
        Serial.print("berat= ");
        Serial.println(berat);
        lcd.setCursor(5, 2);
        lcd.print(berat);
        lcd.print(" ");
        lcd.print("Gram     ");
        lcd.setCursor(0, 3);
      }
      newDataReady = 0;
      t = millis();
    }
  }
}

void Loadcell_Init()
{
  unsigned long stabilizingTime = 2000;
  boolean _tare = true;
  scale.tareNoDelay();
  scale.start(stabilizingTime, _tare);
  if (scale.getTareTimeoutFlag())
  {
    lcd.setCursor(0, 0);
    lcd.print("Sensor Terputus");
    while (1)
      ;
  }
  else
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sensor Terhubung");
    scale.setCalFactor(1.0);
  }
}

void Calibrate()