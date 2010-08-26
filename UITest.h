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

#include "SDL.h"
#include "SDL_ttf.h" 

#include "UI.h"

#ifndef _UITest_H_
#define _UITest_H_

#undef main

class UITest : public UI {
    public:
        UITest(ControllerThread * _controller, Resources * _resources, PConfig * _config);
        virtual ~UITest();
        bool Initialize();
        void Main();
    private:
        void UpdateUIAndControls(int x, int y);
        void eventMouseUp();
        void eventMouseDown();
        void eventMouseMotion();
        void eventKeyDown();
        void eventQuit();
        void apply_surface(int x,int y, SDL_Surface* source, SDL_Surface* destination, SDL_Rect* clip);
        // x y
        int crossX;
        int crossY;
        //The images
        SDL_Surface * backgroundJumpOff;
        SDL_Surface * backgroundJumpOn;
        SDL_Surface * ledOn;
        SDL_Surface * crosshairOn;
        SDL_Surface * crosshairOff;
        SDL_Surface * arrowTop;
        SDL_Surface * arrowRight;
        SDL_Surface * screen;
        SDL_Surface * digitals[256];
        // arrows
        int arrowOffsetX;
        int arrowOffsetY;
        //The event structure that will be used
        SDL_Event event;
        //Make sure the program waits for a quit
        bool quit;
        /*
         * grab flag
         * 0 - none
         * 1 - main field
         * 2 - scrollX
         * 3 - scrollY
         */
        int nMouseState;
        // boolen jump to center logic
        bool jumpToCenter;
        // led
        bool ledStatus;
        // control box
        SDL_Rect boxMainField;
        //PString fontName;
        TTF_Font * font;
        SDL_Color textColor;
};

#endif  // _UITest_H

// End of File ///////////////////////////////////////////////////////////////
// vim:ft=c:ts=4:sw=4
