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

#include "SDL.h"

#include "UI.h"

#ifndef _UIDefault_H_
#define _UIDefault_H_

#undef main

class UIDefault : public UI {
    public:
        UIDefault();
        UIDefault(ControllerThread * _controller, Resources * _resources, PConfig * _config);
        virtual ~UIDefault();
        bool Initialize();
        void Main();
    private:
        //The images
        SDL_Surface * hello;
        SDL_Surface * screen;
        //The event structure that will be used
        SDL_Event event;
        //Make sure the program waits for a quit
        bool quit;
        // x y
        signed short x;
        signed short y;
};

#endif  // _UIDefault_H

// End of File ///////////////////////////////////////////////////////////////
// vim:ft=c:ts=4:sw=4
