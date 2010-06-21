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

#include "UITest.h"

#define new PNEW

UITest::UITest(ControllerThread * _controller, Resources * _resources) :
    UI(_controller, _resources) {
    //Start SDL
    SDL_Init(SDL_INIT_EVERYTHING);

    screen = NULL;
    quit   = false;
    crossX = 0;
    crossY = 0;
    nMouseState = 0;
    // set main field parameters
    boxMainField.x = 190;
    boxMainField.y = 160;
    boxMainField.w = 280;
    boxMainField.h = 280;
 //   fontName = "Vera.ttf";
    textColor.r = (BYTE)0;
    textColor.g = (BYTE)0;
    textColor.b = (BYTE)0;
    // arrow
    arrowOffsetX = 496;
    arrowOffsetY = 134;
    // led
    ledStatus = PFalse;
}

UITest::~UITest() {
    // Free the loaded image
    SDL_FreeSurface(background);
    SDL_FreeSurface(ledOn);
    SDL_FreeSurface(crosshairOn);
    SDL_FreeSurface(crosshairOff);
    SDL_FreeSurface(arrowTop);
    SDL_FreeSurface(arrowRight);
    // Quit SDL
    SDL_Quit();
}

void UITest::Initialize() {
    PString backgroundName("TestUI/background.bmp");
    PString ledOnName("TestUI/ledOn.bmp");
    PString crosshairOnName("TestUI/crosshairOn.bmp");
    PString crosshairOffName("TestUI/crosshairOff.bmp");
    PString arrowTopName("TestUI/arrow_top.bmp");
    PString arrowRightName("TestUI/arrow_right.bmp");
    //Set up screen
    screen = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE);
    //If there was an error in setting up the screen
    if( screen == NULL ) {
        PError << "an error in setting up the screen" << endl;
        return;
    };
    // set the window caption
    SDL_WM_SetCaption( "ENikiBeNiki", NULL ); 
    // load images
    background   = resources->LoadImageOptimized(backgroundName);
    if (!background) {
        PError << "an error loading" << endl;
        return;
    }
    ledOn        = resources->LoadImageOptimized(ledOnName);
    crosshairOn  = resources->LoadImageOptimized(crosshairOnName);
    crosshairOff = resources->LoadImageOptimized(crosshairOffName);
    arrowTop     = resources->LoadImageOptimized(arrowTopName);
    arrowRight   = resources->LoadImageOptimized(arrowRightName);
    // set color keys
    Uint32 colorkey_crosshairOn = SDL_MapRGB(crosshairOn->format, 0xE5, 0xE5, 0xE5); // Map the color key    
    SDL_SetColorKey(crosshairOn, SDL_SRCCOLORKEY, colorkey_crosshairOn); // Set all pixels of color R 0xFF, G 0xFF, B 0xFF to be transparent
    Uint32 colorkey_crosshairOff = SDL_MapRGB(crosshairOff->format, 0xE5, 0xE5, 0xE5); // Map the color key    
    SDL_SetColorKey(crosshairOff, SDL_SRCCOLORKEY, colorkey_crosshairOff); // Set all pixels of color R 0xFF, G 0xFF, B 0xFF to be transparent
    Uint32 colorkey_arrowTop = SDL_MapRGB(arrowTop->format, 0xE5, 0xE5, 0xE5); // Map the color key    
    SDL_SetColorKey(arrowTop, SDL_SRCCOLORKEY, colorkey_arrowTop); // Set all pixels of color R 0xFF, G 0xFF, B 0xFF to be transparent
    Uint32 colorkey_arrowRight = SDL_MapRGB(arrowRight->format, 0xE5, 0xE5, 0xE5); // Map the color key    
    SDL_SetColorKey(arrowRight, SDL_SRCCOLORKEY, colorkey_arrowRight); // Set all pixels of color R 0xFF, G 0xFF, B 0xFF to be transparent
    SDL_Flip(screen);
    if (0 > TTF_Init()) {
        PError << "TTF_Init() failed" << endl;
        TTF_Quit();
        return;
    };
    if (NULL == (font = TTF_OpenFont("Vera.ttf", 20))) {
        PError << "Font '" << "Vera.ttf" << "'failed" << endl;
        return;
    };
    for (int i = 0; i < 256;i++) {
        PString text(i-128);
        SDL_Surface* generatedImage = TTF_RenderText_Solid(font, text, textColor);
        if (generatedImage == NULL) {
            PError << "error in rendering the text" << endl;
            return;
        };
        //Create an optimized image
        digitals[i] = SDL_DisplayFormat(generatedImage);
        //Free the old surface
        SDL_FreeSurface(generatedImage);
    };
    UpdateUIAndControls(331, 299); //center
}

void UITest::Main() {
    while (quit == false) {
        if (SDL_WaitEvent(&event) == 1) {
            // Check for other mouse motion events in the queue.
            SDL_Event eventFuture;
            int num = SDL_PeepEvents( &eventFuture, 1, SDL_PEEKEVENT, SDL_ALLEVENTS );
            // If this is the same state, ignore this one
            if (not (num > 0 && eventFuture.type == SDL_MOUSEMOTION &&
                        eventFuture.motion.state == event.motion.state )) {
                if (event.type == SDL_MOUSEBUTTONDOWN) {
                    eventMouseDown();
                } else if (event.type == SDL_MOUSEMOTION) {
                    eventMouseMotion();
                } else if (event.type == SDL_KEYDOWN) {
                    eventKeyDown();
                } else if (event.type == SDL_QUIT) {
                    eventQuit();
                };
            };
        }
    }
}

void UITest::eventMouseDown() {
    int x = event.button.x;
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
    };
}

void UITest::eventMouseMotion() {
    int x = event.motion.x;
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
    };
}

void UITest::eventKeyDown() {
    // Set the proper message surface
    switch (event.key.keysym.sym)
    {
        case SDLK_ESCAPE:
            eventQuit();
            break;
        case SDLK_UP:
            UpdateUIAndControls(crossX, crossY-1);
            break;
        case SDLK_DOWN:
            UpdateUIAndControls(crossX, crossY+1);
            break;
        case SDLK_LEFT:
            UpdateUIAndControls(crossX-1, crossY);
            break;
        case SDLK_RIGHT:
            UpdateUIAndControls(crossX+1, crossY);
            break;
        default:
            break;
    }
    fflush(stdout);
}

//If the user has Xed out the window
void UITest::eventQuit() {
    //Quit the program
    quit = true;
}
void UITest::apply_surface( int x, int y, SDL_Surface* source, SDL_Surface* destination, SDL_Rect* clip) {
    //Holds offsets
    SDL_Rect offset;

    //Get offsets
    offset.x = x;
    offset.y = y;

    //Blit
    SDL_BlitSurface( source, clip, destination, &offset );
}

void UITest::UpdateUIAndControls(int x, int y) {
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
    apply_surface(0, 0, background, screen, NULL);
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
    controller->pushAction(1, (BYTE)xN);
    controller->pushAction(2, (BYTE)yN);
    // update the screen
    if (SDL_Flip(screen) == -1) {
        return;
    };
}

