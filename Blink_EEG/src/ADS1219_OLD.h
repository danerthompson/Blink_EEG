#ifndef ADS1219_OLD_h
#define ADS1219_OLD_h

#include <Wire.h>

#define WREG_CMD 0x40
#define RDATA_CMD 0x10
#define START_CMD 0x08

#define ADS_SAMPLE_20SPS 0
#define ADS_SAMPLE_90SPS 1
#define ADS_SAMPLE_330SPS 2
#define ADS_SAMPLE_1000SPS 3

#define ADS_REF_INTERNAL 0
#define ADS_REF_EXTERNAL 1

#define ADS_SINGLE_SHOT_MODE 0
#define ADS_CONTINUOUS_MODE 1

// Binary values in datasheet
#define ADS_CH0 3
#define ADS_CH1 4
#define ADS_CH2 5
#define ADS_CH3 6
#define ADS_VDD2 7

class ADS1219 {
    public:
        ADS1219( int ADDR, int RDY_PIN );
        void init( TwoWire *i2c_wire );
        void setSpeed( uint8_t speed );
        void setReference( uint8_t ref );
        void setConversionMode( uint8_t convMode );
        void startConversion( uint8_t channel );
        uint32_t readADC();
        float computeVolts( uint32_t data, float refVoltage );

    private:
        int _addr;
        int _rdy;
        TwoWire *_wire;
        uint8_t _config_reg = 0xFF;

};





#endif