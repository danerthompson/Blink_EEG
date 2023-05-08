#include <Arduino.h>
//#include <ADS1219.h>
#include "ADS1219_V1.h"
#include <Wire.h>
#include <SD.h>
#include "random_forest_classifier.h"

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

#define WINDOW_SIZE 250

float window_data[WINDOW_SIZE] = {0};
int window_index = 0;
float standard_dev, amplitude, derivative;  // Three features to be used in classifiers
float sum, mean;                            // Values used to calculate standard deviation
float minimum, maximum, min_index, max_index;       // Values used to calculate amplitude and derivative features
//float features[3] = {0};
int predictedClass;
int prevClass;
int blinkCount = 0;

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

  Serial.print("Starttime: "); Serial.println(micros());
  float data[3] = {0.000090347133567, 0.048960000000000, -0.000110022471910};
  Serial.println(predictClass(data));
  Serial.print("Endtime: "); Serial.println(micros());

  delay(3000);

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

  // Get initial window of data
  while (window_index < WINDOW_SIZE) {
    window_data[window_index] = ads.computeVolts(ads.readADC(), 3.0);;
    window_index++;
  }
  
}

void loop() {
  // put your main code here, to run repeatedly:

  // Make initial min and max the first value in next window
  minimum = window_data[1];
  maximum = window_data[1];

  // Shift all data one to the left and record min, max, and their indices
  for (int i = 1; i < WINDOW_SIZE; i++) {
    window_data[i-1] = window_data[i];
    if (window_data[i] < minimum) {
      minimum = window_data[i];
      min_index = float(i-1);
    }
    else if (window_data[i] > maximum) {
      maximum = window_data[i];
      max_index = float(i-1);
    }
  }

  // Add new data to last location in window
  EEG_signal = ads.computeVolts(ads.readADC(), 3.0);
  window_data[WINDOW_SIZE-1] = EEG_signal;
  if (window_data[WINDOW_SIZE-1] < minimum) {
    minimum = window_data[WINDOW_SIZE-1];
    min_index = float(WINDOW_SIZE-1);
  }
  else if (window_data[WINDOW_SIZE-1] > maximum) {
    maximum = window_data[WINDOW_SIZE-1];
    max_index = float(WINDOW_SIZE-1);
  }

  // Calculate standard deviation feature
  sum = 0;
  standard_dev = 0;
  for (int i = 0; i < WINDOW_SIZE; i++) {
    sum += window_data[i];
  }
  mean = sum / float(WINDOW_SIZE);
  for (int i = 0; i < WINDOW_SIZE; i++) {
    standard_dev += (window_data[i] - mean) * (window_data[i] - mean);
  }
  standard_dev /= float(WINDOW_SIZE-1);
  standard_dev = sqrt(standard_dev);

  // Calculate amplitude and derivative features
  amplitude = maximum - minimum;
  derivative = amplitude / (max_index - min_index);
  
  float features[3] = {standard_dev, amplitude, derivative};
  predictedClass = predictClass(features);

  if (prevClass == 0 && predictedClass != prevClass) {    // Check if blink changed from 0 to nonzero
    //Serial.print("Blink detected: "); Serial.println(blinkCount++);
    if (predictedClass == 1) {
      Serial.println("Left blink!");
    }
    else if (predictedClass == 2) {
      Serial.println("Right blink!");
    }
  }


  inReg = REG_READ(GPIO_IN_REG);

  data = String(millis()) + "," + String(EEG_signal,6) + "," + 
         String((inReg & 0x0000008)>>BUTT_LEFT) + "," + String((inReg & 0x00000010)>>BUTT_MID) + "," + String((inReg & 0x00200000)>>BUTT_RIGHT);

  //Serial.println(data);
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


