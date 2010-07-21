/***************************************************************************
 * Copyright (C) 2010 by Alexey Aksenov, Alexey Fomichev                   *
 * ezh@ezh.msk.ru, axx@fomichi.ru                                          *
 *                                                                         *
 * This file is part of ENikiBENiki                                        *
 *                                                                         *
 * ENikiBENiki is free software: you can redistribute it and/or modify     *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation, either version 3 of the License, or       *
 * (at your option) any later version.                                     *
 *                                                                         *
 * ENikiBENiki is distributed in the hope that it will be useful,          *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with ENikiBENiki.  If not, see <http://www.gnu.org/licenses/>.    *
 *                                                                         *
 ***************************************************************************/

//#define DEBUG

#ifdef DEBUG
#define debugmsg(msg, size) Serial.print(0, BYTE);Serial.print(size, BYTE);Serial.print(msg);
#else
#define debugmsg(msg, size)
#endif // def DEBUG

#include "TimerOne.h" // using Timer1 library from http://www.arduino.cc/playground/Code/Timer1

#define DATAOUT 11//MOSI
#define DATAIN  12//MISO 
#define SPICLOCK  13//sck
#define SLAVESELECT 10//ss
#define RST 7//reset to center

uint8_t action_is[256]; // current state
uint16_t action_want[256]; // requested state
uint8_t buffer[3]; // global packet storage (1st BYTE: N_action, 2nd BYTE: value, 3rd BYTE: check summ)
int bufferN = 0; // received bytes counter
int ledPin = 6; // led pin
bool ledState = false;
int zeroCounter = 0; // if we got 000 at serial port than reset communication
bool fBeat = false; // heatbeat flag
int rTimeout = 0; // serial port last read timeout seconds

void setup() {
    byte clr;

    /*
     * initialize serial communication
     */
    Serial.begin(115200);
    /*
     * clear buffer
     */
    for (int i=0; i<256; i++) {
        action_is[i] = 0;
        action_want[i] = 0;
    };
    /*
     * initialize led
     */
    pinMode(ledPin, OUTPUT);
    /*
     * initialize SPI
     */
    pinMode(DATAOUT, OUTPUT);
    pinMode(DATAIN, INPUT);
    pinMode(SPICLOCK,OUTPUT);
    pinMode(SLAVESELECT,OUTPUT);
    pinMode(RST,OUTPUT);
    //digitalWrite(RST,LOW);//reset reostat
    digitalWrite(RST,HIGH);
    digitalWrite(SLAVESELECT,HIGH); //disable device
    // SPCR = 01010000
    //interrupt disabled,spi enabled,msb 1st,master,clk low when idle,
    //sample on leading edge of clk,system clock/4 rate (fastest)
    SPCR = (1<<SPE)|(1<<MSTR);
    clr=SPSR;
    clr=SPDR;
    /*
     * initialize timer
     */
    Timer1.initialize(1000); // 1ms = 1000 microseconds
    Timer1.attachInterrupt(beat);
}

char spi_transfer(volatile char data)
{
    SPDR = data;                    // Start the transmission
    while (!(SPSR & (1<<SPIF))) {}; // Wait the end of the transmission
    return SPDR;                    // return the received byte
}

void resetAnalog() {
}

void setAnalog(uint8_t action, uint8_t value) {
    switch(action) {
        case 1: // x
            value += 128;
            write_pot(0, (unsigned int)value);
            break;
        case 2: // y
            value += 128;
            write_pot(1, (unsigned int)value);
            break;
        default:
            break;
    };
    
}

byte write_pot(int address, int value) {
#ifdef DEBUG
    char msg[256];
    int size = sprintf(msg, "write_pot\tadress %u value %u", address, value);
    debugmsg(msg, size);
#endif // def DEBUG
    digitalWrite(SLAVESELECT,LOW);
    //2 byte opcode
    spi_transfer(address);
    spi_transfer(value);
    digitalWrite(SLAVESELECT,HIGH); //release chip, signal end transfer
}

void loop() {
    if (fBeat && bufferN == 0 || fBeat && rTimeout > 1) {
        fBeat    = false; // turn off heatbeat flag
        rTimeout = 0; // reset read timeout
        bufferN  = 0; // reset buffer pointer
        Serial.print(0, BYTE); // debug heatbeat
        Serial.print(0, BYTE);
    };
    /*
     * read
     */
    if (Serial.available() > 0) {
        buffer[bufferN++] = Serial.read();
        rTimeout = 0; // reset read timeout
    };
    /*
     * process
     */
    if (bufferN>2) {
        // action waiting in buff
        bufferN = 0;
        if ((buffer[0] ^ buffer[1]) == buffer[2]) {
            uint8_t rchecksumm = (uint8_t)buffer[0] + (uint8_t)buffer[1] + (uint8_t)buffer[2];
            if (rchecksumm == 0)
                rchecksumm = 1;
            Serial.print(rchecksumm, BYTE);
#ifdef DEBUG
            // set action in buffer
            char msg[256];
            int size = sprintf(msg, "loop\tset action_want %u value %u", buffer[0], buffer[1]);
            debugmsg(msg, size);
#endif // def DEBUG
            action_want[buffer[0]] = 0xFF + buffer[1];
        } else {
            // check summ failed
            Serial.print(buffer[0] + buffer[1] + buffer[2], BYTE);
            Serial.read(); // shift when checksumm failed
        };
    };
}

void beat() {
    static int counter = 0;
    counter++;
    if (counter == 1000) { // 1 second
        fBeat = true; // debug heatbeat
        rTimeout++;
        counter = 0;
    };
    for (int i = 0; i < 256; i++) {
        // is analog?
        switch(i) {
            // analog
            case 0: // reserverd for internal use
            case 1: // x1
            case 2: // y1
            case 3: // x2
            case 4: // y2
                //resetAnalog(i); // TODO!!!
                if (action_want[i] != 0) {
                    action_is[i] = action_want[i] - 0xFF;
                    setAnalog(i, action_is[i]);
                } else {
                    action_is[i] = 0;
                };
                // reset analog state in action_want
                action_want[i] = 0;
                break;
            // digital
            default:
                if (action_want[i] != action_is[i]) {
                    action_is[i] = action_want[i] - 0xFF;
                    setDigital(i, action_is[i]);
                };
                // keep digital state in action_want
                break;
        };
    };
}

void setDigital(uint8_t action, uint8_t value) {
    switch(action) {
        case 5: // led
            if (value) {
                digitalWrite(ledPin, HIGH);
            } else {
                digitalWrite(ledPin, LOW);
            };
            break;
        default:
            break;
    };

}

// vim:ft=c:ts=4:sw=4
