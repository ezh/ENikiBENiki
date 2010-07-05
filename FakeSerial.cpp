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

#include "FakeSerial.h"

#define new PNEW

FakeSerial::FakeSerial() : fakequeue(1000) {
    fakequeue.SetReadTimeout(0); // timeout 0 ms
    fakequeue.SetWriteTimeout(0); // timeout 0 ms
}

PBoolean FakeSerial::Read(void * buf, PINDEX len) {
    if (!fakequeue.Read(buf, len)) {
        return PFalse;
    };
    lastReadCount = fakequeue.GetLastReadCount();
    return PTrue;
}

PBoolean FakeSerial::Write(const void * buf, PINDEX len) {
    if (!fakequeue.Write(buf, len)) {
        PError << "Write\twrite failed" << endl;
        return PFalse;
    };
    return PTrue;
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

