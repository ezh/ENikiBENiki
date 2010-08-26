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
#include "UIXBoxBinding.h"

#define new PNEW

UIXBox::UIXBox(ControllerThread * _controller, Resources * _resources, PConfig * _config) :
    UI(_controller, _resources, _config) {
    //Start SDL
    SDL_Init(SDL_INIT_EVERYTHING);

    // global
    config = _config;
    screen = NULL;
    quit   = PFalse;
    active = PFalse;
    for(int i = 0; i < 32767; i++) {
        codeKeyToBinding[i] = 0;
    };
    // controls
/*    controlX1 = 0;
    controlY1 = 0;
    controlX2 = 0;
    controlY2 = 0;
    controlDU = PFalse;
    controlDD = PFalse;
    controlDL = PFalse;
    controlDR = PFalse;
    controlBack = PFalse;
    controlGuide = PFalse;
    controlStart = PFalse;
    controlTL = PFalse;
    controlTR = PFalse;
    controlA = PFalse;
    controlB = PFalse;
    controlX = PFalse;
    controlY = PFalse;
    controlLB = 0;
    controlRB = 0;
    controlLT = PFalse;
    controlRT = PFalse;*/
    // set button areas
}

UIXBox::~UIXBox() {
    // close UIXBoxBindings
    for(int i = 0; i < 32767; i++) {
        UIXBoxBinding* pUIXBoxBinding = (UIXBoxBinding*)codeKeyToBinding[i];
        if (codeKeyToBinding[i]) {
            if (!pUIXBoxBinding->IsSuspended() && !pUIXBoxBinding->IsTerminated()) {
                pUIXBoxBinding->Stop();
                pUIXBoxBinding->WaitForTermination();
            };
            delete pUIXBoxBinding;
            codeKeyToBinding[i] = 0;
        };
    };
    // Free the loaded image
    if (backgroundPassiveWaiting)
        SDL_FreeSurface(backgroundPassiveWaiting);
    if (backgroundPassiveReady)
        SDL_FreeSurface(backgroundPassiveReady);
    if (backgroundActiveDefault)
        SDL_FreeSurface(backgroundActiveDefault);
    if (screen)
        SDL_FreeSurface(screen);
    // Quit SDL
    SDL_Quit();
}

bool UIXBox::Initialize() {
    PString backgroundPassiveWaitingName("XBoxUI/passiveWaiting.bmp");
    PString backgroundPassiveReadyName("XBoxUI/passiveReady.bmp");
    PString backgroundActiveDefaultName("XBoxUI/activeDefault.bmp");

    //Set up screen
    screen = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE);
    //If there was an error in setting up the screen
    if( screen == NULL ) {
        PError << "an error in setting up the screen" << endl;
        return PFalse;
    };
    // set the window caption
    SDL_WM_SetCaption( "ENikiBeNiki", NULL ); 
    // load images
    backgroundPassiveWaiting = resources->LoadImageOptimized(backgroundPassiveWaitingName);
    if (!backgroundPassiveWaiting) {
        PError << "an error loading " << backgroundPassiveWaitingName << endl;
        return PFalse;
    };
    backgroundPassiveReady = resources->LoadImageOptimized(backgroundPassiveReadyName);
    if (!backgroundPassiveReady) {
        PError << "an error loading " << backgroundPassiveReadyName << endl;
        return PFalse;
    };
    backgroundActiveDefault = resources->LoadImageOptimized(backgroundActiveDefaultName);
    if (!backgroundActiveDefault) {
        PError << "an error loading " << backgroundActiveDefaultName << endl;
        return PFalse;
    };
    // SDL keys
    RegisterKey("backspace", SDLK_BACKSPACE);
    RegisterKey("tab",       SDLK_TAB);
    RegisterKey("clear",     SDLK_CLEAR);
    RegisterKey("enter",     SDLK_RETURN);
    RegisterKey("return",    SDLK_RETURN);
    RegisterKey("pause",     SDLK_PAUSE);
    RegisterKey("esc",       SDLK_ESCAPE);
    RegisterKey("escape",    SDLK_ESCAPE);
    RegisterKey("space",     SDLK_SPACE);
    RegisterKey("delete",    SDLK_DELETE);
    // ASCII mapped keysyms
    for (unsigned char i = ' '; i <= '~'; ++i) {
        if (!isupper(i)) {
            RegisterKey(std::string(1, i), i);
        };
    };
    /* Numeric keypad */
    RegisterKey("numpad0", SDLK_KP0);
    RegisterKey("numpad1", SDLK_KP1);
    RegisterKey("numpad2", SDLK_KP2);
    RegisterKey("numpad3", SDLK_KP3);
    RegisterKey("numpad4", SDLK_KP4);
    RegisterKey("numpad5", SDLK_KP5);
    RegisterKey("numpad6", SDLK_KP6);
    RegisterKey("numpad7", SDLK_KP7);
    RegisterKey("numpad8", SDLK_KP8);
    RegisterKey("numpad9", SDLK_KP9);
    RegisterKey("numpad.", SDLK_KP_PERIOD);
    RegisterKey("numpad/", SDLK_KP_DIVIDE);
    RegisterKey("numpad*", SDLK_KP_MULTIPLY);
    RegisterKey("numpad-", SDLK_KP_MINUS);
    RegisterKey("numpad+", SDLK_KP_PLUS);
    RegisterKey("numpad=", SDLK_KP_EQUALS);
    RegisterKey("numpad_enter", SDLK_KP_ENTER);
    /* Arrows + Home/End pad */
    RegisterKey("up",       SDLK_UP);
    RegisterKey("down",     SDLK_DOWN);
    RegisterKey("right",    SDLK_RIGHT);
    RegisterKey("left",     SDLK_LEFT);
    RegisterKey("insert",   SDLK_INSERT);
    RegisterKey("home",     SDLK_HOME);
    RegisterKey("end",      SDLK_END);
    RegisterKey("pageup",   SDLK_PAGEUP);
    RegisterKey("pagedown", SDLK_PAGEDOWN);
    /* Function keys */
    RegisterKey("f1",  SDLK_F1);
    RegisterKey("f2",  SDLK_F2);
    RegisterKey("f3",  SDLK_F3);
    RegisterKey("f4",  SDLK_F4);
    RegisterKey("f5",  SDLK_F5);
    RegisterKey("f6",  SDLK_F6);
    RegisterKey("f7",  SDLK_F7);
    RegisterKey("f8",  SDLK_F8);
    RegisterKey("f9",  SDLK_F9);
    RegisterKey("f10", SDLK_F10);
    RegisterKey("f11", SDLK_F11);
    RegisterKey("f12", SDLK_F12);
    RegisterKey("f13", SDLK_F13);
    RegisterKey("f14", SDLK_F14);
    RegisterKey("f15", SDLK_F15);
    RegisterKey("shift", SDLK_LSHIFT);
    RegisterKey("ctrl",  SDLK_LCTRL);
    RegisterKey("alt",   SDLK_LALT);
    RegisterKey("meta",  SDLK_LMETA);
    // special values
    RegisterKey("mouse_axis0",  MOUSE_N0);
    RegisterKey("mouse_axis1",  MOUSE_N1);
    RegisterKey("mouse_axis2",  MOUSE_N2);
    RegisterKey("mouse_axis3",  MOUSE_N3);
    RegisterKey("mouse_axis4",  MOUSE_N4);
    RegisterKey("mouse_axis5",  MOUSE_N5);
    RegisterKey("mouse_axis6",  MOUSE_N6);
    RegisterKey("mouse_axis7",  MOUSE_N7);
    RegisterKey("mouse_axis8",  MOUSE_N8);
    RegisterKey("mouse_axis9",  MOUSE_N9);
    RegisterKey("mouse_button0",MOUSE_B0);
    RegisterKey("mouse_button1",MOUSE_B1);
    RegisterKey("mouse_button2",MOUSE_B2);
    RegisterKey("mouse_button3",MOUSE_B3);
    RegisterKey("mouse_button4",MOUSE_B4);
    RegisterKey("mouse_button5",MOUSE_B5);
    RegisterKey("mouse_button6",MOUSE_B6);
    RegisterKey("mouse_button7",MOUSE_B7);
    RegisterKey("mouse_button8",MOUSE_B8);
    RegisterKey("mouse_button9",MOUSE_B9);
    PStringArray keys = config->GetKeys("Bindings");
    if (keys.GetSize() == 0) {
        PError << "binding lost in space... fix it before continue" << endl;
        return PFalse;
    };
    for (PINDEX i = 0; i < keys.GetSize(); i++) {
        PTRACE(1, "Key " << (i + 1) << " of " << keys.GetSize() << " is " << keys[i]);
        if (keyNameToCode.find(keys[i]) != keyNameToCode.end()) {
            BindKeyToClass(keyNameToCode[keys[i]],
                    new UIXBoxBinding(keys[i], keyNameToCode[keys[i]], config->GetString("Bindings", keys[i], ""), this));
        } else {
            PError << "unknown key in section [Bindings]: " << keys[i] << endl;
        };
    };
    return PTrue;
}

void UIXBox::Main() {
    apply_surface(0, 0, backgroundPassiveWaiting, screen, NULL);
    if (SDL_Flip(screen) == -1) { // update the screen
        return;
    };
    // wait for controller
    while (!controller->isReady()) {
        PThread::Sleep(100);
    };
    UpdateUIAndControls();
    while (quit == false) {
        if (SDL_WaitEvent(&event) == 1) {
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
                } else if (event.type == SDL_KEYUP) {
                    eventKeyUp();
                } else if (event.type == SDL_QUIT) {
                    eventQuit();
                };
            };
        };
    };
}

void UIXBox::eventMouseUp() {
    if (active) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            UIXBoxBinding* pUIXBoxBinding = (UIXBoxBinding*)codeKeyToBinding[MOUSE_B0];
            pUIXBoxBinding->SomethingEnd(event);
        } else if (event.button.button == SDL_BUTTON_RIGHT) {
            UIXBoxBinding* pUIXBoxBinding = (UIXBoxBinding*)codeKeyToBinding[MOUSE_B2];
            pUIXBoxBinding->SomethingEnd(event);
        };
    } else {
        if (event.button.button == SDL_BUTTON_LEFT) {
            // If the left mouse button was pressed
            SDL_WM_GrabInput(SDL_GRAB_ON);
            SDL_ShowCursor(SDL_DISABLE);
            active = PTrue;
            UpdateUIAndControls();
            return;
        };
    };
}

void UIXBox::eventMouseDown() {
    if (active) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            UIXBoxBinding* pUIXBoxBinding = (UIXBoxBinding*)codeKeyToBinding[MOUSE_B0];
            pUIXBoxBinding->SomethingBegin(event);
        } else if (event.button.button == SDL_BUTTON_RIGHT) {
            UIXBoxBinding* pUIXBoxBinding = (UIXBoxBinding*)codeKeyToBinding[MOUSE_B2];
            pUIXBoxBinding->SomethingBegin(event);
        };
    } else {
        if (event.button.button == SDL_BUTTON_LEFT) {
            // If the left mouse button was pressed
            SDL_WM_GrabInput(SDL_GRAB_ON);
            SDL_ShowCursor(SDL_DISABLE);
            active = PTrue;
            UpdateUIAndControls();
            return;
        };
    };
}

void UIXBox::eventMouseMotion() {
    if (active) {
        UIXBoxBinding* pUIXBoxBindingX = (UIXBoxBinding*)codeKeyToBinding[MOUSE_N0];
        UIXBoxBinding* pUIXBoxBindingY = (UIXBoxBinding*)codeKeyToBinding[MOUSE_N1];
        if (pUIXBoxBindingX) {
            pUIXBoxBindingX->SomethingBegin(event);
        };
        if (pUIXBoxBindingY) {
            pUIXBoxBindingY->SomethingBegin(event);
        };
    };
}

void UIXBox::eventKeyDown() {
    if (active) {
        UIXBoxBinding* pUIXBoxBinding = (UIXBoxBinding*)codeKeyToBinding[event.key.keysym.sym];
        if (pUIXBoxBinding) {
            pUIXBoxBinding->SomethingBegin(event);
        };
    };
    if (event.key.keysym.sym == SDLK_ESCAPE) {
        if (active) {
            active = PFalse;
            SDL_ShowCursor(SDL_ENABLE);
            SDL_WM_GrabInput(SDL_GRAB_OFF);
            UpdateUIAndControls();
        } else {
            eventQuit();
        };
    };
}

void UIXBox::eventKeyUp() {
    if (active) {
        UIXBoxBinding* pUIXBoxBinding = (UIXBoxBinding*)codeKeyToBinding[event.key.keysym.sym];
        if (pUIXBoxBinding) {
            pUIXBoxBinding->SomethingEnd(event);
        };
    };
}

//If the user has Xed out the window
void UIXBox::eventQuit() {
    //Quit the program
    quit = true;
}

void UIXBox::apply_surface( int x, int y, SDL_Surface* source, SDL_Surface* destination, SDL_Rect* clip) {
    //Holds offsets
    SDL_Rect offset;

    //Get offsets
    offset.x = x;
    offset.y = y;

    //Blit
    SDL_BlitSurface( source, clip, destination, &offset );
}

void UIXBox::UpdateUIAndControls() {
    if (active) {
        apply_surface(0, 0, backgroundActiveDefault, screen, NULL);
        if (SDL_Flip(screen) == -1) { // update the screen
            return;
        };
    } else {
        apply_surface(0, 0, backgroundPassiveReady, screen, NULL);
        if (SDL_Flip(screen) == -1) { // update the screen
            return;
        };
    };
/*    int oldCrossX = crossX;
    int oldCrossY = crossY;
    // add constraint
    if (x <= boxMainField.x + 1) {
        x = boxMainField.x + 2;
    } else if (x > boxMainField.x + boxMainField.w) {
        x = boxMainField.x + boxMainField.w;
    };
    if (y < boxMainField.y) {
        y = boxMainField.y;
    } else if (y >= 439) {
        y = 438;
    };
    // apply the images to the screen
    apply_surface(0, 0, backgroundActiveDefault, screen, NULL);
    // arrow
    apply_surface(x - arrowTop->w/2, arrowOffsetY - arrowTop->h/2, arrowTop, screen, NULL );
    apply_surface(arrowOffsetX - arrowRight->w/2, y - arrowRight->h/2, arrowRight, screen, NULL );
    // allply led
    if (ledStatus) {
        apply_surface(0, 0, ledOn, screen, NULL);
    };
    // crosshair
    if (nMouseState == 1) {
        // set cross coorinates
        crossX = x;
        crossY = y;
        apply_surface(x - crosshairOn->w/2 - 1, y - crosshairOn->h/2 + 1, crosshairOn, screen, NULL);
    } else if (nMouseState == 2) {
        crossX = x;
        apply_surface(x - crosshairOff->w/2 - 1, y - crosshairOff->h/2 + 1, crosshairOff, screen, NULL);
    } else if (nMouseState == 3) {
        crossY = y;
        apply_surface(x - crosshairOff->w/2 - 1, y - crosshairOff->h/2 + 1, crosshairOff, screen, NULL);
    } else {
        crossX = x;
        crossY = y;
        apply_surface(x - crosshairOff->w/2 - 1, y - crosshairOff->h/2 + 1, crosshairOff, screen, NULL);
    };
    // normalize x,y for controller
    signed char xN = ((crossX - boxMainField.x) * 255 / 280) - 128;
    signed char yN = ((((crossY - boxMainField.y)-280) * -1) * 255 / 280) - 128;
    // numbers
    apply_surface(260, 70, digitals[xN+128], screen, NULL);
    apply_surface(410, 70, digitals[yN+128], screen, NULL);
    // controller
    if (oldCrossX != crossX)
        controller->pushAction(1, (BYTE)xN);
    if (oldCrossY != crossY)
        controller->pushAction(2, (BYTE)yN);
    // update the screen
    if (SDL_Flip(screen) == -1) {
        return;
    };*/
}

void UIXBox::RegisterKey(const std::string& name, int code) {
    if (keyNameToCode.find(name) == keyNameToCode.end()) {
        keyNameToCode[name] = code;
    };
    if (keyCodeToName.find(code) == keyCodeToName.end()) {
        keyCodeToName[code] = name;
    };
};

void UIXBox::BindKeyToClass(int code, void* pUIXBoxBinding) {
    PTRACE(4, "Register binding '" << ((UIXBoxBinding*)pUIXBoxBinding)->GetThreadName() << "' code " << code);
    codeKeyToBinding[code] = pUIXBoxBinding;
}

// End of File ///////////////////////////////////////////////////////////////
// vim:ft=c:ts=4:sw=4
