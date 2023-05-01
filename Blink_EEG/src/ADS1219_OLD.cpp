#include "ADS1219_OLD.h"

ADS1219::ADS1219( int ADDR, int RDY_PIN ) {
    _addr = ADDR;
    _rdy = RDY_PIN;
}

void ADS1219::init( TwoWire *i2c_wire ) {
    _wire = i2c_wire;
}

void ADS1219::setSpeed( uint8_t speed ) {
    _config_reg &= 0xF3;
    _config_reg |= (speed<<2);

    _wire->beginTransmission(_addr);
    _wire->write(WREG_CMD);
    _wire->write(_config_reg);
    _wire->endTransmission();  
}

void ADS1219::setReference( uint8_t ref ) {
    _config_reg &= 0xFE;
    _config_reg |= ref;

    _wire->beginTransmission(_addr);
    _wire->write(WREG_CMD);
    _wire->write(_config_reg);
    _wire->endTransmission();  

}

void ADS1219::setConversionMode( uint8_t convMode ) {
    _config_reg &= 0xFD;
    _config_reg |= (convMode<<1);

    _wire->beginTransmission(_addr);
    _wire->write(WREG_CMD);
    _wire->write(_config_reg);
    _wire->endTransmission();  
}

void ADS1219::startConversion( uint8_t channel ) {
    _config_reg &= 0x1F; // Clear first three bits
    _config_reg |= (channel<<5);

    _wire->beginTransmission(_addr);
    _wire->write(WREG_CMD);
    _wire->write(_config_reg);
    _wire->endTransmission();  

    _wire->beginTransmission(_addr);
    _wire->write(START_CMD);
    _wire->endTransmission();  
}

uint32_t ADS1219::readADC() {
    uint32_t data;
    uint8_t byte1, byte2, byte3;
    
    _wire->beginTransmission(_addr);
    _wire->write(RDATA_CMD);
    _wire->endTransmission();  

    _wire->requestFrom(_addr, 3, 1);
    while(3 > _wire->available()) {}
    data = _wire->read();
    data <<= 8;
    data |= _wire->read();
    data <<= 8;
    data |= _wire->read();
    return (data << 8) >> 8;


    //byte1 = _wire->read();
    //byte2 = _wire->read();
    //byte3 = _wire->read();
    //data = 0x00FFFFFF & ( (byte1<<16) | (byte2<<8) | (byte3) );
    //return data;
}

float ADS1219::computeVolts( uint32_t data, float refVoltage ) {
    return float(data)/16777215.0*refVoltage;
}