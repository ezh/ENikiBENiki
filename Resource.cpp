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
#include "physfs.h"
#include "SDL.h"

#include "ResourceRO.h"

#define new PNEW

Resource::Resource(PHYSFS_file* newfile) : file(newfile) {
}

Resource::~Resource() {
    PHYSFS_close(file);
}

bool Resource::eof() {
    return PHYSFS_eof(file);
}

int64_t Resource::tell() {
    return PHYSFS_tell(file);
}

void Resource::seek(uint64_t position) {
    if(!PHYSFS_seek(file, position)) {
     //   throw Exception("Seek operation failed: %s", PHYSFS_getLastError());
    }
}

int64_t Resource::fileLength() {
    return PHYSFS_fileLength(file);
}

void Resource::setBuffer(uint64_t bufsize) {
    if(!PHYSFS_setBuffer(file, bufsize)) {
        //        throw Exception("couldn't adjust buffer size: %s",
          //              PHYSFS_getLastError());
    };
}

void Resource::flush()
{
    PHYSFS_flush(file); // no exception - what should an app do if flush fails?
}

