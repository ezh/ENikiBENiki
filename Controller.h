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

#include <ptlib.h>
#include <ptclib/qchannel.h>
#include <ptlib/serchan.h>

#include "Resources.h"

#ifndef _CONTROLLERTHREAD_H_
#define _CONTROLLERTHREAD_H_

/*
 * commands for controller
 */
#define CMD_RESET 1
#define CMD_SETBASE0 2 // next message set middle [0]
#define CMD_SETBASE1 3 // next message set middle [1]
#define CMD_SETBASE2 4 // next message set middle [2]
#define CMD_SETBASE3 5 // next message set middle [3]
#define CMD_SETBASE4 6 // next message set middle [4]
#define CMD_SETBASE5 7 // next message set middle [5]
#define CMD_SETBASE6 8 // next message set middle [6]
#define CMD_SETBASE7 9 // next message set middle [7]
#define CMD_SETBASE8 10 // next message set middle [8]

class ControllerThread : public PThread {
    PCLASSINFO(ControllerThread, PThread);

    public:
        ControllerThread(PSerialChannel *_serial, Resources * _resources, PConfig *_config);
        ~ControllerThread();
        virtual void Main(); // main thread loop
        void Stop(); // shutdown routine
        bool pushAction(BYTE action, PInt32l value); // populate queue
        bool isReady();
        void resetGameTable();
        void loadGameTableT(PString & name);
        //void loadGameTableB(PString & name);

    private:
        bool popAction(BYTE *action, PInt32l *value); // process queue
        PString dumpAction(const char * szHeader); // dump to stream action array
        void summarizeActions(); // simplify action array
        void processActions(); // send action in array to serial
                               // receive response, remove action that already sent
        bool processTransmit(const unsigned char* message, PINDEX length);
        bool processReceive(BYTE expect = 0, bool peek = 0);
        /*
         * settings
         */
        int mouseMaximum;
        PIntArray *calibrationTable[10];
        PIntArray *gameTable[10];
        /*
         * action queue: BYTE action N, int value
         * AXIS ABSOLUTE (unsigned potentiometer values)
         * action 0 : axis 0 absolute
         * action 1 : axis 1 absolute
         * action 2 : axis 2 absolute
         * ....
         * action 9 : axis 9 absolute
         * BUTTON (bool 0/1)
         * action 10 : button 1 absolute
         * action 11 : button 2 absolute
         * action 12 : button 3 absolute
         * ....
         * action 19 : button 9 absolute
         * AXIS ABSOLUTE (signed percent 100.00 or 10000 integer)
         * action 50 : axis 0 persent
         * action 51 : axis 1 persent
         * action 52 : axis 2 persent
         * ...
         * action 59 : axis 9 persent
         * AXIS RELATIVE (signed pixels px*100)
         * action 100 : axis 0 pixels
         * action 101 : axis 1 pixels
         * action 102 : axis 2 pixels
         * ....
         * action 109 : axis 9 pixels
         */
        PQueueChannel queue;
        /* shutdown trigger */
        PSyncPoint shutdown;
        /*
         * action matrix
         * j action - value1, value2(next step), value3(after that)
         * k action - value1
         * n action - ...
         * value1 - send this step
         * value2..n - will send after
         */
        PIntArray *actionQueuePool[256];
        /* Arduino ready */
        bool fReady;
        /* limit */
        int retryLimit;
        /* I/O timeout ms */
        int timeout;
        /* serial communication channel pointer */
        PSerialChannel * pserial;
        Resources *resources;
        PConfig *config;
        PStringArray analogControls;
};

#endif  // _CONTROLLERTHREAD_H

// End of File ///////////////////////////////////////////////////////////////
// vim:ft=c:ts=4:sw=4
