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

#include "UIXBox.h"
#include "Controller.h"

#include "SDL.h"
#include "lua.hpp"
#include <ptlib/pprocess.h>

#ifndef _UIXBoxBinding_H_
#define _UIXBoxBinding_H_

#undef main

class UIXBoxBinding : public PThread {
    PCLASSINFO(UIXBoxBinding, PThread);
    public:
        UIXBoxBinding(PString _binding, int _code, PString _script, UIXBox* _uixbox);
        virtual ~UIXBoxBinding();
        void Main();
        void Stop();
        void SomethingBegin(SDL_Event &tevent);
        void SomethingEnd(SDL_Event &tevent);
    private:
        // lua script functions
        static int analogControl(lua_State* L);     // mouse action N  0..100.00% (1ms signal) OR keyboard action 50+N 0..100.00 (persistent), where 0 < N < 10
        static int digitalControl(lua_State* L);    // action N 0/1, where 10 <= N < 50
        UIXBox *ui; // binding context
        SDL_Event *event; // last binding SDL event
        bool fNonStop; // unbreakable fuction, skip End event, Begin event run till the end
        bool fOnFunctionExists; // flag, that indicate presence lua 'on' callback
        bool fOffFunctionExists; // flag, that indicate presence lua 'off' callback
        PSyncPoint stateSync; // signal when something changed
        PAtomicInteger statePhase; // binding state
        lua_State *L; // lua state
        PString binding; // binding name
        PString script; // binding script
};

#endif  // _UIXBoxBinding_H_

// End of File ///////////////////////////////////////////////////////////////
// vim:ft=c:ts=4:sw=4
