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

#include "UIConsole.h"

#define new PNEW

UIConsole::UIConsole(ControllerThread * _controller, Resources * _resources, PConfig * _config) :
    UI(_controller, _resources, _config) {

    quit   = PFalse;
}

UIConsole::~UIConsole() {
}

bool UIConsole::Initialize() {
    SDL_WM_GrabInput(SDL_GRAB_ON);
    cout << "XBOX console mode initialized" << endl;
    return PTrue;
}

void UIConsole::Main() {
    PString userinput;
    PRegularExpression regex_help("(\\?|h|help)", PRegularExpression::Extended|PRegularExpression::IgnoreCase);
    PRegularExpression regex_quit("(exit|quit)", PRegularExpression::Extended|PRegularExpression::IgnoreCase);
    PRegularExpression regex_play("(p|play)", PRegularExpression::Extended|PRegularExpression::IgnoreCase);
    PRegularExpression regex_setabs("(sa|setabs) [[:digit:]]+ [[:digit:]]+", PRegularExpression::Extended|PRegularExpression::IgnoreCase);
    PRegularExpression regex_setraw("(sr|setraw) [[:digit:]]+ [[:digit:]]+", PRegularExpression::Extended|PRegularExpression::IgnoreCase);
    PRegularExpression regex_setcmd("(sc|setcmd) [[:digit:]]+", PRegularExpression::Extended|PRegularExpression::IgnoreCase);
    PRegularExpression regex_setsoft("(ss|setsoft) [[:digit:]]+ [-[:digit:]]+", PRegularExpression::Extended|PRegularExpression::IgnoreCase);
    PRegularExpression regex_getabs("(ga|getabs) [[:digit:]]+", PRegularExpression::Extended|PRegularExpression::IgnoreCase);
    PRegularExpression regex_getrel("(gr|getrel) [[:digit:]]+", PRegularExpression::Extended|PRegularExpression::IgnoreCase);
    PRegularExpression regex_unset("(u|unset) [[:digit:]]+", PRegularExpression::Extended|PRegularExpression::IgnoreCase);
    PRegularExpression regex_mouse("(m|mouse)", PRegularExpression::Extended|PRegularExpression::IgnoreCase);
    PRegularExpression regex_reset("(r|reset)", PRegularExpression::Extended|PRegularExpression::IgnoreCase);

    cout << "waiting for arduino..." << endl;
    while (!controller->isReady()) {
        PThread::Sleep(100);
    };
    cout << "controller ready" << endl;

    while (quit == PFalse) {
        cout << "> ";
        userinput.ReadFrom(cin);
        cout << "thinking" << endl;

        if (userinput.MatchesRegEx(regex_help)) {
            cout << "command\t\tdescription" << endl
                 << endl
                 << "exit,quit\tterminates the command-line ENikiBENiki session" << endl
                 << "p,play" << endl
                 << "sa,setabs BYTE BYTE" << endl
                 << "sr,setraw BYTE BYTE" << endl
                 << "sc,setcmd BYTE" << endl
                 << "ss,setsoft BYTE BYTE" << endl
                 << "ga,getabs BYTE" << endl
                 << "gr,getrel BYTE" << endl
                 << "u,unset BYTE" << endl
                 << "m,mouse" << endl
                 << "r,reset" << endl
                 << "?,h,help\twrite this help out." << endl
                 << endl;
        } else if (userinput.MatchesRegEx(regex_quit)) {
            quit = PTrue;
        } else if (userinput.MatchesRegEx(regex_play)) {
            commandPlay();
        } else if (userinput.MatchesRegEx(regex_setabs)) {
            PStringArray parts = userinput.Tokenise(" ");
            if (parts[1].AsUnsigned()>255 || parts[2].AsUnsigned()>255) {
                cout << "ERROR: argument(BYTE) out of range, command: " << userinput << endl;
                continue;
            };
            commandSetAbs(parts[1].AsUnsigned(), parts[2].AsUnsigned());
        } else if (userinput.MatchesRegEx(regex_setraw)) {
            PStringArray parts = userinput.Tokenise(" ");
            if (parts[1].AsUnsigned()>255 || parts[2].AsUnsigned()>255) {
                cout << "ERROR: argument(BYTE) out of range, command: " << userinput << endl;
                continue;
            };
            commandSetRaw(parts[1].AsUnsigned(), parts[2].AsUnsigned());
        } else if (userinput.MatchesRegEx(regex_setcmd)) {
            PStringArray parts = userinput.Tokenise(" ");
            if (parts[1].AsUnsigned()>255) {
                cout << "ERROR: argument(BYTE) out of range, command: " << userinput << endl;
                continue;
            };
            commandSetCmd(parts[1].AsUnsigned());
        } else if (userinput.MatchesRegEx(regex_setsoft)) {
            PStringArray parts = userinput.Tokenise(" ");
            if (parts[1].AsUnsigned()>255 || parts[2].AsInteger() < -10000 || parts[2].AsInteger() > 10000) {
                cout << "ERROR: argument out of range, command: " << userinput << endl;
                continue;
            };
            commandSetSoft(parts[1].AsUnsigned(), parts[2].AsInteger());
        } else if (userinput.MatchesRegEx(regex_getrel)) {
            PStringArray parts = userinput.Tokenise(" ");
            if (parts[1].AsUnsigned()>255) {
                cout << "ERROR: argument(BYTE) out of range, command: " << userinput << endl;
                continue;
            };
            BYTE value = commandGetRel(parts[1].AsUnsigned());
            cout << value << endl;
        } else if (userinput.MatchesRegEx(regex_unset)) {
            PStringArray parts = userinput.Tokenise(" ");
            if (parts[1].AsUnsigned()>255) {
                cout << "ERROR: argument(BYTE) out of range, command: " << userinput << endl;
                continue;
            };
            commandUnSet(parts[1].AsUnsigned());
        } else if (userinput.MatchesRegEx(regex_mouse)) {
            commandMouse();
        } else if (userinput.MatchesRegEx(regex_reset)) {
            commandReset();
        } else if (!userinput.IsEmpty()) {
            cout << "Unknown command: " << userinput << endl;
        };
    }
};

void UIConsole::commandPlay() {
 //       cin.getline(query, 2048, '\n');
 //       SetCStr(queryArray, query);

//        cout << "> " << flush;
//        char ch;
//        cin >> ch;
//        ch = toupper(ch);

/*        if (SDL_WaitEvent(&event) == 1) {
            // Check for other mouse motion events in the queue.
            SDL_Event eventFuture;
            int num = SDL_PeepEvents( &eventFuture, 1, SDL_PEEKEVENT, SDL_ALLEVENTS );
            // If this is the same state, ignore this one
            if (!(num > 0 && eventFuture.type == SDL_MOUSEMOTION &&
                        eventFuture.motion.state == event.motion.state )) {
                if (event.type == SDL_MOUSEBUTTONDOWN) {
                    eventMouseDown();
                } else if (event.type == SDL_MOUSEBUTTONUP) {
                    eventMouseUp();
                } else if (event.type == SDL_MOUSEMOTION) {
                    eventMouseMotion();
                } else if (event.type == SDL_KEYDOWN) {
                    eventKeyDown();
                } else if (event.type == SDL_QUIT) {
                    eventQuit();
                };
            };
        }*/
};

void UIConsole::FlushSDLEvents(void) {
	SDL_Event event;

	while(SDL_PollEvent(&event)) {
    }
}

void UIConsole::commandMouse() {
    PTime tBegin;
    PTime tLast;
    SDL_Surface * screen;
    int maximumShift = 0;
    int maximumDelay = 0;
    PTime tBase; // base time for tStep multiplier
    PTimeInterval tStep(1); // 1ms, loop step (1Hz); up to 14 bytes per step for 115200 serial line
    PTime tNow; // current time
    PTime tThen; // expected execution time
    unsigned short i = 0; // multiplier for tStep
    /* shutdown trigger */
    PSyncPoint shutdown;
    int x, y;
    int counter = 0;

    cout << "enter mouse colibration mode" << endl;
    //Start SDL
    SDL_Init(SDL_INIT_EVERYTHING);
    screen = SDL_SetVideoMode(640, 240, 32, SDL_SWSURFACE);
    SDL_WM_GrabInput(SDL_GRAB_ON);
    SDL_ShowCursor(SDL_DISABLE);
    // skip standard events
    SDL_EventState(SDL_ACTIVEEVENT, SDL_IGNORE);
    SDL_EventState(SDL_KEYDOWN, SDL_IGNORE);
    SDL_EventState(SDL_KEYUP, SDL_IGNORE);
    SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);
    SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_IGNORE);
    SDL_EventState(SDL_MOUSEBUTTONUP, SDL_IGNORE);
    SDL_EventState(SDL_JOYAXISMOTION, SDL_IGNORE);
    SDL_EventState(SDL_JOYBALLMOTION, SDL_IGNORE);
    SDL_EventState(SDL_JOYHATMOTION, SDL_IGNORE);
    SDL_EventState(SDL_JOYBUTTONDOWN, SDL_IGNORE);
    SDL_EventState(SDL_JOYBUTTONUP, SDL_IGNORE);
    // flush junk
    FlushSDLEvents();
    SDL_GetMouseState(&x, &y);
    SDL_GetRelativeMouseState(&x, &y);
    tNow = PTime();
    tLast = tNow;
    do {
        counter++;
        tNow = PTime();
        FlushSDLEvents();
        SDL_GetRelativeMouseState(&x, &y);
        if (x != 0 || y != 0) {
            x = x > 0 ? x / counter : -x / counter;
            y = y > 0 ? y / counter : -y / counter;
            if (x > maximumShift) {
                PTRACE(1, "MAXIMUM X:" << x << " COUNTER: " << counter);
                maximumShift = x;
            };
            if (y > maximumShift) {
                PTRACE(1, "MAXIMUM Y:" << y << " COUNTER: " << counter);
                maximumShift = y;
            };
            if (counter > maximumDelay && counter < 100) {
                maximumDelay = counter;
            }
            counter = 0;
        };
        if ((tNow - tBegin).GetSeconds() > 10) {
            shutdown.Signal();
        };
        /*
         * wait next tStep ms
         */
        i++;
        tThen = tBase + tStep * i;
        tNow = PTime();
        // reset multiplier
        if (i >= 255) {
            i = 0;
            tBase = tThen;
        };
        // step was too long (tThen less than tNow)
        if (tNow.Compare(tThen) != -1) {
            PTRACE(2, "commandMouse\tnow: " << tNow.AsString("h:m:s.uuuu") << " then: " << tThen.AsString("h:m:s.uuuu") << " i: " << (int)i << " diff: " << (tNow - tThen).GetMilliSeconds() << "ms");
            i += (tNow - tThen).GetMilliSeconds() / tStep.GetMilliSeconds() + 1; // number of steps + 1 step
            tThen = tBase + tStep * i;
            PTRACE(2, "commandMouse\tcorrected then: " << tThen.AsString("h:m:s.uuuu") << " i: " << (int)i);
        };
        PTRACE(3, "commandMouse\tstep " << (tThen - tNow).GetMilliSeconds() << "ms"); 
    } while(!shutdown.Wait((tThen - tNow).GetMilliSeconds()));
   // SDL_ShowCursor(SDL_ENABLE);
    SDL_WM_GrabInput(SDL_GRAB_OFF);
    SDL_FreeSurface(screen);
    cout << "maximum offset at ~1kHz: " << maximumShift << " with delay: " << maximumDelay << endl;
    // Quit SDL
    SDL_Quit();
    cout << "exit mouse colibration mode" << endl;
};

void UIConsole::commandReset() {
    controller->pushAction(-1, 1);
};

void UIConsole::commandSetAbs(BYTE action, BYTE value) {
    controller->pushAction(action, value);
};

void UIConsole::commandSetRaw(BYTE action, BYTE value) {
    controller->pushAction(action-1, value);
};

void UIConsole::commandSetCmd(BYTE value) {
    controller->pushAction(-1, value);
};

void UIConsole::commandSetSoft(BYTE action, int value) {
    controller->pushAction(action+50, value);
};

BYTE UIConsole::commandGetAbs(BYTE action) {
    return 0;
};

BYTE UIConsole::commandGetRel(BYTE action) {
    return 0;
};

void UIConsole::commandUnSet(BYTE action) {
    controller->pushAction(action, (WORD)0);
};

