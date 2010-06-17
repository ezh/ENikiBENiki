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

#include "SDL.h"
#include "SDL_ttf.h" 

#include "UI.h"

#ifndef _UITest_H_
#define _UITest_H_

class UITest : public UI {
    public:
        UITest(ControllerThread * _controller, Resources * _resources);
        ~UITest();
        void Initialize();
        void Main();
    private:
        void eventMouseDown();
        void eventMouseMotion();
        void eventKeyDown();
        void eventQuit();
        void apply_surface(int x,int y, SDL_Surface* source, SDL_Surface* destination, SDL_Rect* clip);
        //The images
        SDL_Surface * hello;
        SDL_Surface * dot;
        SDL_Surface * screen;
        //The event structure that will be used
        SDL_Event event;
        //Make sure the program waits for a quit
        bool quit;
        // x y
        signed short x;
        signed short y;
        // grab flag
        bool fMouseGrab;
        // control box
        SDL_Rect boxMainField; 
        //PString fontName;
        TTF_Font * font;
        SDL_Color textColor;
};

#endif  // _UITest_H

// End of File ///////////////////////////////////////////////////////////////
// vim:ft=c:ts=4:sw=4
