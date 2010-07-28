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
#include <stdint.h>
#include "SDL.h"
#include "Resource.h"

#ifndef _ResourceRO_H_
#define _ResourceRO_H_

#undef main

class ResourceRO : public Resource {
    public:
        ResourceRO(PHYSFS_file* file);
        bool isEOF();
        void read(void* buffer, size_t objsize, size_t objcount);
        size_t readz(void* buffer, size_t objsize, size_t objcount);

//        Sint8 read8();

//        Sint16 readSLE16();
//        Uint16 readULE16();
//        Sint16 readSBE16();
//        Uint16 readUBE16();

//        Sint32 readSLE32();
//        Uint32 readULE32();
//        Sint32 readSBE32();
//        Uint32 readUBE32();

//        int64_t readSLE64();
//        uint64_t readULE64();
//        int64_t readSBE64();
//        uint64_t readUBE64();

//        void readLine(std::string& buffer);

        // Returns the SDL_RWops structure which can be used in several SDL
        // commands. Note that you have to free this structure with SDL_FreeRWops.
        // (Most SDL commands also have a freesrc parameter in their calls which you
        // can simply set to 1)
        SDL_RWops* GetSDLRWOps();

        /** for internal use only */

    private:
        static int RWOps_Read(SDL_RWops* context, void* ptr, int size, int maxnum);
        static int RWOps_Seek(SDL_RWops* context, int offset, int whence);
        static int RWOps_Close(SDL_RWops* context);
};

#endif  // _ResourceRO_H

// End of File ///////////////////////////////////////////////////////////////
// vim:ft=c:ts=4:sw=4
