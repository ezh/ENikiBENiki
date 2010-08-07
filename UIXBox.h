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

#ifndef _UIXBox_H_
#define _UIXBox_H_

#undef main

// special codes
// mouse axis X,Y,Z,...
#define MOUSE_N0 32000
#define MOUSE_N1 32001
#define MOUSE_N2 32002
#define MOUSE_N3 32003
#define MOUSE_N4 32004
#define MOUSE_N5 32005
#define MOUSE_N6 32006
#define MOUSE_N7 32007
#define MOUSE_N8 32008
#define MOUSE_N9 32009
// mouse buttons 1,2,3,...
#define MOUSE_B0 32010
#define MOUSE_B1 32011
#define MOUSE_B2 32012
#define MOUSE_B3 32013
#define MOUSE_B4 32014
#define MOUSE_B5 32015
#define MOUSE_B6 32016
#define MOUSE_B7 32017
#define MOUSE_B8 32018
#define MOUSE_B9 32019


class UIXBox : public UI {
    public:
        UIXBox(ControllerThread * _controller, Resources * _resources, PConfig * _config);
        virtual ~UIXBox();
        bool Initialize();
        void Main();
    private:
        void UpdateUIAndControls();
        void eventMouseUp();
        void eventMouseDown();
        void eventMouseMotion();
        void eventKeyDown();
        void eventKeyUp();
        void eventQuit();
        void apply_surface(int x,int y, SDL_Surface* source, SDL_Surface* destination, SDL_Rect* clip);
        void RegisterKey(const std::string& name, int code);
        void BindKeyToClass(int code, void* pUIXBoxBinding);
        // xbox controls (values)
        int controlX1;
        int controlY1;
        int controlX2;
        int controlY2;
        bool controlDU;
        bool controlDD;
        bool controlDL;
        bool controlDR;
        bool controlBack;
        bool controlGuide;
        bool controlStart;
        bool controlTL;
        bool controlTR;
        bool controlA;
        bool controlB;
        bool controlX;
        bool controlY;
        int controlLB;
        int controlRB;
        bool controlLT;
        bool controlRT;
        //The images
        SDL_Surface * screen;
        SDL_Surface * backgroundPassiveWaiting;
        SDL_Surface * backgroundPassiveReady;
        SDL_Surface * backgroundActiveDefault;
        //
        // global
        // -1 MOUSE_X
        // -2 MOUSE_Y
        // -3 MOUSE_N1
        // -4 MOUSE_N2
        // -5 MOUSE_N3
        // -6 MOUSE_N4
        // -7 MOUSE_N5
        // -8 MOUSE_0
        // -9 MOUSE_1
        // -10 MOUSE_2
        // -11 MOUSE_3
        // -12 MOUSE_4
        // -13 MOUSE_5
        // -14 MOUSE_6
        // -15 MOUSE_7
        // -16 MOUSE_8
        // -17 MOUSE_9
        std::map<std::string, int> keyNameToCode;
        std::map<int, std::string> keyCodeToName;
        void * codeKeyToClass[32767];
        //SDL_Surface * ledOn;
        //SDL_Surface * crosshairOn;
        //SDL_Surface * crosshairOff;
        //SDL_Surface * arrowTop;
        //SDL_Surface * arrowRight;
        //SDL_Surface * digitals[256];
        // arrows
        //int arrowOffsetX;
        //int arrowOffsetY;
        //The event structure that will be used
        SDL_Event event;
        bool quit; // make sure the program waits for a quit
        bool active; // passive//active mode
        /*
         * grab flag
         * 0 - none
         * 1 - main field
         * 2 - scrollX
         * 3 - scrollY
         */
        //int nMouseState;
        // boolen jump to center logic
        //bool jumpToCenter;
        // led
        //bool ledStatus;
        // control box
        //SDL_Rect boxMainField;
        //PString fontName;
        //TTF_Font * font;
        //SDL_Color textColor;
        PSyncPoint sync;
        PConfig *config;
};

#endif  // _UITest_H_

// End of File ///////////////////////////////////////////////////////////////
// vim:ft=c:ts=4:sw=4
