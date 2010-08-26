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
#include <stdint.h>
#include "SDL.h"
#include "Resource.h"

#ifndef _ResourceWO_H_
#define _ResourceWO_H_

#undef main

class ResourceWO {
    public:
        ResourceWO(PHYSFS_file* file);
//        void write(const void* buffer, size_t objsize, size_t objcount);

//        void write8(Sint8 val);

//        void writeSLE16(Sint16 val);
//        void writeULE16(Uint16 val);
//        void writeSBE16(Sint16 val);
//        void writeUBE16(Uint16 val);

//        void writeSLE32(Sint32 val);
//        void writeULE32(Uint32 val);
//        void writeSBE32(Sint32 val);
//        void writeUBE32(Uint32 val);

//        void writeSLE64(int64_t val);
//        void writeULE64(uint64_t val);
//        void writeSBE64(int64_t val);
//        void writeUBE64(uint64_t val);

        /// writes the text in the buffer and an additional newline
//        void writeLine(const std::string& line);

        /** for inernal use only */
//        WriteFile();
};

#endif  // _ResourceWO_H

// End of File ///////////////////////////////////////////////////////////////
// vim:ft=c:ts=4:sw=4
