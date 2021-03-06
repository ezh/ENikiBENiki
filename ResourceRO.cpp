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

ResourceRO::ResourceRO(PHYSFS_file* file) : Resource(file) {
};

SDL_RWops* ResourceRO::GetSDLRWOps() {
    SDL_RWops * rwops = (SDL_RWops*) malloc(sizeof(SDL_RWops));
    memset(rwops, 0, sizeof(SDL_RWops));
    rwops->read = RWOps_Read;
    rwops->seek = RWOps_Seek;
    rwops->close = RWOps_Close;
    rwops->hidden.unknown.data1 = this;
    return rwops;
}

int ResourceRO::RWOps_Read(SDL_RWops* context, void* ptr, int size, int maxnum) {
    ResourceRO * file = (ResourceRO *)context->hidden.unknown.data1;
	file->read(ptr, size, maxnum);
    return maxnum;
};

int ResourceRO::RWOps_Seek(SDL_RWops* context, int offset, int whence) {
    ResourceRO * file = (ResourceRO*)context->hidden.unknown.data1;
    switch(whence) {
        case SEEK_SET:
            file->seek(offset);
            break;
        case SEEK_CUR:
            file->seek(file->tell() + offset);
            break;
        case SEEK_END:
            file->seek(file->fileLength() + offset);
            break;
    }
   // } catch(...) {
   //     LOG(("Unexpected exception while seeking in file."));
   //     return -1;
   // }

    return file->tell();
}

int ResourceRO::RWOps_Close(SDL_RWops* context) {
    ResourceRO * file = (ResourceRO *)context->hidden.unknown.data1;
    delete file;
    context->hidden.unknown.data1 = 0;
    return 1;
}

/*size_t ResourceRO::readz(void* buffer, size_t objsize, size_t objcount)
{
    return (size_t) PHYSFS_read(file, buffer, objsize, objcount);
} */

void ResourceRO::read(void* buffer, size_t objsize, size_t objcount) {
    PHYSFS_read(file, buffer, objsize, objcount);
}

BYTE ResourceRO::read8() {
    BYTE val;
    if(PHYSFS_read(file, &val, 1, 1) != 1) {
        PError << "read error: %s", PHYSFS_getLastError();
    };
    return val;
}

bool ResourceRO::isEOF() {
    return PHYSFS_eof(file);
}

bool ResourceRO::readText(PStringArray & textBuffer) {
    int nString = 0;
    char c;

    seek(0);
    if(isEOF()) {
        PError << "end of file while reading line";
        return PFalse;
    };
    while(!isEOF()) {
        if ((c = read8()) != '\n') {
            textBuffer[nString] += c;
        } else {
            nString++;
        };
    };
    return PTrue;
}
