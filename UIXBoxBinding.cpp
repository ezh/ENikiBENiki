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

#include "UIXBox.h"
#include "UIXBoxBinding.h"

#define new PNEW

enum {
    STATE_WAIT,
    STATE_BEGIN,
    STATE_END,
    STATE_QUIT
};

UIXBoxBinding::UIXBoxBinding(PString _binding, int _code, PString _script, void *(*codeKeyToClass)[32767], ControllerThread *_controller, PConfig *config) : 
    PThread(10000, NoAutoDeleteThread),
    fOnFunctionExists(PFalse),
    fOffFunctionExists(PFalse),
    statePhase(STATE_WAIT),
    binding(_binding),
    script(_script) {
    PStringStream threadName;
    PStringArray analogControls = (config->GetString("Gamepad", "AnalogControl", "")).ToUpper().Tokenise(",", PFalse);
    PStringArray digitalControls = (config->GetString("Gamepad", "DigitalControl", "")).ToUpper().Tokenise(",", PFalse);

    controller = _controller;
    threadName << "'" << _binding << "' binding";
    SetThreadName(threadName);
    PTRACE(2, "Constructing instance for binding '" << _binding << "' with internal control code " << _code);
    // setup lua environamet
    L = lua_open();
    luaL_openlibs(L); // load Lua base libraries
    // add functions to 'ui' table
    lua_newtable(L);
    for (PINDEX i = 0; i < analogControls.GetSize(); i++) {
        bool skip = PFalse; // skip register process
        PString functionName("Control");
        functionName += analogControls[i];

        // check that control already registered
        for (PINDEX j = 0; j < analogControls.GetSize(); j++) {
            if (i > j && analogControls[i] == analogControls[j]) {
                skip = PTrue;
                break;
            };
        };
        if (!skip) {
            PTRACE(4, "register lua function 'ui." << functionName << "' for analog control N" << i);
            lua_pushstring(L, functionName);
            lua_pushstring(L, analogControls[i]);
            lua_pushinteger(L, _code);
            lua_pushinteger(L, i);
            lua_pushlightuserdata(L, this);
            lua_pushcclosure(L, analogControl, 4);
            lua_rawset(L, -3);
        } else {
            PError << "skip lua function 'ui." << functionName << "' for analog control N" << i << ", " << analogControls[i] << " already registered" << endl;
        };
    };
    for (PINDEX i = 0; i < digitalControls.GetSize(); i++) {
        bool skip = PFalse; // skip register process
        PString functionName("Control");
        functionName += digitalControls[i];

        // check that control already registered
        for (PINDEX j = 0; j < digitalControls.GetSize(); j++) {
            if (i > j && digitalControls[i] == digitalControls[j]) {
                skip = PTrue;
                break;
            };
        };
        if (!skip) {
            PTRACE(4, "register lua function 'ui." << functionName << "' for digital control N" << i);
            lua_pushstring(L, functionName);
            lua_pushstring(L, digitalControls[i]);
            lua_pushinteger(L, _code);
            lua_pushinteger(L, i);
            lua_pushlightuserdata(L, this);
            lua_pushcclosure(L, digitalControl, 4);
            lua_rawset(L, -3);
        } else {
            PError << "skip lua function 'ui." << functionName << "' for digital control N" << i << ", " << digitalControls[i] << " already registered" << endl;
        };
    };
    lua_setglobal(L, "ui");
    // load binding
    if (luaL_loadstring(L, _script) || lua_pcall(L, 0, LUA_MULTRET, 0)) {
        PError << threadName << " lua error: " << lua_tostring(L,-1) << endl;
        return;
    };
    // check 'on' function
    lua_pushstring(L, "on");
    lua_gettable(L, LUA_GLOBALSINDEX);
    if (lua_isfunction(L, -1)) {
        PTRACE(4, "lua 'on' callback found");
        fOnFunctionExists = PTrue;
    } else {
        PTRACE(4, "lua 'on' callback not found");
    };
    lua_pop(L, 1); // pop "on"
    // check 'off' function
    lua_pushstring(L, "off");
    lua_gettable(L, LUA_GLOBALSINDEX);
    if (lua_isfunction(L, -1)) {
        PTRACE(4, "lua 'off' callback found");
        fOffFunctionExists = PTrue;
    } else {
        PTRACE(4, "lua 'ff' callback not found");
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
int UIXBoxBinding::analogControl(lua_State* L) {
    const char *name = lua_tostring(L, lua_upvalueindex(1));
    int id = lua_tointeger(L, lua_upvalueindex(3));
    UIXBoxBinding* ui = static_cast<UIXBoxBinding *>(const_cast<void *>(lua_topointer(L, lua_upvalueindex(4))));
    if (ui->event) {
        int kpersent = luaL_checknumber(L,1);
        if (ui->event->type == SDL_MOUSEMOTION) {
            int code = lua_tointeger(L, lua_upvalueindex(2));
            int value = 0;
            // get value
            switch (code) {
                case MOUSE_N0:
                    value = ui->event->motion.xrel;
                    break;
                case MOUSE_N1:
                    value = ui->event->motion.yrel;
                    break;
                default:
                    PError << ui->GetThreadName() << " Control" << name << " unknow axis for code " << code << endl;
                    break;
            };
            // mouse
            if (kpersent > 1000) {
                PError << ui->GetThreadName() << " Control" << name << "(persent) argument " << kpersent << " too high. Assign correct maximum value 1000%" << endl;
                kpersent = 1000;
            };
            if (kpersent < -1000) {
                PError << ui->GetThreadName() << " Control" << name << "(persent) argument " << kpersent << " too low. Assign correct minimum value -1000%" << endl;
                kpersent = -1000;
            };
            value *= kpersent/10;
            PTRACE(5, "Control" << name << "\tcall with modifier " << kpersent << "%, shift is " << value << "px (mouse mode)");
            ui->controller->pushAction(100+id, value); // mouse shift Npx*N%*10
        } else {
            // keyboard
            if (kpersent > 100) {
                PError << ui->GetThreadName() << " Control" << name << "(persent) argument " << kpersent << " too high. Assign correct maximum value 100%" << endl;
                kpersent = 100;
            };
            if (kpersent < -100) {
                PError << ui->GetThreadName() << " Control" << name << "(persent) argument " << kpersent << " too low. Assign correct minimum value -100%" << endl;
                kpersent = -100;
            };
            PTRACE(5, "Control" << name << "\tcall with modifier " << kpersent << "% (keyboard mode)");
            ui->controller->pushAction(50+id, 10000*kpersent/100); // gamepad angle NNN.NN% up to 100%
        };
    } else {
        PTRACE(1, "Control" << name << "\tcall without SDL event");
    };
    return 0;
};

int UIXBoxBinding::digitalControl(lua_State* L) {
    const char *name = lua_tostring(L, lua_upvalueindex(1));
    int id = lua_tointeger(L, lua_upvalueindex(3));
    UIXBoxBinding* ui = static_cast<UIXBoxBinding *>(const_cast<void *>(lua_topointer(L, lua_upvalueindex(4))));
    if (ui->event) {
        int kvalue = luaL_checknumber(L,1);
        if (kvalue != 0 && kvalue != 1) {
            kvalue = 1;
        };
        PTRACE(5, "Control" << name << "\tcall with value " << kvalue);
        ui->controller->pushAction(id, kvalue);
    } else {
        PTRACE(1, "Control" << name << "\tcall without SDL event");
    };
    return 0;
};

// End of File ///////////////////////////////////////////////////////////////
// vim:ft=c:ts=4:sw=4
