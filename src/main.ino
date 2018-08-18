/*

Moteino Weather Station

Copyright (C) 2016 by Xose Pérez <xose dot perez at gmail dot com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "settings.h"
#include <RFM69Manager.h>
#include <LowPower.h>
#include <Wire.h>
#include "SparkFunBME280.h"
#include <SPIFlash.h>

// -----------------------------------------------------------------------------
// Globals
// -----------------------------------------------------------------------------

RFM69Manager radio;
SPIFlash flash(FLASH_SS, 0xEF30);
BME280 bme;

// -----------------------------------------------------------------------------
// Hardware
// -----------------------------------------------------------------------------

void blink(byte times, byte mseconds) {
    pinMode(LED_PIN, OUTPUT);
    for (byte i=0; i<times; i++) {
        if (i>0) delay(mseconds);
        digitalWrite(LED_PIN, HIGH);
        delay(mseconds);
        digitalWrite(LED_PIN, LOW);
    }
}

void hardwareSetup() {
    Serial.begin(SERIAL_BAUD);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BATTERY_PIN, INPUT);
    #if USE_MOSFET
        pinMode(BATTERY_ENABLE_PIN, INPUT);
    #endif
    delay(1);
}

// -----------------------------------------------------------------------------
// BME280
// -----------------------------------------------------------------------------

void bmeForceRead() {

    // We set the sensor in "forced mode" to force a reading.
    // After the reading the sensor will go back to sleep mode.
    uint8_t value = bme.readRegister(BME280_CTRL_MEAS_REG);
    value = (value & 0xFC) + 0x01;
    bme.writeRegister(BME280_CTRL_MEAS_REG, value);

    // Measurement Time (as per BME280 datasheet section 9.1)
    // T_max(ms) = 1.25
    //  + (2.3 * T_oversampling)
    //  + (2.3 * P_oversampling + 0.575)
    //  + (2.4 * H_oversampling + 0.575)
    //  ~ 9.3ms for current settings
    delay(10);

}

void bmeSetup() {

    bme.settings.commInterface = I2C_MODE;
	bme.settings.I2CAddress = 0x77;
    bme.settings.runMode = 1;
    bme.settings.tStandby = 0;
    bme.settings.filter = 0;
    bme.settings.tempOverSample = 1;
    bme.settings.pressOverSample = 1;
    bme.settings.humidOverSample = 1;

    // Make sure sensor had enough time to turn on. BME280 requires 2ms to start up
    delay(10);
	Serial.print("[BME] Begin output: ");
    Serial.println(bme.begin(), HEX);

}

// -----------------------------------------------------------------------------
// RFM69
// -----------------------------------------------------------------------------

void radioSetup() {
    radio.initialize(FREQUENCY, NODEID, NETWORKID, ENCRYPTKEY, GATEWAYID, ATC_RSSI);
    radio.sleep();
}

// -----------------------------------------------------------------------------
// Flash
// -----------------------------------------------------------------------------

void flashSetup() {
    if (flash.initialize()) {
        flash.sleep();
    }
}

// -----------------------------------------------------------------------------
// Messages
// -----------------------------------------------------------------------------

void sendSensor() {

    char buffer[8];

    // Trigger reading
    bmeForceRead();

    // Temperature
    dtostrf(bme.readTempC(), 4, 1, buffer);
    radio.send((char *) "TMP", buffer, (uint8_t) 2);

    // Humidity
    itoa((int) bme.readFloatHumidity(), buffer, 10);
    radio.send((char *) "HUM", buffer, (uint8_t) 2);

    // Pressure
    dtostrf(bme.readFloatPressure() / 100.0F, 6, 1, buffer);
    radio.send((char *) "PRS", buffer, (uint8_t) 2);

}

void sendBattery() {

    unsigned int voltage;

    // LowPowerLabs WeatherShield v2 can use p-mosfet to enable
    // power monitoring, but it ships without this feature. If you
    // add the required components (a p-mosfet plus 2 resistors)
    // change the USE_MOSFET to 1 in the settings.h file
    #if USE_MOSFET
        pinMode(BATTERY_ENABLE_PIN, OUTPUT);
        digitalWrite(BATTERY_ENABLE_PIN, LOW);
    #endif
    voltage = analogRead(BATTERY_PIN);
    #if USE_MOSFET
        pinMode(BATTERY_ENABLE_PIN, INPUT);
    #endif

    // Map analog reading to VIN value
    voltage = BATTERY_RATIO * voltage;

    char buffer[6];
    itoa((int) voltage, buffer, 10);
    radio.send((char *) "BAT", buffer, (uint8_t) 2);

}

void send()  {

    // Send current sensor readings
    sendSensor();

    // Send battery status once every 10 messages, starting with the first one
    static unsigned char batteryCountdown = 0;
    if (batteryCountdown == 0) sendBattery();
    batteryCountdown = (batteryCountdown + 1) % 10;

    // Radio back to sleep
    radio.sleep();

    // Show visual notification
    blink(1, NOTIFICATION_TIME);

}

// -----------------------------------------------------------------------------
// Common methods
// -----------------------------------------------------------------------------

void setup() {
    hardwareSetup();
    bmeSetup();
    flashSetup();
    radioSetup();
}

void loop() {

    // We got here for three possible reasons:
    // - it's the first time (so we report status and battery)
    // - after ~300 seconds (we report status and maybe battery)
    send();

    // Sleep loop
    for (byte i = 0; i < SLEEP_COUNT; i++) {

        // Sleep for 8 seconds (the maximum the WDT accepts)
        LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);

    }

}
