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
#include "SDL_ttf.h" 

#include "UI.h"

#ifndef _UIConsole_H_
#define _UIConsole_H_

#undef main

class UIConsole : public UI {
    public:
        UIConsole(ControllerThread * _controller, Resources * _resources);
        ~UIConsole();
        void Initialize();
        void Main();
    private:
        void commandPlay();
        void commandSetAbs(BYTE action, BYTE value);
        void commandSetRel(BYTE action, BYTE value);
        BYTE commandGetAbs(BYTE action);
        BYTE commandGetRel(BYTE action);
        void commandUnSet(BYTE action);
        void commandMouse();
        //Make sure the program waits for a quit
        bool quit;
};

#endif  // _UIConsole_H

// End of File ///////////////////////////////////////////////////////////////
// vim:ft=c:ts=4:sw=4
