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

#include "UIDefault.h"
#include "Controller.h"

#define new PNEW

UIDefault::UIDefault() {
    //Start SDL
    SDL_Init(SDL_INIT_EVERYTHING);

    hello  = NULL;
    screen = NULL;
    quit   = false;
    x      = 0;
    y      = 0;
}

UIDefault::UIDefault(ControllerThread * tcontroller) {
    //Start SDL
    SDL_Init(SDL_INIT_EVERYTHING);

    hello  = NULL;
    screen = NULL;
    quit   = false;
    x      = 0;
    y      = 0;
    controller = tcontroller;
}

UIDefault::~UIDefault() {
    //Free the loaded image
    SDL_FreeSurface(hello);
    //Quit SDL
    SDL_Quit();
}

void UIDefault::Initialize() {
    //Set up screen
    screen = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE);
    //If there was an error in setting up the screen
    if( screen == NULL ) {
        return;
    };
    //Set the window caption
    SDL_WM_SetCaption( "ENikiBeNiki", NULL ); 
    //Load image
    hello = SDL_LoadBMP("back.bmp");
    //Apply image to screen
    SDL_BlitSurface( hello, NULL, screen, NULL );
    //Update Screen
    SDL_Flip(screen);
}

void UIDefault::Main() {
    //While the user hasn't quit
    while( quit == false ) {
        Uint8 *keys = SDL_GetKeyState(NULL);
        if (keys[SDLK_ESCAPE] == SDL_PRESSED){
            quit = true;
        }
        //If there's an event to handle
        if(SDL_PollEvent(&event) == 1) {
            //If a key was pressed
            if( event.type == SDL_KEYDOWN )
            {
                //Set the proper message surface
                switch( event.key.keysym.sym )
                {
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
            else if( event.type == SDL_QUIT ) {
                //Quit the program
                quit = true;
            }
        }
        SDL_Delay(1);
    }
}

