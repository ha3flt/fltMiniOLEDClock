/****************************************************************************
** Module...: fltCSensors.cpp                                              **
** Purpose..:                                                              **
**                                                                         **
** Notes....:                                                              **
**                                                                         **
** Copyright (c) 2025-2026 by HA3FLT                                       **
****************************************************************************/
#include <Adafruit_BMP085.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include "fltCSensors.h"

#define ONE_WIRE_PIN    D5
OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature ds(&oneWire);

Adafruit_BMP085 bmp;

/****************************************************************************
** Purpose: CSensors - Init                                                **
** Param's: None                                                           **
** Returns: None                                                           **
****************************************************************************/
bool CSensors::init()
{
    ds.begin();
    ds.setWaitForConversion(false);
    auto devices = ds.getDeviceCount();
    // We will take care of an eventually missing DS18B20 device later
    Serial.print(F("OneWire device count: "));
    Serial.print(devices, DEC);
    Serial.println(F(""));

    if (!bmp.begin()) {
      	Serial.println(F("Could not find a valid BMP085 sensor"));
        return false;
    }
    // Calibrate pressure sensor as a default value
    if (getPressureSensorOffset() == 0.0) {
        setPressureSensorOffset(calculatePressureSensorOffset(
            (float)FMOC_MEASURED_RAW_PA, (float)FMOC_REFERENCE_SEA_LEVEL_PA));
    }
    return true;
}

/****************************************************************************
** Purpose: BMP085 temperature                                             **
** Param's: None                                                           **
** Returns: None                                                           **
****************************************************************************/
bool CSensors::isValidTemperature1(float temperature)
{
    return temperature > -43.0 && ForcedCounter == 0;
}

float CSensors::getTemperatureSensorOffset1()
{
    TemperatureSensorOffset1 = getSettings()->getSensor1Offset();
    return TemperatureSensorOffset1;
}

void CSensors::setTemperatureSensorOffset1(float offset)
{
    TemperatureSensorOffset1 = offset;
    getSettings()->setSensor1Offset(TemperatureSensorOffset1);
}

float CSensors::getTemperature1()
{
    LastTemperature1 = bmp.readTemperature();
    // We will display the potentially incorrect value, but we don't store it. The
    //   BMP085's lowest temp. is about -46.3 *C when both its registers are zero.
    if (isValidTemperature1(LastTemperature1)) {
        LastTemperature1 += TemperatureSensorOffset1;
        setMinTemperature1(LastTemperature1);
        setMaxTemperature1(LastTemperature1);
    }
    return LastTemperature1;
}

float CSensors::getLastTemperature1()
{
    return LastTemperature1;
}

float CSensors::getMinTemperature1()
{
    return MinTemperature1;
}

void CSensors::setMinTemperature1(float temperature)
{
    if (temperature < MinTemperature1) {
        MinTemperature1 = temperature;
    }
}

float CSensors::getMaxTemperature1()
{
    return MaxTemperature1;
}

void CSensors::setMaxTemperature1(float temperature)
{
    if (temperature > MaxTemperature1) {
        MaxTemperature1 = temperature;
    }
}

void CSensors::resetMinMaxTemperature1()
{
    if (isValidTemperature1(LastTemperature1)) {
        MinTemperature1 = LastTemperature1;
        MaxTemperature1 = LastTemperature1;
    }
}

/****************************************************************************
** Purpose: DS18B20 temperature                                            **
** Param's: None                                                           **
** Returns: None                                                           **
****************************************************************************/
bool CSensors::isValidTemperature2(float temperature)
{
    return temperature > DEVICE_DISCONNECTED_C && ForcedCounter == 0;
}

float CSensors::getTemperatureSensorOffset2()
{
    TemperatureSensorOffset2 = getSettings()->getSensor2Offset();
    return TemperatureSensorOffset2;
}

void CSensors::setTemperatureSensorOffset2(float offset)
{
    TemperatureSensorOffset2 = offset;
    getSettings()->setSensor2Offset(TemperatureSensorOffset2);
}

float CSensors::requestTemperature2()
{
    return ds.requestTemperatures();
}

bool CSensors::isConversionComplete2()
{
    return ds.isConversionComplete();
}

float CSensors::getTemperature2(bool wait /*= false*/)
{
    if (wait) {
        while (!isConversionComplete2()) {};
    }
    LastTemperature2 = ds.getTempCByIndex(0);
    // We will display the potentially incorrect
    //   value or error code, but we don't store it
    if (isValidTemperature2(LastTemperature2)) {
        LastTemperature2 += TemperatureSensorOffset2;
        setMinTemperature2(LastTemperature2);
        setMaxTemperature2(LastTemperature2);
    }
    return LastTemperature2;
}

float CSensors::getLastTemperature2()
{
    return LastTemperature2;
}

float CSensors::getMinTemperature2()
{
    return MinTemperature2;
}

void CSensors::setMinTemperature2(float temperature)
{
    if (temperature < MinTemperature2) {
        MinTemperature2 = temperature;
    }
}

float CSensors::getMaxTemperature2()
{
    return MaxTemperature2;
}

void CSensors::setMaxTemperature2(float temperature)
{
    if (temperature > MaxTemperature2) {
        MaxTemperature2 = temperature;
    }
}

void CSensors::resetMinMaxTemperature2()
{
    if (isValidTemperature2(LastTemperature2)) {
        MinTemperature2 = LastTemperature2;
        MaxTemperature2 = LastTemperature2;
    }
}

/****************************************************************************
** Purpose: Debug function                                                 **
** Param's: None                                                           **
** Returns: None                                                           **
****************************************************************************/
void CSensors::debugTemperature()
{
    Serial.print(F("Temperature sensor offset 1: "));
    Serial.print(TemperatureSensorOffset1);
    Serial.println(F(" *C"));
    Serial.print(F("Temperature 1 = "));
    Serial.print(getTemperature1());
    Serial.println(F(" *C"));
    Serial.println(F("----------"));

    Serial.print(F("Device count: "));
    Serial.print(ds.getDeviceCount(), DEC);
    Serial.println(F(""));
    Serial.print(F("Temperature sensor offset 2: "));
    Serial.print(TemperatureSensorOffset2);
    Serial.println(F(" *C"));

    Serial.println(F("Requesting temperature..."));
    requestTemperature2();
    Serial.println(F("Done."));
    Serial.println(F("Getting temperature with waiting..."));
    Serial.print(F("Temperature 2 = "));
    Serial.print(getTemperature2(true));
    Serial.println(F(" *C"));
    Serial.println(F("----------"));
}

/****************************************************************************
** Purpose: BMP085 - Pressure                                              **
** Param's: None                                                           **
** Returns: None                                                           **
****************************************************************************/
uint16_t CSensors::getAltitudeMeters()
{
    AltitudeMeters = getSettings()->getAltitudeMeters();
    return AltitudeMeters;
}

void CSensors::setAltitudeMeters(uint16_t altitude)
{
    AltitudeMeters = altitude;
    getSettings()->setAltitudeMeters(AltitudeMeters);
}

float CSensors::currentLevelToSeaLevelPressure(float pressure)
{
    return pressure * pow(1.0 - (AltitudeMeters / 44330.0), -5.255);
}

float CSensors::seaLevelPressureToCurrentLevel(float seaLevelPressure)
{
    return seaLevelPressure * pow(1.0 - (AltitudeMeters / 44330.0), 5.255);
}

float CSensors::getPressureSensorOffset()
{
    PressureSensorOffset = (float)getSettings()->getPressureOffset();
    return PressureSensorOffset;
}

void CSensors::setPressureSensorOffset(float offset)
{
    PressureSensorOffset = offset;
    getSettings()->setPressureOffset((int32_t)PressureSensorOffset);
}

float CSensors::calculatePressureSensorOffset(float measuredRawPa, float referenceP0Pa)
{
    return seaLevelPressureToCurrentLevel(referenceP0Pa) - measuredRawPa;
}

float CSensors::getRawPressure()
{
    return bmp.readPressure();
}

float CSensors::getPressure()
{
    LastPressure = currentLevelToSeaLevelPressure(bmp.readPressure() + PressureSensorOffset);
    setMinPressure(LastPressure);
    setMaxPressure(LastPressure);
    return LastPressure;
}

float CSensors::getLastPressure()
{
    return LastPressure;
}

float CSensors::getMinPressure()
{
    return MinPressure;
}

float CSensors::getMaxPressure()
{
    return MaxPressure;
}

void CSensors::setMinPressure(float pressure)
{
    if (pressure < MinPressure) {
        MinPressure = pressure;
    }
}

void CSensors::setMaxPressure(float pressure)
{
    if (pressure > MaxPressure) {
        MaxPressure = pressure;
    }
}

void CSensors::resetMinMaxPressure()
{
    MinPressure = LastPressure;
    MaxPressure = LastPressure;
}

/****************************************************************************
** Purpose: Debug function                                                 **
** Param's: None                                                           **
** Returns: None                                                           **
****************************************************************************/
void CSensors::debugPressure()
{
    // Calibration test
    uint16_t altitudeMeters = 116;
    float inputRawPa = 99873;
    float referenceP0Pa = 101400;
    //float sensorOffsetPa = calculatePressureSensorOffset(inputRawPa, referenceP0Pa);
    float sensorOffsetPa = referenceP0Pa * pow(1.0 - (altitudeMeters / 44330.0), 5.255) - inputRawPa;

    Serial.print(F("Input altitude: "));
    Serial.print(altitudeMeters);
    Serial.println(F(" meters"));
    Serial.print(F("Input raw pressure: "));
    Serial.print(inputRawPa);
    Serial.println(F(" Pa"));
    Serial.print(F("Input sea level pressure: "));
    Serial.print(referenceP0Pa);
    Serial.println(F(" Pa"));
    Serial.print(F("Calculated offset = "));
    Serial.print(sensorOffsetPa);
    Serial.println(F(" Pa"));
    Serial.println(F(".........."));

    // Read the pressure from the sensor and calculate the pressure
    float prPa = getRawPressure();

    float p0mbar = getPressure();
    uint16_t p0hPa = round(p0mbar / 100.0);

    Serial.print(F("Stored altitude: "));
    Serial.print(AltitudeMeters);
    Serial.println(F(" meters"));
    Serial.print(F("Stored sensor offset: "));
    Serial.print(PressureSensorOffset);
    Serial.println(F(" Pa"));
    Serial.print(F("Raw pressure: "));
    Serial.print(prPa);
    Serial.println(F(" hPa"));
    Serial.print(F("Pressure = "));
    Serial.print(p0hPa);
    Serial.println(F(" hPa"));
    Serial.println(F("**********"));
}

/****************************************************************************
** Purpose: BMP085 - Humidity                                              **
** Param's: None                                                           **
** Returns: None                                                           **
****************************************************************************/
uint8_t CSensors::getHumidity()
{
    LastHumidity = 0;
    setMinHumidity(LastHumidity);
    setMaxHumidity(LastHumidity);
    return 0;
}

uint8_t CSensors::getLastHumidity()
{
    return LastHumidity;
}

uint8_t CSensors::getMinHumidity()
{
    return MinHumidity;
}

void CSensors::setMinHumidity(uint8_t humidity)
{
    if (humidity < MinHumidity) {
        MinHumidity = humidity;
    }
}

uint8_t CSensors::getMaxHumidity()
{
    return MaxHumidity;
}

void CSensors::setMaxHumidity(uint8_t humidity)
{
    if (humidity > MaxHumidity) {
        MaxHumidity = humidity;
    }
}

void CSensors::resetMinMaxHumidity()
{
    MinHumidity = LastHumidity;
    MaxHumidity = LastHumidity;
}

/****************************************************************************
** Purpose: Debug function                                                 **
** Param's: None                                                           **
** Returns: None                                                           **
****************************************************************************/
void CSensors::debugHumidity()
{
}

/****************************************************************************
** Purpose: Callback function                                              **
** Param's: None                                                           **
** Returns: None                                                           **
****************************************************************************/
void CSensors::loop(uint32_t now)
{
    static uint32_t lastUpdateTime = ULONG_MAX / 2;
    // Sensor management - do some initial reading to make sure that we get real values.
    // 1 second is probably a safe value for the DS1820 sensors, and the BMP085 is much faster
    if ((now - lastUpdateTime) / 1000 >= 1) {
        if ((now - lastUpdateTime) / 1000 >= FMOC_SENSORS_UPDATES_INTERVAL_SEC
                || ForcedCounter > 0) {
            // The main update block
            lastUpdateTime = now;
            // Force reading for SENSORS_FORCED_UPDATES times
            //   (last time, the min/max values will be updated)
            if (ForcedCounter > 0) {
                --ForcedCounter;
            }
            // These are the actual temperature reading calls
            // Min/max values will only be updated if ForcedCounter is zero
            (void) getTemperature1();
            (void) getTemperature2();
            // Request the next value
            requestTemperature2();
            getHumidity();
        }
    }
}
