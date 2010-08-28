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

#include "Controller.h"

#define new PNEW

#define POTENTIOMETER_SCALE 256

ControllerThread::ControllerThread(PSerialChannel *_serial, Resources * _resources, PConfig *_config) : PThread(10000, NoAutoDeleteThread), queue() {
    pserial   = _serial;
    resources = _resources;
    config    = _config;
    analogControls = (config->GetString("Gamepad", "AnalogControl", "")).ToUpper().Tokenise(",", PFalse);

    PTRACE(2, "Constructing instance for controller");
    for(int i = 0; i < 256; i++) {
        actionQueuePool[i] = new PIntArray();
    };
    queue.Open(10000);
    queue.SetReadTimeout(0); // timeout 0 ms
    queue.SetWriteTimeout(0); // timeout 0 ms
    timeout = 100;
    retryLimit = 5;
    fReady = PFalse;
    mouseMaximum = config->GetInteger("Mouse", "maximumOffsetPerMillisecond", 30);
    for (PINDEX i = 0; i < 10; i++) {
        calibrationTable[i] = new PIntArray;
    };
    for (PINDEX i = 0; i < analogControls.GetSize(); i++) {
        PStringStream summary; // summary string
        int summaryN = -100000; // last value in calibrationTable
        PString calibrationValuesName("Axis");
        calibrationValuesName += analogControls[i];
        calibrationValuesName += "Motion";
        PStringArray calibrationValues = (config->GetString("Gamepad", calibrationValuesName, "")).ToUpper().Tokenise(";", PFalse);
        if (calibrationValues.GetSize() == 0) {
            // push linear sequence
            for (PINDEX j = 0; j < POTENTIOMETER_SCALE; j++) {
                // from -100.00% to +100.00% we have POTENTIOMETER_SCALE values
                int min = ((float)20000/POTENTIOMETER_SCALE)*j;
                int max = ((float)20000/POTENTIOMETER_SCALE)*(j+1);
                for (PINDEX k = min; k <= max; k++) { // overwrite last value
                    PIntArray *table = calibrationTable[i];
                    table->SetAt(k, j);
                };
            };
        } else {
            bool redo;
            do {
                int previous = 0;
                int current  = 0;
                int next     = 0;
                redo = PFalse;
                for (PINDEX j = 0; j < POTENTIOMETER_SCALE; j++) {
                    previous = current;
                    current  = calibrationValues[j].AsInteger();
                    next     = calibrationValues[j+1].AsInteger();
                    if (previous == 20000 && current == 20000) {
                        break;
                    };
                    if (current > next) {
                        if (next != 0 && next != 10000) {
                            PTRACE(1, "calibration error for axis " << analogControls[i] << " value: " << calibrationValues[j] << " previous:" << previous << " current: " << current << " next: " << next);
                            redo = PTrue;
                            calibrationValues[j] = PString(next);
                            calibrationValues[j+1] = PString(current);
                            break;
                        } else {
                            break;
                        };
                    };
                    // set range
                    for (int k = (current+previous)/2; k<=(next+current/2); k++) {
                        PIntArray *table = calibrationTable[i];
                        table->SetAt(k, j);
                    };
                };
            } while(redo);
            // fix head and tail
            int calibrationMinimum = calibrationTable[i]->GetAt(0)-1 > 0 ? calibrationTable[i]->GetAt(0)-1 : 0;
            calibrationTable[i]->SetAt(0, calibrationMinimum);
            int calibrationMaximum = calibrationTable[i]->GetAt(20000)+1 < POTENTIOMETER_SCALE ? calibrationTable[i]->GetAt(20000)+1 : POTENTIOMETER_SCALE-1;
            calibrationTable[i]->SetAt(20000, calibrationMaximum);

        };
        for(PINDEX j = 0; j<= 20000; j++) {
            if (summaryN < calibrationTable[i]->GetAt(j)) {
                summaryN = calibrationTable[i]->GetAt(j);
                summary << " [" << j << "]:" << calibrationTable[i]->GetAt(j);
            };
        };
        PTRACE(4, "ControllerThread\tcalibration summary for '"<< analogControls[i] << "'" << summary);
    };
    // prepare game table
    for (PINDEX i = 0; i < 10; i++) {
        gameTable[i] = new PIntArray;
    };
    resetGameTable();
    PString zzz("mw2x.z");
    loadGameTableT(zzz);
    // start
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
    
    // drop junk from serial port
    do {
        if (shutdown.Wait(0)) {
            return;
        };
        pserial->Read(buffer, 256); // flush serial data
    } while (pserial->GetLastReadCount());
    memset(buffer, 0, 256);

    // waiting for 3 heat beat
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

    // reset Arduino
    pushAction(0xFF, CMD_RESET);
    pushAction(0xFF, CMD_SETBASE0); // X1
    pushAction(0xFF, calibrationTable[0]->GetAt(10000));
    pushAction(0xFF, CMD_SETBASE1); // Y1
    pushAction(0xFF, calibrationTable[1]->GetAt(10000));
    pushAction(0xFF, CMD_SETBASE2); // X2
    pushAction(0xFF, calibrationTable[2]->GetAt(10000));
    pushAction(0xFF, CMD_SETBASE3); // Y2
    pushAction(0xFF, calibrationTable[3]->GetAt(10000));
    pushAction(0xFF, CMD_SETBASE4); // LT
    pushAction(0xFF, calibrationTable[4]->GetAt(10000));
    pushAction(0xFF, CMD_SETBASE5); // RT
    pushAction(0xFF, calibrationTable[5]->GetAt(10000));
    pushAction(0xFF, CMD_RESET);
    // main loop
    do {
        bool fNewActions = false;
        BYTE naction;
        PInt32l value;
        /*
         * get x, y, button and other events
         */
        PTRACE(6, "Main\t" << dumpAction("actions before population: "));
        while(popAction(&naction, &value)) {
            PIntArray *actionQueue = actionQueuePool[naction];
            
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

bool ControllerThread::pushAction(BYTE action, PInt32l value) {
    BYTE buffer[5] = {
        action,
        (BYTE)(value >> 24),
        (BYTE)(value >> 16),
        (BYTE)(value >> 8),
        (BYTE)value
    };
    PTRACE(5, "pushAction\twriting buffer " <<
            psprintf("%02x,(%02x%02x%02x%02xh OR %i OR unsinged %u)", (BYTE)buffer[0], (BYTE)buffer[1], (BYTE)buffer[2], (BYTE)buffer[3], (BYTE)buffer[4], (int)value, (int)value));
    if (!queue.Write(buffer, sizeof(buffer))) {
        PError << "pushAction\twrite failed" << endl;
        return false;
    };
    return true;
}

bool ControllerThread::popAction(BYTE* action, PInt32l* value) {
    BYTE buffer[5] = {0, 0, 0, 0, 0};
    if (!queue.Read(buffer, 5)) {
        return PFalse;
    };
    *action = buffer[0];
    *value = ((PInt32l)buffer[1] << 24) + ((PInt32l)(buffer[2] & 0xFF) << 16) + ((PInt32l)(buffer[3] & 0xFF) << 8) + ((PInt32l)buffer[4] & 0xFF);
    PTRACE(5, "popAction\treading buffer " <<
            psprintf("%02x,(%02x%02x%02x%02xh OR %i OR unsinged %u)", (BYTE)buffer[0], (BYTE)buffer[1], (BYTE)buffer[2], (BYTE)buffer[3], (BYTE)(buffer[4]), (int)*value, (int)*value));
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
                case 255:
                    break; // skip summarization of commands controller
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
    //static PTimeInterval[256] = 0;

    for(int i = 0; i < 256; i++) {
        int actionSize = actionQueuePool[i]->GetSize();
        if (actionSize>0) {
            BYTE action = i;
            PInt32l value  = actionQueuePool[i]->GetAt(0);
            if (action>=0 && action<10) {
                // process relative motion actions (axis x1,y1,x2 ...)
                PTRACE(1, "REL AXIS");
            } else if (action>=10 && action<50) {
                // buttons at arduino begin from 20
                action += 10;
                // process absolute trigger action (buttons A,B,...)
                if (value!=0) {
                    value = 1;
                };
                PTRACE(5, "processActions\tSend absolute trigger action " << (int)action << " value " << (int)value);
            } else if (action>=50 && action<60) {
                // process absolute motion actions in persents (axis x1,y1,x2 ...)
                int gamevalue;
                // lookup for value in calibration table
                action -= 50;
                if (value>10000) {
                    value = 10000;
                };
                if (value<-10000) {
                    value = -10000;
                };
                int lookup = value + 10000; // -100,00 -> 0,00 and 100.00 -> 200.00
                // look at 0 .. 20000
                PTRACE(1, "AAAAAAAAAAAAAA!");
                gamevalue = gameTable[action]->GetAt(lookup);
                value = calibrationTable[action]->GetAt(gamevalue);
                PTRACE(5, "processActions\tSend absolute motion action " << (int)action << " offset " << (float)((lookup-10000)/100) << "% value " << (int)value);
            } else if (action>=100 && action<110) {
                // process relative motion actions in pixels*100 (axis x1,y1,x2 ...)
                int gamevalue;
                action -= 100;
                int lookup = (((float)(100*value)/mouseMaximum))+10000; // -100,00 -> 0,00 and 100.00 -> 200.00
                if (lookup>20000) {
                    lookup = 20000;
                };
                if (lookup<0) {
                    lookup = 0;
                };
                gamevalue = gameTable[action]->GetAt(lookup);
                value = calibrationTable[action]->GetAt(gamevalue);
                PTRACE(5, "processActions\tSend relative motion action (1ms) " << (int)action << " offset " << (float)((lookup-10000)/100) << "% value " << (int)value);
            } else if (action != 255) {
                // action 255 is controller command (255+1 == 256 == 0) :-)
                PError << "ControllerThread::processActions() receive unknown action " << (int)action << endl;
            };
            message[0] = action+1; // action
            message[1] = (BYTE)value; // value
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
    processReceive(0, PTrue);
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

bool ControllerThread::isReady() {
    return fReady;
}

void ControllerThread::resetGameTable() {
    for(PINDEX i = 0; i < 10; i++) {
        for(PINDEX j = 0; j<= 20000; j++) {
            gameTable[i]->SetAt(j, j);
        };
    };
}

void ControllerThread::loadGameTableT(PString & name) {
    PStringArray calibrationVector;
    PTRACE(1, "loading game profile '" << name << "'");
    resources->LoadTextFile(name, calibrationVector);
    for (PINDEX i = 0; i < analogControls.GetSize(); i++) {
        PString calibrationValuesName("Axis");
        calibrationValuesName += analogControls[i];
        calibrationValuesName += " ";
        bool tryToFindCommon = PTrue;
        PTRACE(4, "search calibration vector for " << calibrationValuesName);
        // try to find specific vector
        for (PINDEX j = 0; j < calibrationVector.GetSize(); j++) {
            if (strncmp(calibrationVector[j], calibrationValuesName, calibrationValuesName.GetSize()-1) == 0) {
                tryToFindCommon = PFalse;
                // ok, let's parse
                PStringArray calibrationValues = calibrationVector[j].Tokenise(" ", PFalse);
                if (calibrationValues.GetSize() == 2) {
                    PTRACE(1, "apply default calibration vector to " << calibrationValuesName);
                    for (PINDEX k = 0; k <= 20000; k++) { // skip title
                        gameTable[i]->SetAt(k, k);
                    };
                } else {
                    PTRACE(1, "apply specific calibration vector (length " << calibrationValues.GetSize() << ") to " << calibrationValuesName);
                    for (PINDEX k = 1; k < calibrationValues.GetSize(); k++) { // skip title
                        gameTable[i]->SetAt(k, calibrationValues[k].AsInteger());
                    };
                    PTRACE(1, gameTable[i]->GetAt(10000));
                    gameTable[i]->SetAt(0, 0);
                    gameTable[i]->SetAt(20000, 20000);
                };
                break;
            };
        };
        // specific vector not found, try to find common vector
        if (tryToFindCommon) {
            for (PINDEX j = 0; j < calibrationVector.GetSize(); j++) {
                if (strncmp(calibrationVector[j], "common ", 7) == 0) {
                    // ok, let's parse
                    PStringArray calibrationValues = calibrationVector[j].Tokenise(" ", PFalse);
                    PTRACE(1, "apply common calibration vector (length " << calibrationValues.GetSize() << ") to " << calibrationValuesName);
                    for (PINDEX k = 1; k < calibrationValues.GetSize(); k++) { // skip title
                        gameTable[i]->SetAt(k, calibrationValues[k].AsInteger());
                    };
                    PTRACE(1, gameTable[i]->GetAt(10000));
                    gameTable[i]->SetAt(0, 0);
                    gameTable[i]->SetAt(20000, 20000);
                    break;
                };
            };
        };
    };
}

// End of File ///////////////////////////////////////////////////////////////
// vim:ft=c:ts=4:sw=4
