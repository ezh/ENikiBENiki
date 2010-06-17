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

#include "Controller.h"

#define new PNEW

ControllerThread::ControllerThread(PSerialChannel * tserial) : PThread(10000, NoAutoDeleteThread), queue(1000) {
    int i;

    PTRACE(4, "Constructor");
    pserial = tserial;
    for(i = 0; i < 256; i++) {
        action[i] = new PWORDArray();
    };
    queue.SetReadTimeout(0); // timeout 0 ms
    Resume();
}

ControllerThread::~ControllerThread() {
    unsigned char i;

    PTRACE(4, "Destructor");
    for(i = 0; i == 255; i++) {
        delete action[i];
    };
}

void ControllerThread::Main() {
    PTime tBase; // base time for tStep multiplier
    PTimeInterval tStep(1); // 1ms, loop step (1Hz); up to 14 bytes per step for 115200 serial line
    PTime tNow; // current time
    PTime tThen; // expected execution time
    unsigned short i = 0; // multiplier for tStep

    do {
        unsigned char buffer[3] = {0, 0, 0};
        bool fWork = false;
        /*
         * get x, y, button and other events
         */
        PTRACE(6, "Main\tactions before population:"); dumpAction();
        while(popAction(buffer)) {
            PWORDArray * actions;
            WORD value;

            fWork = true;
            actions = action[buffer[0]];
            value = buffer[1] << 8;
            value += buffer[2];
            PTRACE(6, "Main\tadd action: " << (int)buffer[0] << " for buffer with size " << actions->GetSize());
            actions->SetAt(actions->GetSize(), value);
        };
        if (fWork) {
            /*
             * process actions
             */
            PTRACE(6, "Main\tactions after population:"); dumpAction();
            summarizeActions(); // summarize action events by type
            PTRACE(6, "Main\tactions after summarization:"); dumpAction();
            /*
             * send current state to arduino
             * then receive state from arduino
             * and update actions after sucsessful transmit
             */
            processActions();
        } else {
            PTRACE(7, "Main\tskip step - no action events");
        };
        /*
         * wait next 10ms
         */
        i++;
        tThen = tBase + tStep * i;
        tNow = PTime();
        // reset multiplier
        if (i >= 255) {
            i = 0;
            tBase = tThen;
        };
        // step was too long (tThen less than tNow)
        if (tNow.Compare(tThen) != LessThan) {
            PTRACE(6, "Main\tnow: " << tNow.AsString("h:m:s.uuuu") << " then: " << tThen.AsString("h:m:s.uuuu") << " i: " << (int)i << " diff: " << (tNow - tThen).GetMilliSeconds() << "ms");
            i += (tNow - tThen).GetMilliSeconds() / tStep.GetMilliSeconds() + 1; // number of steps + 1 step
            tThen = tBase + tStep * i;
            PTRACE(6, "Main\tcorrected then: " << tThen.AsString("h:m:s.uuuu") << " i: " << (int)i);
        };
        PTRACE(7, "Main\tstep " << (tThen - tNow).GetMilliSeconds() << "ms"); 
    } while(!shutdown.Wait((tThen - tNow).GetMilliSeconds()));
}

void ControllerThread::Stop() {
    // signal the shutdown PSyncPoint. On the next iteration, the thread's
    // Main() function will exit cleanly.
    shutdown.Signal();
}

bool ControllerThread::pushAction(BYTE action, WORD value) {
    BYTE buffer[2] = {0, 0};

    buffer[0] = value >> 8;
    buffer[1] = value & 0xff;
    return pushAction(action, buffer[0], buffer[1]);
}

bool ControllerThread::pushAction(BYTE action, BYTE value1, BYTE value2) {
    unsigned char buffer[3] = {0, 0, 0};

    buffer[0] = action;
    buffer[1] = value1;
    buffer[2] = value2;
    PTRACE(5, "pushAction\twriting buffer");
    if (!queue.Write(buffer, sizeof(buffer))) {
        PTRACE(1, "pushAction\twrite failed");
        return false;
    };
    return true;
}

bool ControllerThread::popAction(BYTE buffer[3]) {
    if (!queue.Read(buffer, sizeof(buffer))) {
        return false;
    };
    return true;
}

void ControllerThread::dumpAction() {
    int i;

    for(i = 0; i < 256; i++) {
        int actionSize = action[i]->GetSize(); 
        if (actionSize>0) {
            int j;
            // WORD(BYTE/BYTE)
            PStringStream values;
            WORD word  = action[i]->GetAt(0);
            BYTE byte1 = word >> 8;
            BYTE byte2 = word & 0xff;

            values << (int)word << "(" << (int)byte1 << "/" << (int)byte2 << ")";
            for(j = 1; j < actionSize; j++) {
                word  = action[i]->GetAt(j);
                byte1 = word >> 8;
                byte2 = word & 0xff;
                values << "," << (int)word << "(" << (int)byte1 << "/" << (int)byte2 << ")";
            };
            PTRACE(5, i << ": " << values);
        };
    };
}

void ControllerThread::summarizeActions() {
    int i;

    for(i = 0; i < 256; i++) {
        int actionSize = action[i]->GetSize(); 
        if (actionSize>1) {
            int j;
            switch(i) {
                case 0:
                case 1:
                    // summarize relative value x/y
                    for(j = actionSize; j > 0; j--) {
                        signed short old = action[i]->GetAt(0);
                        signed short add = action[i]->GetAt(j);
                        action[i]->SetAt(0, old+add);
                        action[i]->SetSize(j);
                    };
                    break;
                case 2:
                case 3:
                    // replace first absolute value x/y with last
                    action[i]->SetAt(0, action[i]->GetAt(j));
                    for(j = actionSize; j > 0; j--) {
                        action[i]->SetSize(j);
                    };
                    break;
                default:
                    // keep first 3 digital values
                    for(j = actionSize; j > 2; j--) {
                        action[i]->SetSize(j);
                    };
                    break;
            };
        };
    };
}

void ControllerThread::processActions() {
    int messageMax = 256;
    char buffer[messageMax];
    PBYTEArray message(4);

    // compose send buffer
    for(int i = 0; i < 256; i++) {
        int actionSize = action[i]->GetSize();
        if (actionSize>0) {
            bool retransmit = false;

            message[0] = i; // action
            message[1] = action[i]->GetAt(0) >> 8; // 1st byte
            message[2] = action[i]->GetAt(0) & 0xff; // 2nd byte
            message[3] = message[0] ^ message[1] ^ message[2]; // check summ
            do {
                PINDEX len = 0;
                /*
                 * send action
                 */
                PTRACE(1, "processActions\tSend query message (hex) "
                        << psprintf("%02x,%02x,%02x,%02x", (BYTE)message[0], (BYTE)message[1], (BYTE)message[2], (BYTE)message[3])
                        << " to the serial port");
                pserial->Write(message.GetPointer(), message.GetSize());
                /*
                 * receive reply from confroller
                 */
                memset(buffer, 0, messageMax);
                do {
                    pserial->Read(buffer, messageMax);
                    len += pserial->GetLastReadCount();
                } while(len < 4);
                /*
                 * process reply
                 */
                if (len == 4) {
                    PTRACE(1, "processActions\tReceive reply message (hex) "
                            << psprintf("%02x,%02x,%02x,%02x", (BYTE)buffer[0], (BYTE)buffer[1], (BYTE)buffer[2], (BYTE)buffer[3])
                            << " from the serial port");
                    if (buffer[0] == message[0] && buffer[1] == message[1] && buffer[2] == message[2] && buffer[3] == message[3]) {
                        retransmit = false;
                        // transmit was sucsessful, drop 1st action from array
                        for(int j = 0; j < actionSize; j++) {
                            action[i]->SetAt(j, action[i]->GetAt(j+1));
                        };
                        action[i]->SetSize(actionSize-1);
                    } else {
                        retransmit = true;
                        PTRACE(1, "retransmit, receive zero reply");
                    };
                } else {
                    retransmit = true;
                    PTRACE(1, "retransmit, receive broken reply, len: " << (int)len);
                };
            } while(retransmit);
        };
    };
}

/*        if (len != 0) {
            buffer[len] = 0;
            PTRACE(1, "Read the string \"" << buffer << "\" from the serial port");
            str += PString(buffer);
            if (str.Find("\n") != P_MAX_INDEX)
                found = PTrue;
        }
        PINDEX err = serial.GetErrorCode();
        if ((err != PChannel::NoError) && (err != PChannel::Timeout)) {
            PTRACE(1, "get data from serial port, failed, error is " << serial.GetErrorText());
            cout << "get data from serial port, failed, error is " << serial.GetErrorText() << endl;
        }
        if (found) {
            str.Replace("\n", "");
            PTRACE(1, "Read the message \"" << str << "\"");
            cout << "have read the message \"" << str << "\" from the serial port" << endl;
            str = "";
            found = PFalse;
        }*/

// End of File ///////////////////////////////////////////////////////////////
// vim:ft=c:ts=4:sw=4
