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

#include "FakeSerial.h"

#define new PNEW

FakeSerial::FakeSerial() : fakequeue(1000) {
    fakequeue.SetReadTimeout(0); // timeout 0 ms
    fakequeue.SetWriteTimeout(0); // timeout 0 ms
    PThread::Create(PCREATE_NOTIFIER(HeatBeat), 30000,
            PThread::NoAutoDeleteThread,
            PThread::NormalPriority);
}

PBoolean FakeSerial::Read(void * buf, PINDEX len) {
    bool result = fakequeue.Read(buf, len);
    if (result) {
        lastReadCount = fakequeue.GetLastReadCount();
    } else {
        lastReadCount = 0;
    };
    return result;
}

PBoolean FakeSerial::Write(const void * buf, PINDEX len) {
    bool result = PFalse;
    const BYTE *pbuf = (BYTE*)buf;

    if (len == 3) {
        BYTE expect = pbuf[0] + pbuf[1] + pbuf[2];
        if (expect == 0) {
            expect = 1;
        };
        result = fakequeue.Write(&expect, 1);
    } else {
        PError << "unknown message" << endl;
    };
    return result;
}

PINDEX FakeSerial::GetLastReadCount() const {
    return lastReadCount;
};

PBoolean FakeSerial::Open(const PString & port, DWORD speed, BYTE data, Parity parity, BYTE stop, FlowControl inputFlow, FlowControl outputFlow) {
    return PTrue;
};

PBoolean FakeSerial::Close() {
    return PTrue;
};

void FakeSerial::SetReadTimeout(const PTimeInterval & time) {
};

void FakeSerial::SetWriteTimeout(const PTimeInterval & time) {
};

void FakeSerial::HeatBeat(PThread &, INT) {
    char buf[2] = {0, 0};
    for (;;) {
        fakequeue.Write(buf, 2);
        PThread::Sleep(1000);
    };
}

