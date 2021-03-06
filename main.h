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

#ifndef _ENIKIBENIKI_MAIN_H
#define _ENIKIBENIKI_MAIN_H

#include <ptlib/pprocess.h>
#include <ptclib/qchannel.h>
#include <ptlib/serchan.h>

#define APP_CONFIG_DIR ".gamepad fun mod/"

/** The main class that is instantiated to do things */
class ENikiBeNikiProcess : public PProcess
{
    PCLASSINFO(ENikiBeNikiProcess, PProcess)
    public:
        /* Constructor, which initalises version number, application name etc */
        ENikiBeNikiProcess();
        /*
         * Execution starts here, where the command line is processed. In here, the
         * child threads (for generating and consuming data) are launched.
         */
        void Main();

    protected:
        /* serial communication*/
        PSerialChannel * pserial;
    private:
        PBoolean InitializeSerial(PConfigArgs & args);
};


#endif  // _ENIKIBENIKI_MAIN_H

// End of File ///////////////////////////////////////////////////////////////
// vim:ft=c:ts=4:sw=4
