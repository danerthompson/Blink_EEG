#include <Arduino.h>
//#include <ADS1219.h>
#include "ADS1219_V1.h"
#include <Wire.h>
#include <SD.h>

#define EEG_CH ADS_CH1
#define ECG_CH ADS_CH0
#define REF_CH ADS_CH3
#define BAT_CH ADS_CH2

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

ADS1219 ads(ADC_ADDR, ADC_RDY);
TwoWire adc_i2c = TwoWire(0);
float ref_electrode;
float EEG_signal;
float ECG_signal;
float bat_voltage;
float batScale = 82.0 / (82.0 + 20.0);

unsigned long timeElapsed;
uint32_t inReg = 0;

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


  delay(500);

  Serial.begin(115200);
  while (!Serial) {
    delay(100);
  }
  Serial.setTxTimeoutMs(0); // Remove delay when no USB connection is present

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
  
  // ADC initilization
  adc_i2c.begin(I2C_SDA, I2C_SCL);
  ads.init(&adc_i2c);
  ads.setConversionMode(ADS_CONTINUOUS_MODE);
  ads.setSpeed(ADS_SAMPLE_1000SPS);
  ads.setReference(ADS_REF_EXTERNAL);
  ads.setMUX(EEG_CH);
  ads.startConversion();
  
}

void loop() {
  // put your main code here, to run repeatedly:

  
  EEG_signal = ads.computeVolts(ads.readADC(), 3.0);
  //Serial.println(EEG_signal,4);
  //Serial.println(micros());

  inReg = REG_READ(GPIO_IN_REG);

  data = String(millis()) + "," + String(EEG_signal,6) + "," + 
         String((inReg & 0x0000008)>>BUTT_LEFT) + "," + String((inReg & 0x00000010)>>BUTT_MID) + "," + String((inReg & 0x00200000)>>BUTT_RIGHT);

  Serial.println(data);
  //Serial.println(micros());
  //data = String(millis()) + "," + String(ECG_signal,5) + "," + String(EEG_signal,5) + "," + String(bat_voltage,3) + "," + String(ref_electrode,5) + "," +
  //       String((inReg & 0x0000008)>>BUTT_LEFT) + "," + String((inReg & 0x00000010)>>BUTT_MID) + "," + String((inReg & 0x00200000)>>BUTT_RIGHT);

  //delay(10);
  
  if (SD_flag) {

      // Save file every 250 ms
      if (millis() - timeElapsed >= 250) {
        dir.close();
        dir =  SD.open(filename, FILE_APPEND);  // Don't use FILE_WRITE unless you want to lose data
        timeElapsed += 250;
      }
      dir.println(data);
  }

} // End of loop


