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

#include "ResourceRO.h"
#include "ResourceWO.h"

#undef main

#ifndef _Resources_H_
#define _Resources_H_

class Resources {
    public:
        Resources(PString & _resourceExt);
        ~Resources();
        bool Open(PString & argv0, PString & application);
        bool Close();
        SDL_Surface * LoadImage(PString & imageName);
        SDL_Surface * LoadImageOptimized(PString & imageName);
        //int * loadFile(char *fileName, int *fileSize);
        ResourceRO* RetrieveRead(PString & filename);
        ResourceWO* RetrieveAppend(PString & filename);
        ResourceWO* RetrieveWrite(PString & filename);
    protected:
        /// remember to call freeList()
        char** enumerateFiles(const char* directory);
        static char** enumerateFiles(const std::string& directory);
        void freeList(char** list);
        bool mkdir(const char* dirname);
        bool remove(const char* filename);
        bool exists(const char* filename);
        bool isDirectory(const char* filename);
        bool isSymbolicLink(const char* filename);
        int64_t getLastModTime(const char* filename);
    private:
        bool addToSearchPath(const char* dir, bool append = true);
        bool removeFromSearchPath(const char* dir);
        const char* getRealDir(const char* filename);
        std::string getRealName(const char* filename);
        std::string getRealWriteName(const char* filename);

        bool fOpen;
        PString resourceName;
        PString resourceExt;
};

#endif  // _Resources_H

// End of File ///////////////////////////////////////////////////////////////
// vim:ft=c:ts=4:sw=4
