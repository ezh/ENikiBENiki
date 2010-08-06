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

UIXBox::UIXBox(ControllerThread * _controller, Resources * _resources, PConfig * _config) :
    UI(_controller, _resources, _config) {
    //Start SDL
    SDL_Init(SDL_INIT_EVERYTHING);

    // global
    screen = NULL;
    quit   = PFalse;
    active = PFalse;
    for(int i = 0; i < 32767; i++) {
        codeKeyToClass[i] = 0;
    };
    // controls
    controlX1 = 0;
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
    controlRT = PFalse;
    // set button areas
}

UIXBox::~UIXBox() {
    // close UIXBoxBindings
    for(int i = 0; i < 32767; i++) {
        UIXBoxBinding* pUIXBoxBinding = (UIXBoxBinding*)codeKeyToClass[i];
        if (codeKeyToClass[i]) {
            if (!pUIXBoxBinding->IsSuspended() && !pUIXBoxBinding->IsTerminated()) {
                pUIXBoxBinding->Stop();
                pUIXBoxBinding->WaitForTermination();
            };
            delete pUIXBoxBinding;
            codeKeyToClass[i] = 0;
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
    //PString ledOnName("TestUI/ledOn.bmp");
    //PString crosshairOnName("TestUI/crosshairOn.bmp");
    //PString crosshairOffName("TestUI/crosshairOff.bmp");
    //PString arrowTopName("TestUI/arrow_top.bmp");
    //PString arrowRightName("TestUI/arrow_right.bmp");

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
//    ledOn        = resources->LoadImageOptimized(ledOnName);
//    crosshairOn  = resources->LoadImageOptimized(crosshairOnName);
//    crosshairOff = resources->LoadImageOptimized(crosshairOffName);
//    arrowTop     = resources->LoadImageOptimized(arrowTopName);
//    arrowRight   = resources->LoadImageOptimized(arrowRightName);
    // set color keys
/*    Uint32 colorkey_crosshairOn = SDL_MapRGB(crosshairOn->format, 0xE5, 0xE5, 0xE5); // Map the color key    
    SDL_SetColorKey(crosshairOn, SDL_SRCCOLORKEY, colorkey_crosshairOn); // Set all pixels of color R 0xFF, G 0xFF, B 0xFF to be transparent
    Uint32 colorkey_crosshairOff = SDL_MapRGB(crosshairOff->format, 0xE5, 0xE5, 0xE5); // Map the color key    
    SDL_SetColorKey(crosshairOff, SDL_SRCCOLORKEY, colorkey_crosshairOff); // Set all pixels of color R 0xFF, G 0xFF, B 0xFF to be transparent
    Uint32 colorkey_arrowTop = SDL_MapRGB(arrowTop->format, 0xE5, 0xE5, 0xE5); // Map the color key    
    SDL_SetColorKey(arrowTop, SDL_SRCCOLORKEY, colorkey_arrowTop); // Set all pixels of color R 0xFF, G 0xFF, B 0xFF to be transparent
    Uint32 colorkey_arrowRight = SDL_MapRGB(arrowRight->format, 0xE5, 0xE5, 0xE5); // Map the color key    
    SDL_SetColorKey(arrowRight, SDL_SRCCOLORKEY, colorkey_arrowRight); // Set all pixels of color R 0xFF, G 0xFF, B 0xFF to be transparent*/
    if (0 > TTF_Init()) {
        PError << "TTF_Init() failed" << endl;
        TTF_Quit();
        return PFalse;
    };
/*    if (NULL == (font = TTF_OpenFont("Vera.ttf", 20))) {
        PError << "Font '" << "Vera.ttf" << "'failed" << endl;
        return PFalse;
    };*/
/*    for (int i = 0; i < 256;i++) {
        PString text(i-128);
        SDL_Surface* generatedImage = TTF_RenderText_Solid(font, text, textColor);
        if (generatedImage == NULL) {
            PError << "error in rendering the text" << endl;
            return PFalse;
        };
        //Create an optimized image
        digitals[i] = SDL_DisplayFormat(generatedImage);
        //Free the old surface
        SDL_FreeSurface(generatedImage);
    };*/
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
    RegisterKey("mouse_x",  -1);
    RegisterKey("mouse_y",  -2);
    RegisterKey("mouse_n1", -3);
    RegisterKey("mouse_n2", -4);
    RegisterKey("mouse_n3", -5);
    RegisterKey("mouse_n4", -6);
    RegisterKey("mouse_n5", -7);
    RegisterKey("mouse_0",  -8);
    RegisterKey("mouse_1",  -9);
    RegisterKey("mouse_2", -10);
    RegisterKey("mouse_3", -11);
    RegisterKey("mouse_4", -12);
    RegisterKey("mouse_5", -13);
    RegisterKey("mouse_6", -14);
    RegisterKey("mouse_7", -15);
    RegisterKey("mouse_8", -16);
    RegisterKey("mouse_9", -17);
    PStringArray keys = config->GetKeys("Bindings");
    if (keys.GetSize() == 0) {
        PError << "binding lost in space... fix it before continue" << endl;
        return PFalse;
    };
    for (PINDEX i = 0; i < keys.GetSize(); i++) {
        PTRACE(1, "Key " << (i + 1) << " of " << keys.GetSize() << " is " << keys[i]);
        if (keyNameToCode.find(keys[i]) != keyNameToCode.end()) {
            BindKeyToClass(keyNameToCode[keys[i]],
                    new UIXBoxBinding(keys[i], config->GetString("Bindings", keys[i], ""), controller));
        } else {
            PError << "unknown key in section [Bindings]: " << keys[i] << endl;
        };
    };
/*    const char *lua_code = "print(SLB.ui)\n";
    // Custom SLB::SCript, a simplification to use SLB
    SLB::Script s;
    s.doString(lua_code);
     but you can include SLB features on any lua_State
    // Create a lua State, using normal lua API
    lua_State *L = luaL_newstate();
    // load default functions (the current example uses print)
    // and by default, SLB::Script does this.
    luaL_openlibs(L);
    // Register SLB inside the lua_State, calling the SLB::Manager
    // that handles bindings, default functions, default values...
    SLB::Manager::getInstance().registerSLB(L);
    // No call lua API to execute the same code as above
    luaL_dostring(L, lua_code); // execute code*/

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
    };
/*    if (nMouseState == 1 && event.button.button == SDL_BUTTON_LEFT && jumpToCenter) {
        // release left button
        SDL_WM_GrabInput(SDL_GRAB_OFF);
        nMouseState = 0;
        UpdateUIAndControls(331, 299); //center
    };*/
}

void UIXBox::eventMouseDown() {
    if (active) {
    } else {
        if (event.button.button == SDL_BUTTON_LEFT) {
            // If the left mouse button was pressed
            SDL_WM_GrabInput(SDL_GRAB_ON);
            active = 1;
            UpdateUIAndControls();
            return;
        };
    };
/*    int x = event.button.x;
    int y = event.button.y; 
    if (nMouseState == 0 && event.button.button == SDL_BUTTON_RIGHT) {
        UpdateUIAndControls(331, 299); //center
        return;
    };
    if (event.button.button == SDL_BUTTON_LEFT) {
        PTRACE(5, "eventMouseDown\tclick left x:" << x << " y:" << y);
    };
    if (nMouseState > 0 && event.button.button == SDL_BUTTON_LEFT) {
        // If the left mouse button was pressed
        SDL_WM_GrabInput(SDL_GRAB_OFF);
        //SDL_ShowCursor(SDL_ENABLE);
        if (nMouseState == 1) {
            // main field
            nMouseState = 0;
            UpdateUIAndControls(x, y);
        } else if (nMouseState == 2) {
            // scroll X
            UpdateUIAndControls(x, crossY);
        } else if (nMouseState == 3) {
            // scroll Y
            UpdateUIAndControls(crossX, y);
        };
        nMouseState = 0;
        return;
    };
    if (nMouseState == 0) {
        // If the mouse is over the main field
        if ((x > boxMainField.x) && (x < boxMainField.x + boxMainField.w) &&
                (y > boxMainField.y) && (y < boxMainField.y + boxMainField.h) ) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                nMouseState = 1;
                //SDL_WM_GrabInput(SDL_GRAB_ON);
                //SDL_ShowCursor(SDL_DISABLE);
                UpdateUIAndControls(x, y);
            };
            return;
        };
        // If mouse is over the x scroller
        if ((x > crossX - arrowTop->w/2) && (x < crossX + arrowTop->w/2) &&
                (y > arrowOffsetY - arrowTop->h/2) && (y < arrowOffsetY + arrowTop->h/2)) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                nMouseState = 2;
                //SDL_WM_GrabInput(SDL_GRAB_ON);
                //SDL_ShowCursor(SDL_DISABLE);
                UpdateUIAndControls(x, crossY);
            };
        };
        // If mouse is over the y scroller
        if ((x > arrowOffsetX - arrowRight->w/2) && (x < arrowOffsetX + arrowRight->w/2) &&
                (y > crossY - arrowRight->h/2) && (y < crossY + arrowRight->h/2)) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                nMouseState = 3;
                //SDL_WM_GrabInput(SDL_GRAB_ON);
                //SDL_ShowCursor(SDL_DISABLE);
                UpdateUIAndControls(crossX, y);
            };
        };
        // If the mouse is over led
        if ((x > 100) && (x < 150) &&
                (y > 10) && (y < 60) ) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                if (ledStatus) {
                    // turn led OFF
                    ledStatus = PFalse;
                    controller->pushAction(5, (BYTE)0);
                    UpdateUIAndControls(crossX, crossY);
                } else {
                    // turn led ON
                    ledStatus = PTrue;
                    controller->pushAction(5, (BYTE)1);
                    UpdateUIAndControls(crossX, crossY);
                };
            };
            return;
        };
        // If mouse is over jumpTo checkbox
        if ((x > 540) && (x < 600) &&
                (y > 365) && (y < 425) ) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                if (jumpToCenter) {
                    // turn led OFF
                    jumpToCenter = PFalse;
                } else {
                    // turn led ON
                    jumpToCenter = PTrue;
                };
                UpdateUIAndControls(crossX, crossY);
            };
            return;
        };
    };*/
}

void UIXBox::eventMouseMotion() {
/*    int x = event.motion.x;
    int y = event.motion.y;
    if (nMouseState == 1) {
        // main field
        UpdateUIAndControls(x, y);
    } else if (nMouseState == 2) {
        // scroll X
        UpdateUIAndControls(x, crossY);
    } else if (nMouseState == 3) {
        // scroll Y
        UpdateUIAndControls(crossX, y);
    };*/
}

void UIXBox::eventKeyDown() {
    if (active) {
        UIXBoxBinding* pUIXBoxBinding = (UIXBoxBinding*)codeKeyToClass[event.key.keysym.sym];
        if (pUIXBoxBinding) {
            pUIXBoxBinding->SomethingBegin(event);
        };
    };
    if (event.key.keysym.sym == SDLK_ESCAPE) {
        if (active) {
            active = PFalse;
            SDL_WM_GrabInput(SDL_GRAB_OFF);
            UpdateUIAndControls();
        } else {
            eventQuit();
        };
    };
}

void UIXBox::eventKeyUp() {
    if (active) {
        UIXBoxBinding* pUIXBoxBinding = (UIXBoxBinding*)codeKeyToClass[event.key.keysym.sym];
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
    codeKeyToClass[code] = pUIXBoxBinding;
}

// End of File ///////////////////////////////////////////////////////////////
// vim:ft=c:ts=4:sw=4
