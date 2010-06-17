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
#include "physfs.h"
#include "SDL.h"

#include "ResourceRO.h"

#define new PNEW

ResourceRO::ResourceRO(PHYSFS_file* file) : Resource(file) {
};

SDL_RWops* ResourceRO::getSDLRWOps() {
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

size_t ResourceRO::readz(void* buffer, size_t objsize, size_t objcount)
{
    return (size_t) PHYSFS_read(file, buffer, objsize, objcount);
}

void ResourceRO::read(void* buffer, size_t objsize, size_t objcount)
{
    PHYSFS_sint64 objsread = PHYSFS_read(file, buffer, objsize, objcount);
    //if(objsread != (PHYSFS_sint64) objcount)
	//throw FileReadException(objsread, objcount, "eof while reading");
}

bool ResourceRO::isEOF()
{
    return PHYSFS_eof(file);
}

