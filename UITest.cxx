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
    x      = 0;
    y      = 0;
    fMouseGrab = PFalse;
    // set main field parameters
    boxMainField.x = 190;
    boxMainField.y = 160;
    boxMainField.w = 280;
    boxMainField.h = 280;
 //   fontName = "Vera.ttf";
    textColor.r = (BYTE)0;
    textColor.g = (BYTE)0;
    textColor.b = (BYTE)0;
}

UITest::~UITest() {
    // Free the loaded image
    SDL_FreeSurface(background);
    SDL_FreeSurface(crosshairOn);
    SDL_FreeSurface(crosshairOff);
    SDL_FreeSurface(arrowTop);
    SDL_FreeSurface(arrowRight);
    // Quit SDL
    SDL_Quit();
}

void UITest::Initialize() {
    PString backgroundName("TestUI/background.bmp");
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
    UpdateUIAndControls(331, 299);
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
    if (fMouseGrab) {
        // If the left mouse button was pressed
        if (event.button.button == SDL_BUTTON_LEFT) {
            fMouseGrab = PFalse;
            SDL_WM_GrabInput(SDL_GRAB_OFF);
            //SDL_ShowCursor(SDL_ENABLE);
            UpdateUIAndControls(x, y);
        };
    } else {
        // If the left mouse button was pressed
        if (event.button.button == SDL_BUTTON_LEFT) {
            // Get the mouse offsets
            // If the mouse is over the button
            if ((x > boxMainField.x) && (x < boxMainField.x + boxMainField.w) &&
                    (y > boxMainField.y) && (y < boxMainField.y + boxMainField.h) ) {
                fMouseGrab = PTrue;
                SDL_WM_GrabInput(SDL_GRAB_ON);
                //SDL_ShowCursor(SDL_DISABLE);
                UpdateUIAndControls(x, y);
            };
        };
    };
}

void UITest::eventMouseMotion() {
    int x;
    int y;
    if (fMouseGrab) {
        // Get the mouse offsets
        x = event.motion.x;
        y = event.motion.y;
        UpdateUIAndControls(x, y);
        // Send command to controller
        //controller->pushAction(0, (unsigned char)xN);
        //controller->pushAction(1, (unsigned char)xY);
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
            y+=5;
            cout << "UP y: " << (int)y << endl;
            controller->pushAction(3, y);
            break;
        case SDLK_DOWN:
            y-=5;
            cout << "DOWN y: " << (int)y << endl;
            controller->pushAction(3, y);
            break;
        case SDLK_LEFT:
            x-=5;
            cout << "LEFT x: " << (int)x << endl;
            controller->pushAction(2, x);
            break;
        case SDLK_RIGHT:
            x+=5;
            cout << "RIGHT x: " << (int)x << endl;
            controller->pushAction(2, x);
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
    if (x <= 191) {
        x = 192;
    } else if (x > 470) {
        x = 470;
    };
    if (y < 160) {
        y = 160;
    } else if (y >= 439) {
        y = 438;
    };
    // subtract shift
    x -= 190;
    y -= 160;
    // normalize x,y
    signed char xN = (x * 255 / 280) - 128;
    signed char yN = (((y-280) * -1) * 255 / 280) - 128;
    // apply the images to the screen
    apply_surface(0, 0, background, screen, NULL);
    apply_surface(260, 70, digitals[xN+128], screen, NULL);
    apply_surface(410, 70, digitals[yN+128], screen, NULL);
    apply_surface(x - 140 + 298, 102, arrowTop, screen, NULL );
    apply_surface(464, y - 140 + 269, arrowRight, screen, NULL );
    if (fMouseGrab) {
        apply_surface(x + 190 - 21, y + 160 - 19, crosshairOn, screen, NULL);
    } else {
        apply_surface(x + 190 - 21, y + 160 - 19, crosshairOff, screen, NULL);
    }
    // update the screen
    if (SDL_Flip(screen) == -1) {
        return;
    }
}


