#include <Arduino.h>
#include <HX711.h>
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 9
#define SS_PIN 10

const int LOADCELL_DOUT_PIN = A2;
const int LOADCELL_SCK_PIN = A3;
const int calVal_addr = 0;

int bca, bca_kal, bca_tare;

float berat, brt_krm, strd_data;

SoftwareSerial SIM800L(2, 3);
LiquidCrystal_PCF8574 lcd(0x27);
HX711_ADC scale(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
MFRC522 mfrc522(SS_PIN, RST_PIN);

String API_Key_jala = "KCFWAYSEXVSNFSCN";
String APN = "M2MINTERNET";
String tempRFID;

unsigned long t = 0;

void Show_RFID();
void Scan_RFID();
String Read_RFID();
int baca_btn();
int baca_btn_kal();
int baca_tare();
void SetupModule();
void Read_Weight();
void SendToServer();
void ShowSerialData();
void Loadcell_Init();
void Calibrate();