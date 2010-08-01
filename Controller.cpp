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

ControllerThread::ControllerThread(PSerialChannel * tserial) : PThread(10000, NoAutoDeleteThread), queue() {
    PTRACE(4, "Constructor");
    pserial = tserial;
    for(int i = 0; i < 256; i++) {
        actionQueuePool[i] = new PWORDArray();
    };
    queue.Open(10000);
    queue.SetReadTimeout(0); // timeout 0 ms
    queue.SetWriteTimeout(0); // timeout 0 ms
    timeout = 100;
    retryLimit = 5;
    fReady = PFalse;
    Resume();
}

ControllerThread::~ControllerThread() {
    PTRACE(4, "Destructor");
    for(int i = 0; i < 256; i++) {
        delete actionQueuePool[i];
    };
    queue.Close();
}

void ControllerThread::Main() {
    PTime tBase; // base time for tStep multiplier
    PTimeInterval tStep(1); // 1ms, loop step (1Hz); up to 14 bytes per step for 115200 serial line
    PTime tNow; // current time
    PTime tThen; // expected execution time
    unsigned short i = 0; // multiplier for tStep

    // initialize
    PTRACE(1, "Arduino initialization");
    char buffer[256];
    PINDEX iRead = 0;
    PINDEX len   = 0;
    // reset serial port
    pserial->ClearDTR();
    pserial->ClearRTS();
    pserial->ClearBreak();
    pserial->SetDTR();
    pserial->SetRTS();
    do {
        if (shutdown.Wait(0)) {
            return;
        };
        pserial->Read(buffer, 256); // flush serial data
    } while (pserial->GetLastReadCount());
    memset(buffer, 0, 256);
    // waiting for heat beat
    do {
        if (shutdown.Wait(0)) {
            return;
        };
        pserial->Read(buffer, 256);
        iRead = pserial->GetLastReadCount();
        if (iRead == 2 && buffer[0] == 0 && buffer[1] == 0) {
            len += iRead;
            PTRACE(1, "HeatBeat from Arduino #" << len/2);
        };
    } while(len < 6);
    PTRACE(1, "Arduino initialization done");
    fReady = PTrue;


    // loop
    do {
        bool fNewActions = false;
        BYTE naction;
        WORD value;
        /*
         * get x, y, button and other events
         */
        PTRACE(6, "Main\t" << dumpAction("actions before population: "));
        while(popAction(&naction, &value)) {
            PWORDArray *actionQueue = actionQueuePool[naction];
            
            fNewActions = true;
            PTRACE(6, "Main\tadd new value to actionQueuePool[" << (int)naction << "] with queue size " << actionQueue->GetSize());
            actionQueue->SetAt(actionQueue->GetSize(), value);
        };
        if (fNewActions) {
            PTRACE(6, "Main\t" << dumpAction("actions after population: "));
            summarizeActions(); // summarize action events by type
            PTRACE(6, "Main\t" << dumpAction("actions after summarization: "));
        };
        /*
         * process actions:
         * send current state to arduino
         * then receive state from arduino
         * and update actions after sucsessful transmit
         */
        processActions();
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
    BYTE buffer[3] = {0, 0, 0};
    buffer[0] = action;
    buffer[1] = (BYTE)value >> 8;
    buffer[2] = (BYTE)value;
    PTRACE(5, "pushAction\twriting buffer " <<
            psprintf("%02x,%02x,%02x", (BYTE)buffer[0], (BYTE)buffer[1], (BYTE)buffer[2]));
    if (!queue.Write(buffer, sizeof(buffer))) {
        PError << "pushAction\twrite failed" << endl;
        return false;
    };
    return true;
}

bool ControllerThread::popAction(BYTE* action, WORD* value) {
    BYTE buffer[3] = {0, 0, 0};
    if (!queue.Read(buffer, 3)) {
        return PFalse;
    };
    *action = buffer[0];
    *value = (((WORD)buffer[1]) << 8) | buffer[2];
    PTRACE(5, "popAction\treading buffer " <<
            psprintf("%02x,%02x,%02x", (BYTE)buffer[0], (BYTE)buffer[1], (BYTE)buffer[2]));
    return PTrue;
}

PString ControllerThread::dumpAction(const char * szHeader) {
    PStringStream streamValues;
    bool fEmpty = true;

    streamValues << szHeader;
    for(int i = 0; i < 256; i++) {
        if (!actionQueuePool[i]->IsEmpty()) {
            int actionSize = actionQueuePool[i]->GetSize(); 
            BYTE value  = actionQueuePool[i]->GetAt(0);
            streamValues << endl << "actions[" << i << "]: " << (int)value;
            for(int j = 1; j < actionSize; j++) {
                value  = actionQueuePool[i]->GetAt(j);
                streamValues << "," << (int)value;
            };
            fEmpty = false;
        };
    };
    if (fEmpty) {
        streamValues << "empty";
    };
    return streamValues;
}

void ControllerThread::summarizeActions() {
    for(int i = 0; i < 256; i++) {
        int actionSize = actionQueuePool[i]->GetSize(); 
        if (actionSize>1) {
            switch(i) {
                case 0:
                case 1:
                case 2:
                case 3:
                    // replace first absolute value x/y with last
                    actionQueuePool[i]->SetAt(0, actionQueuePool[i]->GetAt(actionSize));
                    actionQueuePool[i]->SetSize(1);
                    break;
                default:
                    // keep first 3 digital values
                    if (actionSize>3) {
                        actionQueuePool[i]->SetSize(3);
                    };
                    break;
            };
        };
    };
}

void ControllerThread::processActions() {
    PBYTEArray message(3);

    for(int i = 0; i < 256; i++) {
        int actionSize = actionQueuePool[i]->GetSize();
        if (actionSize>0) {
            message[0] = i; // action
            message[1] = actionQueuePool[i]->GetAt(0); // value
            message[2] = message[0] ^ message[1]; // check summ

            // transmit information to controller
            if (!processTransmit(message.GetPointer(), message.GetSize())) {
                PError << "ControllerThread::processActions() data transmit failed" << endl;
                return;
            };
            // receive information from controller
            if (!processReceive((BYTE)message[0] + (BYTE)message[1] + (BYTE)message[2], PFalse)) {
                PError << "ControllerThread::processActions() data receive failed" << endl;
                return;
            };
            // conversation was successful, drop 1st action from array
            for(int j = 0; j < actionSize; j++) {
                actionQueuePool[i]->SetAt(j, actionQueuePool[i]->GetAt(j+1));
            };
            actionQueuePool[i]->SetSize(actionSize-1);
        };
    };
    processReceive(00, PTrue);
}

// transmit
bool ControllerThread::processTransmit(const unsigned char* message, PINDEX length) {
    bool error = PFalse;
    int retry = 0;

    do {
        // send information to controller
        PTRACE(3, "processActions\tSend query message (hex) "
                << psprintf("%02x,%02x,%02x", (BYTE)message[0], (BYTE)message[1], (BYTE)message[2])
                << " to the serial port");
        if (!pserial->Write(message, length)) {
            PError << "write data to serial port failed, error is " << pserial->GetErrorText() << endl;
            error = PTrue;
            retry++;
        };
    } while(error && retry <= retryLimit);
    if (retry > retryLimit) {
        return PFalse;
    } else {
        return PTrue;
    };
}

// receive
bool ControllerThread::processReceive(BYTE expect, bool peek) {
    const int messageMax = 1024;
    BYTE buffer[messageMax];

    if (expect == 0)
        expect = 1; // shift control summ
    if (!peek) {
        PTRACE(2, "processReceive\tWaiting for: " << psprintf("%02x", (BYTE)expect));
    };
    do {
        if (peek) {
            PTimeInterval readTimeout = pserial->GetReadTimeout();
            pserial->SetReadTimeout(1);
            pserial->Read(buffer, 1);
            pserial->SetReadTimeout(readTimeout);
        } else {
            pserial->Read(buffer, 1);
        };
        if (pserial->GetLastReadCount() == 1) {
            if (buffer[0] == expect) {
                PTRACE(2, "processReceive\tCorrect reply: " <<  psprintf("%02x", expect));
                return PTrue;
            } else if (buffer[0] == 0) {
                // message from controller
                pserial->Read(buffer, 1);
                if (pserial->GetLastReadCount() == 1) {
                    if (buffer[0] == 0) {
                        // heatbeat
                        PTRACE(2, "processReceive\tController heatbeat");
                        continue;
                    } else {
                        // message
                        PINDEX length = buffer[0];
                        PStringStream streamValues;
                        do {
                            int read = 0;
                            pserial->Read(buffer, length);
                            read = pserial->GetLastReadCount();
                            length -= read;
                            buffer[read] = '\0';
                            streamValues << buffer;
                        } while(length > 0);
                        PTRACE(2, "Controller message: " << streamValues);
                    };
                };
            } else {
                PTRACE(2, "processReceive\treceive broken reply:" << psprintf("%02x", (BYTE)buffer[0]));
                // flush
                PTimeInterval readTimeout = pserial->GetReadTimeout();
                pserial->SetReadTimeout(1);
                do {
                    pserial->Read(buffer, messageMax); // flush serial data
                } while (pserial->GetLastReadCount());
                pserial->SetReadTimeout(readTimeout);
                return PFalse;
            };
        } else {
            // if must!
            return PFalse;
        };
    } while(PTrue);
    return PFalse;
}

bool  ControllerThread::isReady() {
    return fReady;
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

/*                } else {
            PStringStream streamValues;

            retransmit++;
            for(int i=0; i<len; i++) {
                streamValues << psprintf("%02x/", (BYTE)buffer[i]) << (BYTE)buffer[i] << " ";
            };
            PTRACE(1, "retransmit(" << retransmit << "), receive broken reply, message:" << streamValues);*/
// End of File ///////////////////////////////////////////////////////////////
// vim:ft=c:ts=4:sw=4
