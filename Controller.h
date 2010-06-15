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

#include <ptlib.h>
#include <ptclib/qchannel.h>
#include <ptlib/serchan.h>

#ifndef _CONTROLLERTHREAD_H_
#define _CONTROLLERTHREAD_H_

class ControllerThread : public PThread {
    PCLASSINFO(ControllerThread, PThread);

    public:
        /**Constructor, which initalises version number, application name etc*/
        ControllerThread(PSerialChannel * tserial);
        ~ControllerThread();
        virtual void Main(); // main thread loop
        void Stop(); // shutdown routine
        bool pushAction(BYTE control, WORD value); // populate queue
        bool pushAction(BYTE control, BYTE value1, BYTE value2); // populate queue

    private:
        bool popAction(BYTE buffer[3]); // process queue
        void dumpAction(); // dump to stream action array
        void summarizeActions(); // simplify action arrays
        void processActions(); // send 1st action in array to serial
                               // receive response, remove 1st action that already sent
        /*
         * action queue, 2 bytes
         * 1 byte - action type
         * 1 byte - action quantity
         * x relative  - 0,  0..255 (middle - 128) # left/right
         * y relative  - 1,  0..255 (middle - 128) # up/down
         * x absolute  - 2,  0..255 (middle - 128) # left/right
         * y absolute  - 3,  0..255 (middle - 128) # up/down
         * digital_1  - 4,  0/1
         * digital_2  - 5,  0/1
         * digital_3  - 6,  0/1
         * digital_4  - 7,  0/1
         * digital_5  - 8,  0/1
         * digital_6  - 9,  0/1
         * digital_7  - 10, 0/1
         * digital_8  - 11, 0/1
         * digital_9  - 12, 0/1
         * digital_10 - 13, 0/1
         * digital_11 - 14, 0/1
         * digital_12 - 15, 0/1
         * digital_13 - 16, 0/1
         * digital_14 - 17, 0/1
         * digital_15 - 18, 0/1
         * ...
         * digital_N  - 255,0/1
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
        PWORDArray * action[256];
        /* serial communication channel pointer */
        PSerialChannel * pserial;
};

#endif  // _CONTROLLERTHREAD_H

// End of File ///////////////////////////////////////////////////////////////
// vim:ft=c:ts=4:sw=4
