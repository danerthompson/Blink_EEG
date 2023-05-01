#include <Arduino.h>
#include <ADS1219.h>
#include <Wire.h>
#include <SD.h>

#define ADC_RDY 10
#define LED 20
#define BUTT_LEFT 3
#define BUTT_MID 4
#define BUTT_RIGHT 21

#define I2C_SDA 5
#define I2C_SCL 6
#define ADC_ADDR 0x40

#define SPI_MISO 2
#define SPI_MOSI 7
#define SPI_CLK 0
#define SPI_CS 1

ADS1219 ads(ADC_RDY, ADC_ADDR);
TwoWire adc_i2c = TwoWire(0);
float ref_electrode;
float EEG_signal;
float ECG_signal;
float bat_voltage;
float batScale = 82.0 / (82.0 + 20.0);

unsigned long timeElapsed;

SPIClass SPI_SD(FSPI);
String filename;
File dir;
int SD_flag = 1;
String data;

void setup() {
  // put your setup code here, to run once:
  pinMode(LED, OUTPUT);
  pinMode(ADC_RDY, INPUT);
  pinMode(BUTT_LEFT, INPUT);
  pinMode(BUTT_MID, INPUT);
  pinMode(BUTT_RIGHT, INPUT);

  pinMode(SPI_CS, OUTPUT);

  

  Serial.begin(115200);
  while (!Serial) {
    delay(100);
  }

  // ADC initilization
  ads.begin(&adc_i2c, I2C_SDA, I2C_SCL);
  ads.setVoltageReference(REF_EXTERNAL);
  ads.setDataRate(1000);

  // SD card initialization
  SPI_SD.begin(SPI_CLK, SPI_MISO, SPI_MOSI, SPI_CS);
  delay(100);
  filename = "/0.csv";
  if (!SD.begin(SPI_CS, SPI_SD, 4000000)) {
    Serial.println("Failed to initialize SD card");
    SD_flag = 0;
    delay(500);\
  }
  int filenum = 0;
  while (SD.exists(filename)) {
    filename = "/" + String(filenum) + ".csv";
    filenum++;
  }

  if (SD_flag) {
    
    dir =  SD.open(filename, FILE_APPEND);  // Don't use FILE_WRITE unless you want to lose data
  }
  

}

void loop() {
  // put your main code here, to run repeatedly:
  ECG_signal = float(ads.readSingleEnded(0)) / 8388607.0 * 3.0;
  EEG_signal = float(ads.readSingleEnded(1)) / 8388607.0 * 3.0;
  bat_voltage = float(ads.readSingleEnded(2)) / 8388607.0 * 3.0 / batScale;
  ref_electrode = float(ads.readSingleEnded(3)) / 8388607.0 * 3.0;
  
  data = String(millis()) + "," + String(ECG_signal,5) + "," + String(EEG_signal,5) + "," + String(bat_voltage,3) + "," + String(ref_electrode,5) + "," +
         String(digitalRead(BUTT_LEFT)) + "," + String(digitalRead(BUTT_MID)) + "," + String(digitalRead(BUTT_RIGHT));

  Serial.println(data);
  
  if (SD_flag) {

      // Save file every 250 ms
      if (millis() - timeElapsed >= 250) {
        dir.close();
        dir =  SD.open(filename, FILE_APPEND);  // Don't use FILE_WRITE unless you want to lose data
        timeElapsed += 250;
      }
      dir.println(data);
  }

}