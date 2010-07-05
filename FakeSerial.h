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
#include <ptlib/serchan.h>
#include <ptclib/qchannel.h>

#ifndef _FAKESERIAL_H_
#define _FAKESERIAL_H_

class FakeSerial : public PSerialChannel
{
    public:
        FakeSerial();
        PBoolean Read(void * buf, PINDEX len);
        PBoolean Write(const void * buf, PINDEX len);
        PINDEX GetLastReadCount() const;
        PBoolean Open(const PString & port, DWORD speed, BYTE data, Parity parity, BYTE stop, FlowControl inputFlow, FlowControl outputFlow);
        PBoolean Close();
        void SetReadTimeout(const PTimeInterval & time);
        void SetWriteTimeout(const PTimeInterval & time);
    private:
        int lastReadCount;
        PQueueChannel fakequeue;
};

#endif // _FAKESERIAL_H_
// End of File ///////////////////////////////////////////////////////////////
// vim:ft=c:ts=4:sw=4
