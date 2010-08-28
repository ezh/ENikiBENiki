/***************************************************************************
 * Copyright (C) 2010 Alexey Aksenov, Alexx Fomichew                       *
 * Alexey Aksenov (ezh at ezh.msk.ru) software, firmware                   *
 * Alexx Fomichew (axx at fomichi.ru) hardware                             *
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

#include "TimerOne.h" // using Timer1 library from http://www.arduino.cc/playground/Code/Timer1

#define DEBUG

#ifdef DEBUG
#define dprintf(...) _dprintf(__VA_ARGS__)
#else
#define dprintf(...)
#endif // def DEBUG*/

#define DATAOUT 11 // MOSI
#define DATAIN  12  // MISO 
#define SPICLOCK  13 // CLOCK
#define SLAVESELECT_A 10 // SS
#define SLAVESELECT_B 9 // SS
#define ACTIONS 64 // maximum actions for controller
/*
 * commands 0 n
 */
#define CMD_RESET 1
#define CMD_SETBASE0 2 // next message set base level [0]
#define CMD_SETBASE1 3 // next message set base level [1]
#define CMD_SETBASE2 4 // next message set base level [2]
#define CMD_SETBASE3 5 // next message set base level [3]
#define CMD_SETBASE4 6 // next message set base level [4]
#define CMD_SETBASE5 7 // next message set base level [5]
#define CMD_SETBASE6 8 // next message set base level [6]
#define CMD_SETBASE7 9 // next message set base level [7]
#define CMD_SETBASE8 10 // next message set base level [8]
/*
 * hardware configurations
 */
#define HW_UNKNOWN 0
#define HW_LAYOUT_A 1 // two MAX395 and one AD8402 at SLAVESELECT_A + AD8402 at SLAVESELECT_B for XBOX360
#define HW_LAYOUT_B 2 // who knows?
byte baselevel[8] = {128, 128, 128, 128, 128, 128, 128, 128}; // values for analog axis
byte actionActive[ACTIONS]; // current state
byte actionWant[ACTIONS*2]; // requested state
byte buffer[3]; // global packet storage (1st BYTE: N_action, 2nd BYTE: value, 3rd BYTE: check summ)
byte bufferN = 0; // received bytes counter
byte zeroN = 0; // received zeros counter if we got 000 at serial port than reset communication
int rTimeout = 0; // serial port last read timeout
bool fBeat = false; // heatbeat flag

int hconfig = HW_LAYOUT_A;

void _dprintf(const char *format, ...) {
    char buffer[256];
    va_list args;           //variable args
    va_start(args, format); //variable l-ist
    buffer[0] = 0;
    int size = vsprintf(buffer+2, format, args);
    buffer[1] = (byte)size;
    va_end(args);
    Serial.write((uint8_t*)buffer, size+2);
};

char spi_transfer(volatile char data) {
    SPDR = data;                    // Start the transmission
    while (!(SPSR & (1<<SPIF))) {}; // Wait the end of the transmission
    return SPDR;                    // return the received byte
}

void setup() {
    byte clr;

    /*
     * initialize serial communication
     */
    Serial.begin(115200);
    /*
     * clear buffers
     */
    for (int i=0; i<ACTIONS; i++) {
        actionActive[i] = 0;
        actionWant[i] = 0;
    };
    /*
     * initialize SPI
     */
    pinMode(DATAOUT, OUTPUT);
    pinMode(DATAIN, INPUT);
    pinMode(SPICLOCK,OUTPUT);
    pinMode(SLAVESELECT_A,OUTPUT);
    pinMode(SLAVESELECT_B,OUTPUT);
    digitalWrite(SLAVESELECT_A,HIGH); //disable device
    digitalWrite(SLAVESELECT_B,HIGH); //disable device
    SPCR = (1<<SPE)|(1<<MSTR);
    clr=SPSR;
    clr=SPDR;
    /*
     * initialize timer
     */
    Timer1.initialize(1000); // 1ms = 1000 microseconds
    Timer1.attachInterrupt(beat);
}

void beat() {
    Timer1.detachInterrupt();
    static int counter = 0;
    counter++;
    if (counter == 1000) { // 1 second
        fBeat = true; // debug heatbeat
        rTimeout++;
        counter = 0;
    };
    for (int i = 0; i < ACTIONS; i++) {
        if (actionWant[i*2] != 0) {
            // PREPROCESS TODO?!
            /*
             * FIRE!
             */
            if (actionActive[i] != actionWant[i*2+1]) {
                // set action in buffer
                actionActive[i] = actionWant[i*2+1];
                dprintf("beat\tget actionWant %u value %u:%u", i, actionWant[i*2], actionWant[i*2+1]);
                fire(i);
            } else {
                dprintf("beat\tskip actionWant %u value %u:%u", i, actionWant[i*2], actionWant[i*2+1]);
            };
        };
        // reset state in actionWant
        actionWant[i*2] = 0;
        actionWant[i*2+1] = 0;
    };
    Timer1.attachInterrupt(beat);
}

void loop() {
    static byte expectMessage = 0; // for command more than 1 message 
    noInterrupts(); // critical section
    /*
     * heatbeat
     */
    if (fBeat && bufferN == 0 || fBeat && rTimeout > 1) {
        fBeat    = false; // turn off heatbeat flag
        rTimeout = 0; // reset read timeout
        
        bufferN  = 0; // reset buffer pointer
        Serial.print(0, BYTE); // heatbeat
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
            Serial.print(rchecksumm, BYTE); // reply
            /*
             * process actions
             */
            if (expectMessage == 0) {
                if (buffer[0] == 0) {
                    if (buffer[1] == CMD_RESET) {
                        // reset
                        dprintf("reset");
                        for (int i=0; i<ACTIONS; i++) {
                            actionActive[i] = 0;
                            actionWant[i] = 0;
                        };
                        if (hconfig == HW_LAYOUT_A) {
                            dprintf("reset HW_LAYOUT_A");
                            LAYOUT_A(0, 0, 0, baselevel[4], 0, baselevel[0]); // reset buttons, LT and X1
                            LAYOUT_A(0xFFFF, 0xFFFF, 1, baselevel[5], 1, baselevel[1]); // reset RT and Y1
                            LAYOUT_A(0xFFFF, 0xFFFF, 0xFF, 0xFF, 2, baselevel[2]); // reset X2
                            LAYOUT_A(0xFFFF, 0xFFFF, 0xFF, 0xFF, 3, baselevel[3]); // reset Y2
                        };
                    } else if (buffer[1] >= CMD_SETBASE0 and buffer[1] <= CMD_SETBASE8) {
                        expectMessage = buffer[1];
                    };
                } else {
                    // set action in buffer
                    actionWant[(buffer[0]-1)*2]++; // 2*N (request counter)
                    actionWant[(buffer[0]-1)*2+1] = buffer[1]; // 2*N+1 (value)
                    dprintf("loop\tset actionWant %u value %u:%u", buffer[0]-1, actionWant[(buffer[0]-1)*2], actionWant[(buffer[0]-1)*2+1]);
                };
            } else {
                if (expectMessage >= CMD_SETBASE0 and expectMessage <= CMD_SETBASE8) {
                    dprintf("set baselevel[%u]: %u", expectMessage-2, buffer[1]);
                    baselevel[expectMessage-2] = buffer[1];
                    expectMessage = 0;
                };
            };
        } else {
            // check summ failed
            Serial.print(buffer[0] + buffer[1] + buffer[2], BYTE);
            Serial.read(); // shift when checksumm failed
        };
    };
    interrupts();
}

void LAYOUT_A(word maxVal1 = 0xFFFF, word maxVal2 = 0xFFFF, byte ad84Addr1 = 0xFF, byte ad84Val1 = 0, byte ad84Addr2 = 0xFF, byte ad84Val2 = 0);

void fire(byte n) {
    if (hconfig == HW_LAYOUT_A) {
        if (n >= 0 and n < 10) {
            // AXIS ABSOLUTE PERMANENT
            if (n == 0) {
                LAYOUT_A(0xFFFF, 0xFFFF, 0xFF, 0, 0, actionActive[n]);
            } else if (n == 1) {
                LAYOUT_A(0xFFFF, 0xFFFF, 0xFF, 0, 1, actionActive[n]);
            } else if (n == 2) {
                LAYOUT_A(0xFFFF, 0xFFFF, 0xFF, 0, 2, actionActive[n]);
            } else if (n == 3) {
                LAYOUT_A(0xFFFF, 0xFFFF, 0xFF, 0, 3, actionActive[n]);
            } else if (n == 4) {
                LAYOUT_A(0xFFFF, 0xFFFF, 0, actionActive[n]); // analog button
            } else if (n == 5) {
                LAYOUT_A(0xFFFF, 0xFFFF, 1, actionActive[n]); // analog button
            };
        } else if (n >= 10 and n < 20) {
            // AXIS ABSOLUTE 1MS
            if (n == 0) {
                LAYOUT_A(0xFFFF, 0xFFFF, 0xFF, 0, 0, actionActive[n]);
            } else if (n == 1) {
                LAYOUT_A(0xFFFF, 0xFFFF, 0xFF, 0, 1, actionActive[n]);
            } else if (n == 2) {
                LAYOUT_A(0xFFFF, 0xFFFF, 0xFF, 0, 2, actionActive[n]);
            } else if (n == 3) {
                LAYOUT_A(0xFFFF, 0xFFFF, 0xFF, 0, 3, actionActive[n]);
            } else if (n == 4) {
                LAYOUT_A(0xFFFF, 0xFFFF, 0, actionActive[n]); // analog button
            } else if (n == 5) {
                LAYOUT_A(0xFFFF, 0xFFFF, 1, actionActive[n]); // analog button
            };
        } else if (n >= 20 and n < ACTIONS) {
            // BUTTONS
            if (n >= 20 and n < 28) {
                word current = 0;
                bitSet(current, n-20); // invert bit, TODO: check to skip
                LAYOUT_A((current+0xFF), 0xFFFF);
            } else if (n >= 28 and n < 38) {
                word current = 0;
                bitSet(current, n-28); // invert bit, TODO: check to skip
                LAYOUT_A(0xFFFF, (current+0xFF));
            };
        };
    };
}

void LAYOUT_A(word maxVal1, word maxVal2, byte ad84Addr1, byte ad84Val1, byte ad84Addr2, byte ad84Val2) {
    static byte lastMaxVal1 = 0;
    static byte lastMaxVal2 = 0;
    static byte lastAd84Addr1 = 0;
    static byte lastAd84Val1 = 0;
    static byte lastAd84Addr2 = 0;
    static byte lastAd84Val2 = 0;
    byte buffer[5] = {0, 0, 0, 0, 0};

    // check maxVals
    if (maxVal1 == 0xFFFF) {
        maxVal1 = lastMaxVal1; // skip
    } else if (maxVal1 <=0xFF) {
        lastMaxVal1 = maxVal1; // new absolute value
    } else {
        lastMaxVal1 ^= (maxVal1-0xFF); // from 0xFF01 to 0xFFFF, relative value; XOR with last
    };
    if (maxVal2 == 0xFFFF) {
        maxVal2 = lastMaxVal2; // skip
    } else if (maxVal2 <=0xFF) {
        lastMaxVal2 = maxVal2; // new absolute value
    } else {
        lastMaxVal2 ^= (maxVal2-0xFF); // from 0xFF01 to 0xFFFF, relative value; XOR with last
    };
    // check ad84
    if (ad84Addr1 < 2) {
        lastAd84Addr1 = ad84Addr1;
        lastAd84Val1   = ad84Val1;
    };
    if (ad84Addr2 < 4) {
        lastAd84Addr2 = ad84Addr2;
        lastAd84Val2 = ad84Val2;
    };
    // shift bytes
    buffer[0] = (lastAd84Addr1 << 2)|(lastAd84Val1 >> 6);
    buffer[1] = (lastAd84Val1 << 2)|(lastAd84Addr2);
    buffer[2] = lastAd84Val2;
    buffer[3] = lastMaxVal1;
    buffer[4] = lastMaxVal2;
    dprintf("LAYOUT_A set maxVal1:%u, maxVal2:%u, ad84Addr1:%u, ad84Val1:%u, ad84Addr2:%u, ad84Val2:%u",
            lastMaxVal1, lastMaxVal2, lastAd84Addr1, lastAd84Val1, lastAd84Addr2, lastAd84Val2);
    dprintf("LAYOUT_A set H buffer[0]:%u < buffer[1]:%u < buffer[2]:%u < buffer[3]:%u < buffer[4]:%u T",
            buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]);
    digitalWrite(SLAVESELECT_A,LOW);
    spi_transfer(buffer[0]);
    spi_transfer(buffer[1]);
    spi_transfer(buffer[2]);
    spi_transfer(buffer[3]);
    spi_transfer(buffer[4]);
    digitalWrite(SLAVESELECT_A,HIGH);
}

// vim:ft=c:ts=4:sw=4
