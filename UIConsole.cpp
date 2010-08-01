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

#include "UIConsole.h"

#define new PNEW

UIConsole::UIConsole(ControllerThread * _controller, Resources * _resources) :
    UI(_controller, _resources) {
    
    quit   = PFalse;
}

UIConsole::~UIConsole() {
}

void UIConsole::Initialize() {
    SDL_WM_GrabInput(SDL_GRAB_ON);
    cout << "XBOX console mode initialized" << endl;
}

void UIConsole::Main() {
    PString userinput;
    PRegularExpression regex_help("(\\?|h|help)", PRegularExpression::Extended|PRegularExpression::IgnoreCase);
    PRegularExpression regex_quit("(exit|quit)", PRegularExpression::Extended|PRegularExpression::IgnoreCase);
    PRegularExpression regex_play("(p|play)", PRegularExpression::Extended|PRegularExpression::IgnoreCase);
    PRegularExpression regex_setabs("(sa|setabs) [[:digit:]]+ [[:digit:]]+", PRegularExpression::Extended|PRegularExpression::IgnoreCase);
    PRegularExpression regex_setrel("(sr|setrel) [[:digit:]]+ [[:digit:]]+", PRegularExpression::Extended|PRegularExpression::IgnoreCase);
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
                 << "sr,setrel BYTE BYTE" << endl
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
        } else if (userinput.MatchesRegEx(regex_setrel)) {
            PStringArray parts = userinput.Tokenise(" ");
            if (parts[1].AsUnsigned()>255 || parts[2].AsUnsigned()>255) {
                cout << "ERROR: argument(BYTE) out of range, command: " << userinput << endl;
                continue;
            };
            commandSetRel(parts[1].AsUnsigned(), parts[2].AsUnsigned());
        } else if (userinput.MatchesRegEx(regex_setrel)) {
            PStringArray parts = userinput.Tokenise(" ");
            if (parts[1].AsUnsigned()>255) {
                cout << "ERROR: argument(BYTE) out of range, command: " << userinput << endl;
                continue;
            };
            BYTE value = commandGetAbs(parts[1].AsUnsigned());
            cout << value << endl;
        } else if (userinput.MatchesRegEx(regex_setrel)) {
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

void UIConsole::commandMouse() {
    SDL_Surface * screen;
    int x    = 0;
    int y    = 0;
    int maximum = 0;
    int steps = 1;

    cout << "enter mouse colibration mode" << endl;
    //Start SDL
    SDL_Init(SDL_INIT_EVERYTHING);
    screen = SDL_SetVideoMode(640, 240, 32, SDL_SWSURFACE);
    SDL_WM_GrabInput(SDL_GRAB_ON);
    SDL_PumpEvents();
    SDL_GetRelativeMouseState(&x, &y);
    for(int i = 0; i < 5000; i++) {
        if (i > 1) {
            SDL_PumpEvents();
            SDL_GetRelativeMouseState(&x, &y);
            if (x == 0 && y == 0) {
                // frequency hole
                steps++;
            } else if (steps != 1) { // steps == 1 may be buffered value, but we need real
                PTRACE(1, "X:" << x << " Y:" << y << " steps:" << steps);
                if (x < 0) {
                    x *= -1;
                };
                x = x/steps;
                if (x > maximum) {
                    PTRACE(1, "MAXIMUM X:" << x);
                    maximum = x;
                };
                y = y/steps;
                if (y < 0) {
                    y *= -1;
                };
                if (y > maximum) {
                    PTRACE(1, "MAXIMUM Y:" << y);
                    maximum = y;
                };
                steps = 1;
            };
            PThread::Sleep(1);
        };
    };
    cout << "maximum offset at ~1kHz: " << maximum << endl;
    SDL_WM_GrabInput(SDL_GRAB_OFF);
    SDL_FreeSurface(screen);
    // Quit SDL
    SDL_Quit();
    cout << "exit mouse colibration mode" << endl;
};

void UIConsole::commandReset() {
    controller->pushAction(0, (WORD)1);
};

void UIConsole::commandSetAbs(BYTE action, BYTE value) {
    controller->pushAction(action, value);
};

void UIConsole::commandSetRel(BYTE action, BYTE value) {
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

