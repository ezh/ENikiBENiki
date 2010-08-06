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
        UIXBoxBinding(PString _binding, PString _code, ControllerThread * _controller);
        virtual ~UIXBoxBinding();
        void Main();
        void Stop();
        void SomethingBegin(SDL_Event &tevent);
        void SomethingEnd(SDL_Event &tevent);
    private:
        // script functions
        static UIXBoxBinding* GetThis(lua_State* L);
        static int strikeControlX1(lua_State* L);
        static int strikeControlY1(lua_State* L);
        static int strikeControlX2(lua_State* L);
        static int strikeControlY2(lua_State* L);
        static int setControlDU(lua_State* L);
        static int setControlDD(lua_State* L);
        static int setControlDL(lua_State* L);
        static int setControlDR(lua_State* L);
        static int setControlBack(lua_State* L);
        static int setControlGuide(lua_State* L);
        static int setControlStart(lua_State* L);
        static int setControlTL(lua_State* L);
        static int setControlTR(lua_State* L);
        static int setControlA(lua_State* L);
        static int setControlB(lua_State* L);
        static int setControlX(lua_State* L);
        static int setControlY(lua_State* L);
        static int addControlLB(lua_State* L);
        static int addControlRB(lua_State* L);
        static int setControlLT(lua_State* L);
        static int setControlRT(lua_State* L);
        // execution context
        ControllerThread * controller;
        SDL_Event * event; // last event
        bool fNonStop; // unbreakable fuction, skip End event, Begin event run till the end
        bool fOnFunctionExists; // flag, that indicate presence lua 'on' callback
        bool fOffFunctionExists; // flag, that indicate presence lua 'off' callback
        PSyncPoint stateSync;
        PAtomicInteger statePhase;
        lua_State *L;
        PString binding;
        PString code;
};

#endif  // _UIXBoxBinding_H_

// End of File ///////////////////////////////////////////////////////////////
// vim:ft=c:ts=4:sw=4
