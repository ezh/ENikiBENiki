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

#include "UIXBoxBinding.h"

#define new PNEW

#define PUSH_LUA_CFUNC(L, x)  \
    lua_pushliteral(L, #x);  \
    lua_pushcfunction(L, x); \
    lua_rawset(L, -3)

enum {
    STATE_WAIT,
    STATE_BEGIN,
    STATE_END,
    STATE_QUIT
};

static int thisIndex = -21832; // LUA_REGISTRYINDEX for the UIXBoxBinding pointer

UIXBoxBinding::UIXBoxBinding(PString _binding, PString _code, ControllerThread * _controller) : 
    PThread(10000, NoAutoDeleteThread),
    fOnFunctionExists(PFalse),
    fOffFunctionExists(PFalse),
    statePhase(STATE_WAIT),
    binding(_binding),
    code(_code) {
    PStringStream threadName;

    controller = _controller;
    threadName << "binding '" << _binding << "'";
    SetThreadName(threadName);
    PTRACE(5, "Constructing instance for binding '" << _binding << "'");
    // setup lua environamet
    L = lua_open();
    luaL_openlibs(L); // load Lua base libraries
    // register this
    lua_pushlightuserdata(L, this);
    lua_rawseti(L, LUA_REGISTRYINDEX, thisIndex);
    // add functions to 'ui' table
    lua_newtable(L);
    PUSH_LUA_CFUNC(L, strikeControlX1);
    PUSH_LUA_CFUNC(L, strikeControlY1);
    PUSH_LUA_CFUNC(L, strikeControlX2);
    PUSH_LUA_CFUNC(L, strikeControlY2);
    PUSH_LUA_CFUNC(L, setControlDU);
    PUSH_LUA_CFUNC(L, setControlDD);
    PUSH_LUA_CFUNC(L, setControlDL);
    PUSH_LUA_CFUNC(L, setControlDR);
    PUSH_LUA_CFUNC(L, setControlBack);
    PUSH_LUA_CFUNC(L, setControlGuide);
    PUSH_LUA_CFUNC(L, setControlStart);
    PUSH_LUA_CFUNC(L, setControlTL);
    PUSH_LUA_CFUNC(L, setControlTR);
    PUSH_LUA_CFUNC(L, setControlA);
    PUSH_LUA_CFUNC(L, setControlB);
    PUSH_LUA_CFUNC(L, setControlX);
    PUSH_LUA_CFUNC(L, setControlY);
    PUSH_LUA_CFUNC(L, addControlLB);
    PUSH_LUA_CFUNC(L, addControlRB);
    PUSH_LUA_CFUNC(L, setControlLT);
    PUSH_LUA_CFUNC(L, setControlRT);
    lua_setglobal(L, "ui");
    // load binding
    if (luaL_loadstring(L, _code) || lua_pcall(L, 0, LUA_MULTRET, 0)) {
        PError << threadName << " lua error: " << lua_tostring(L,-1) << endl;
        return;
    };
    // check on function
    lua_pushstring(L, "on");
    lua_gettable(L, LUA_GLOBALSINDEX);
    if (lua_isfunction(L, -1)) {
        PTRACE(5, "lua 'on' callback found");
        fOnFunctionExists = PTrue;
    };
    lua_pop(L, 1); // pop "on"
    // check off function
    lua_pushstring(L, "off");
    lua_gettable(L, LUA_GLOBALSINDEX);
    if (lua_isfunction(L, -1)) {
        PTRACE(5, "lua 'off' callback found");
        fOffFunctionExists = PTrue;
    };
    lua_pop(L, 1); // pop "off"
    Resume();
}

UIXBoxBinding::~UIXBoxBinding() {
    PTRACE(5, "Destroying instance for binding '" << binding << "'");
    lua_close(L);
}

void UIXBoxBinding::Main() {
    PTRACE(5, "Main\tstart now");
    while(STATE_QUIT != (int)statePhase) {
        stateSync.Wait();
        if (fOnFunctionExists && STATE_BEGIN == (int)statePhase) {
            PTRACE(5, "STATE_BEGIN");
            if (luaL_loadstring(L, "on()") || lua_pcall(L, 0, LUA_MULTRET, 0)) {
                PError << PThread::GetThreadName() << " lua error: " << lua_tostring(L,-1) << endl;
            };
        } else if (fOffFunctionExists && STATE_END == (int)statePhase) {
            PTRACE(5, "STATE_END");
            if (luaL_loadstring(L, "off()") || lua_pcall(L, 0, LUA_MULTRET, 0)) {
                PError << PThread::GetThreadName() << " lua error: " << lua_tostring(L,-1) << endl;
            };
        };
        if (STATE_QUIT != (int)statePhase) {
            statePhase.SetValue(STATE_WAIT);
        };
    };
    PTRACE(5, "Main\tfinished");
}

void UIXBoxBinding::Stop() {
    statePhase.SetValue(STATE_QUIT);
    stateSync.Signal();
}

void UIXBoxBinding::SomethingBegin(SDL_Event &tevent) {
    event = &tevent;
    statePhase.SetValue(STATE_BEGIN);
    stateSync.Signal();
}

void UIXBoxBinding::SomethingEnd(SDL_Event &tevent) {
    event = &tevent;
    statePhase.SetValue(STATE_END);
    stateSync.Signal();
}

/*
 * static functions
 */

UIXBoxBinding* UIXBoxBinding::GetThis(lua_State* L) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, thisIndex);
    if (!lua_isuserdata(L, -1)) {
        luaL_error(L, "Internal error -- missing 'thisIndex'");
    };
    UIXBoxBinding* pUIXBoxBinding = (UIXBoxBinding*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    return pUIXBoxBinding;
}

int UIXBoxBinding::strikeControlX1(lua_State* L) {
    UIXBoxBinding* lthis = UIXBoxBinding::GetThis(L);
    if (lthis->event) {
        if (lthis->event->type == SDL_MOUSEMOTION) {
            // mouse
            // TODO
            PTRACE(1, "strikeControlX1\tcall with SDL event");
        } else {
            // keyboard
            int kpersent = luaL_checknumber(L,1);
            lthis->controller->pushAction(64, 100*kpersent); // gamepad angle MAXIMUM%*kpersent%
        };
    } else {
        PTRACE(1, "strikeControlX1\tcall without SDL event");
    };
    return 0;
};

int UIXBoxBinding::strikeControlY1(lua_State* L) {
    UIXBoxBinding* lthis = UIXBoxBinding::GetThis(L);
    if (lthis->event) {
        if (lthis->event->type == SDL_MOUSEMOTION) {
            // mouse
            // TODO
            PTRACE(1, "strikeControlY1\tcall with SDL event");
        } else {
            // keyboard
            int kpersent = luaL_checknumber(L,1);
            lthis->controller->pushAction(65, 100*kpersent); // gamepad angle MAXIMUM%*kpersent%
        };
    } else {
        PTRACE(1, "strikeControlY1\tcall without SDL event");
    };
    return 0;
};

int UIXBoxBinding::strikeControlX2(lua_State* L) {
    UIXBoxBinding* lthis = UIXBoxBinding::GetThis(L);
    if (lthis->event) {
        if (lthis->event->type == SDL_MOUSEMOTION) {
            // mouse
            // TODO
            PTRACE(1, "strikeControlX2\tcall with SDL event");
        } else {
            // keyboard
            int kpersent = luaL_checknumber(L,1);
            lthis->controller->pushAction(66, 100*kpersent); // gamepad angle MAXIMUM%*kpersent%
        };
    } else {
        PTRACE(1, "strikeControlX2\tcall without SDL event");
    };
    return 0;
};

int UIXBoxBinding::strikeControlY2(lua_State* L) {
    UIXBoxBinding* lthis = UIXBoxBinding::GetThis(L);
    if (lthis->event) {
        if (lthis->event->type == SDL_MOUSEMOTION) {
            // mouse
            // TODO
            PTRACE(1, "strikeControlY2\tcall with SDL event");
        } else {
            // keyboard
            int kpersent = luaL_checknumber(L,1);
            lthis->controller->pushAction(67, 100*kpersent); // gamepad angle MAXIMUM%*kpersent%
        };
    } else {
        PTRACE(1, "strikeControlY2\tcall without SDL event");
    };
    return 0;
};

int UIXBoxBinding::setControlDU(lua_State* L) {
    UIXBoxBinding* lthis = UIXBoxBinding::GetThis(L);
    if (lthis->event) {
        int kvalue = luaL_checknumber(L,1);
        PTRACE(5, "setControlDU\tset value " << kvalue);
        lthis->controller->pushAction(68, kvalue);
    } else {
        PTRACE(1, "setControlDU\tcall without SDL event");
    };
    return 0;
};

int UIXBoxBinding::setControlDD(lua_State* L) {
    UIXBoxBinding* lthis = UIXBoxBinding::GetThis(L);
    if (lthis->event) {
        int kvalue = luaL_checknumber(L,1);
        PTRACE(5, "setControlDD\tset value " << kvalue);
        lthis->controller->pushAction(69, kvalue);
    } else {
        PTRACE(1, "setControlDD\tcall without SDL event");
    };
    return 0;
};

int UIXBoxBinding::setControlDL(lua_State* L) {
    UIXBoxBinding* lthis = UIXBoxBinding::GetThis(L);
    if (lthis->event) {
        int kvalue = luaL_checknumber(L,1);
        PTRACE(5, "setControlDL\tset value " << kvalue);
        lthis->controller->pushAction(70, kvalue);
    } else {
        PTRACE(1, "setControlDL\tcall without SDL event");
    };
    return 0;
};

int UIXBoxBinding::setControlDR(lua_State* L) {
    UIXBoxBinding* lthis = UIXBoxBinding::GetThis(L);
    if (lthis->event) {
        int kvalue = luaL_checknumber(L,1);
        PTRACE(5, "setControlDR\tset value " << kvalue);
        lthis->controller->pushAction(71, kvalue);
    } else {
        PTRACE(1, "setControlDR\tcall without SDL event");
    };
    return 0;
};

int UIXBoxBinding::setControlBack(lua_State* L) {
    UIXBoxBinding* lthis = UIXBoxBinding::GetThis(L);
    if (lthis->event) {
        int kvalue = luaL_checknumber(L,1);
        PTRACE(5, "setControlBack\tset value " << kvalue);
        lthis->controller->pushAction(72, kvalue);
    } else {
        PTRACE(1, "setControlBack\tcall without SDL event");
    };
    return 0;
};

int UIXBoxBinding::setControlGuide(lua_State* L) {
    UIXBoxBinding* lthis = UIXBoxBinding::GetThis(L);
    if (lthis->event) {
        int kvalue = luaL_checknumber(L,1);
        PTRACE(5, "setControlGuide\tset value " << kvalue);
        lthis->controller->pushAction(73, kvalue);
    } else {
        PTRACE(1, "setControlGuide\tcall without SDL event");
    };
    return 0;
};

int UIXBoxBinding::setControlStart(lua_State* L) {
    UIXBoxBinding* lthis = UIXBoxBinding::GetThis(L);
    if (lthis->event) {
        int kvalue = luaL_checknumber(L,1);
        PTRACE(5, "setControlStart\tset value " << kvalue);
        lthis->controller->pushAction(74, kvalue);
    } else {
        PTRACE(1, "setControlStart\tcall without SDL event");
    };
    return 0;
};

int UIXBoxBinding::setControlTL(lua_State* L) {
    UIXBoxBinding* lthis = UIXBoxBinding::GetThis(L);
    if (lthis->event) {
        int kvalue = luaL_checknumber(L,1);
        PTRACE(5, "setControlTL\tset value " << kvalue);
        lthis->controller->pushAction(75, kvalue);
    } else {
        PTRACE(1, "setControlTL\tcall without SDL event");
    };
    return 0;
};

int UIXBoxBinding::setControlTR(lua_State* L) {
    UIXBoxBinding* lthis = UIXBoxBinding::GetThis(L);
    if (lthis->event) {
        int kvalue = luaL_checknumber(L,1);
        PTRACE(5, "setControlTR\tset value " << kvalue);
        lthis->controller->pushAction(76, kvalue);
    } else {
        PTRACE(1, "setControlTR\tcall without SDL event");
    };
    return 0;
};

int UIXBoxBinding::setControlA(lua_State* L) {
    UIXBoxBinding* lthis = UIXBoxBinding::GetThis(L);
    if (lthis->event) {
        int kvalue = luaL_checknumber(L,1);
        PTRACE(5, "setControlA\tset value " << kvalue);
        lthis->controller->pushAction(77, kvalue);
    } else {
        PTRACE(1, "setControlA\tcall without SDL event");
    };
    return 0;
};

int UIXBoxBinding::setControlB(lua_State* L) {
    UIXBoxBinding* lthis = UIXBoxBinding::GetThis(L);
    if (lthis->event) {
        int kvalue = luaL_checknumber(L,1);
        PTRACE(5, "setControlB\tset value " << kvalue);
        lthis->controller->pushAction(78, kvalue);
    } else {
        PTRACE(1, "setControlB\tcall without SDL event");
    };
    return 0;
};

int UIXBoxBinding::setControlX(lua_State* L) {
    UIXBoxBinding* lthis = UIXBoxBinding::GetThis(L);
    if (lthis->event) {
        int kvalue = luaL_checknumber(L,1);
        PTRACE(5, "setControlX\tset value " << kvalue);
        lthis->controller->pushAction(79, kvalue);
    } else {
        PTRACE(1, "setControlX\tcall without SDL event");
    };
    return 0;
};

int UIXBoxBinding::setControlY(lua_State* L) {
    UIXBoxBinding* lthis = UIXBoxBinding::GetThis(L);
    if (lthis->event) {
        int kvalue = luaL_checknumber(L,1);
        PTRACE(5, "setControlY\tset value " << kvalue);
        lthis->controller->pushAction(80, kvalue);
    } else {
        PTRACE(1, "setControlY\tcall without SDL event");
    };
    return 0;
};

int UIXBoxBinding::addControlLB(lua_State* L) {
    UIXBoxBinding* lthis = UIXBoxBinding::GetThis(L);
    if (lthis->event) {
        int kvalue = luaL_checknumber(L,1);
        PTRACE(5, "setControlLB\tset value " << kvalue);
        lthis->controller->pushAction(81, kvalue);
    } else {
        PTRACE(1, "setControlLB\tcall without SDL event");
    };
    return 0;
};

int UIXBoxBinding::addControlRB(lua_State* L) {
    UIXBoxBinding* lthis = UIXBoxBinding::GetThis(L);
    if (lthis->event) {
        int kvalue = luaL_checknumber(L,1);
        PTRACE(5, "setControlRB\tset value " << kvalue);
        lthis->controller->pushAction(82, kvalue);
    } else {
        PTRACE(1, "setControlRB\tcall without SDL event");
    };
    return 0;
};

int UIXBoxBinding::setControlLT(lua_State* L) {
    UIXBoxBinding* lthis = UIXBoxBinding::GetThis(L);
    if (lthis->event) {
        int kvalue = luaL_checknumber(L,1);
        PTRACE(5, "setControlLT\tset value " << kvalue);
        lthis->controller->pushAction(83, kvalue);
    } else {
        PTRACE(1, "setControlLT\tcall without SDL event");
    };
    return 0;
};

int UIXBoxBinding::setControlRT(lua_State* L) {
    UIXBoxBinding* lthis = UIXBoxBinding::GetThis(L);
    if (lthis->event) {
        int kvalue = luaL_checknumber(L,1);
        PTRACE(5, "setControlRT\tset value " << kvalue);
        lthis->controller->pushAction(84, kvalue);
    } else {
        PTRACE(1, "setControlRT\tcall without SDL event");
    };
    return 0;
};

// End of File ///////////////////////////////////////////////////////////////
// vim:ft=c:ts=4:sw=4
