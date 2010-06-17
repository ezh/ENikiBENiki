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

#ifndef _Resource_H_
#define _Resource_H_

class Resource {
    public:
        ~Resource();
        bool eof();
        int64_t tell();
        void seek(uint64_t position);
        int64_t fileLength();
        // conveniance function, since I think this name is more c++ typic
        int64_t length();
        void setBuffer(uint64_t bufsize);
        void flush();
    protected:
        Resource(PHYSFS_file* file);
        PHYSFS_file * file;
};

#endif  // _Resource_H

// End of File ///////////////////////////////////////////////////////////////
// vim:ft=c:ts=4:sw=4
