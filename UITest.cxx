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

    hello  = NULL;
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
    //Free the loaded image
    SDL_FreeSurface(hello);
    //Quit SDL
    SDL_Quit();
}

void UITest::Initialize() {
    PString backgroundName("TestUI.bmp");
    PString dotName("dot.bmp");
    //Set up screen
    screen = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE);
    //If there was an error in setting up the screen
    if( screen == NULL ) {
        return;
    };
    //Set the window caption
    SDL_WM_SetCaption( "ENikiBeNiki", NULL ); 
    //Load image
    hello = resources->loadImage(backgroundName);
    dot = resources->loadImage(dotName);
    //Apply image to screen
    SDL_BlitSurface( hello, NULL, screen, NULL );
    //Update Screen
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
}

void UITest::Main() {
    while (quit == false) {
        if (SDL_PollEvent(&event) == 1) {
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                eventMouseDown();
            } else if (event.type == SDL_MOUSEMOTION) {
                eventMouseMotion();
            } else if (event.type == SDL_KEYDOWN) {
                eventKeyDown();
            } else if (event.type == SDL_QUIT) {
                eventQuit();
            };
        }
        SDL_Delay(1);
    }
}

void UITest::eventMouseDown() {
    if (fMouseGrab) {
        fMouseGrab = PFalse;
        SDL_WM_GrabInput(SDL_GRAB_OFF);
        SDL_ShowCursor(SDL_ENABLE);
    } else {
        // If the left mouse button was pressed
        if (event.button.button == SDL_BUTTON_LEFT) {
            // Get the mouse offsets
            int x = event.button.x;
            int y = event.button.y; 
            // If the mouse is over the button
            if ((x > boxMainField.x) && (x < boxMainField.x + boxMainField.w) &&
                    (y > boxMainField.y) && (y < boxMainField.y + boxMainField.h) ) {
                fMouseGrab = PTrue;
                SDL_WM_GrabInput(SDL_GRAB_ON);
                SDL_ShowCursor(SDL_DISABLE);
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
        // add constraint
        if (x < 190) {
            x = 190;
        } else if (x > 470) {
            x = 470;
        };
        if (y < 160) {
            y = 160;
        } else if (y > 440) {
            y = 440;
        };
        // subtract shift
        x -= 190;
        y -= 160;
        // normalize x,y
        signed char xN = (x * 255 / 280) - 128;
        signed char yN = (((y-280) * -1) * 255 / 280) - 128;
        PString xtext(xN);
        PString ytext(yN);
        SDL_Surface * xval = TTF_RenderText_Solid(font, xtext, textColor);
        SDL_Surface * yval = TTF_RenderText_Solid(font, ytext, textColor);
        //If there was an 
        if (xval == NULL) {
            PError << "error in rendering the text" << endl;
            return;
        };
        //Apply the images to the screen
        apply_surface( 0, 0, hello, screen, NULL );
        apply_surface( 260, 70, xval, screen, NULL );
        apply_surface( 420, 70, yval, screen, NULL );
        apply_surface( x + 190 - 10, y + 160 - 10, dot, screen, NULL );
        //Update the screen
        if( SDL_Flip( screen ) == -1 ) {
            return;
        }
        // Send command to controller
        controller->pushAction(0, (unsigned char)xN);
        controller->pushAction(1, (unsigned char)xY);
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

