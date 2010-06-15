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

#define DATAOUT 11//MOSI
#define DATAIN  12//MISO 
#define SPICLOCK  13//sck
#define SLAVESELECT 10//ss
#define RST 7//reset to center

uint8_t action_is[256]; // current state
uint8_t action_want[256]; // requested state
uint8_t buffer[2]; // global packet storage (1st BYTE: N_action, 2nd BYTE: value, 3rd BYTE: check summ)
int bufferN = 0; // received bytes counter

void setup() {
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
     * initialize SPI
     */
    pinMode(DATAOUT, OUTPUT);
    pinMode(DATAIN, INPUT);
    pinMode(SPICLOCK,OUTPUT);
    pinMode(SLAVESELECT,OUTPUT);
    pinMode(RST,OUTPUT);
    //digitalWrite(RST,LOW);//reset reostat
    dgitalWrite(RST,HIGH);
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
}

void setDigital(uint8_t action, uint8_t value) {
}

void loop() {
    /*
     * read
     */
    if (Serial.available() > 0) {
        buffer[bufferN++] = Serial.read();
    };
    /*
     * process
     */
    if (bufferN>2) {
        // action waiting in buff
        bufferN = 0;
        if ((buffer[0] ^ buffer[1]) == buffer[2]) {
            // check summ ok
            /*
             * write
             */
            for (int i = 0; i < 3; i++) {
                Serial.print(buffer[i], BYTE);
            };
            // set action in buffer
            action_want[buffer[0]] = buffer[1];
        } else {
            // check summ failed
            /*
             * write
             */
            for (int i = 0; i < 3; i++) {
                Serial.print(0, BYTE);
            };
            Serial.read(); // shift when checksumm failed
        };
    };
}

void beat() {
    for (int i = 0; i < 256; i++) {
        // is analog?
        switch() {
            // analog
            case 0: // x1
            case 1: // y1
            case 2: // x2
            case 3: // y2
                //resetAnalog(i); // TODO!!!
                if (action_want[i] != 0) {
                    action_is[i] = action_want[i];
                    //setAnalog(i, action_is[i]) // TODO!!!
                } else {
                    action_is[i] = 0;
                };
                // reset analog state in action_want
                action_want[i] = 0;
                break;
            // digital
            default:
                if (action_want[i] != action_is[i]) {
                    action_is[i] = action_want[i];
                    //setDigital(i, action_is[i]); // TODO!!!
                };
                // keep digital state in action_want
                break;
        };
    };
}
// vim:ft=c:ts=4:sw=4
