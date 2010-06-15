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

#include <ptlib.h>

#include "SDL.h"

#include "main.h"
#include "version.h"
#include "Controller.h"

#define new PNEW


PCREATE_PROCESS(ENikiBeNikiProcess);

///////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

ENikiBeNikiProcess::ENikiBeNikiProcess()
  : PProcess("ENiki and BeNiki", "gamepad fun mod", MAJOR_VERSION, MINOR_VERSION, BUILD_TYPE, BUILD_NUMBER)
{
    //serial.SetReadTimeout(0); // timeout 0 ms
}

void ENikiBeNikiProcess::Main()
{
    PConfigArgs args(GetArguments());
    PStringStream progName;
    ControllerThread *controller;
    //The images
    SDL_Surface* hello = NULL;
    SDL_Surface* screen = NULL;
    //The event structure that will be used
    SDL_Event event;
    //Make sure the program waits for a quit
    bool quit = false;
    // x y
    signed short x = 0;
    signed short y = 0;
    
    args.Parse(
#if PTRACING
            "t-trace."              "-no-trace."
            "o-output:"             "-no-output."
#endif
#ifdef PMEMORY_CHECK
            "-setallocationbreakpoint:"
#endif
            "-baud:"
            "-databits:"
            "-parity:"
            "-stopbits:"
            "-flowcontrol:"
            "-serialport:"
            "v-version."
            "h-help."
            , PFalse);

#if PMEMORY_CHECK
    if (args.HasOption("setallocationbreakpoint"))
        PMemoryHeap::SetAllocationBreakpoint(args.GetOptionString("setallocationbreakpoint").AsInteger());
#endif
    
    progName << "Product Name: " << GetName() << endl
        << "Manufacturer: " << GetManufacturer() << endl
        << "Version     : " << GetVersion(PTrue) << endl
        << "System      : " << GetOSName() << '-'
        << GetOSHardware() << ' '
        << GetOSVersion();
    cout << progName << endl;
    
    if (args.HasOption('v'))
        return;

#if PTRACING
    PTrace::Initialise(args.GetOptionCount('t'),
            args.HasOption('o') ? (const char *)args.GetOptionString('o') : NULL,
            PTrace::Blocks | PTrace::Timestamp | PTrace::Thread | PTrace::FileAndLine);
#endif
    
    if (args.HasOption('h')) {
        cout << endl
#if PTRACING
            <<  "-t   --trace                     Debugging. Using more times for more detail" << endl
            <<  "-o   --output                    name of trace output file. If not specified, goes to stdout" << endl
#endif
#ifdef PMEMORY_CHECK
            <<  "     --setallocationbreakpoint   stop program on allocation of memory block number" << endl
#endif
            <<  "     --baud                      Set the data rate for serial comms" << endl
            <<  "     --databits                  Set the number of data bits (5, 6, 7, 8)" << endl
            <<  "     --parity                    Set parity, even, odd or none " << endl
            <<  "     --stopbits                  Set the number of stop bits (0, 1, 2) " << endl
            <<  "     --flowcontrol               Specifiy flow control, (none rtscts, xonxoff)" << endl
            <<  "     --serialport                Which serial port to use (COM1, ttyUSB2, ttya) ..." << endl
            <<  "-v   --version                   Print version information and exit" << endl
            <<  "-h   --help                      Write this help out.                   " << endl
            << endl;
        return;
    };
/*
             "1-test1."       "-no-test1."
             "2-test2."       "-no-test2."
             "3-test3."       "-no-test3.");

*/
    // serial communication
    if (!InitialiseSerial(args)) {
        cout << "failed to initialise the program" << endl;
        PThread::Sleep(100);
        return;
    };
    cout << "timer resolution reported as " << PTimer::Resolution() << "ms" << endl;
    controller = new ControllerThread(&serial);
    //Start SDL
    SDL_Init(SDL_INIT_EVERYTHING);
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
    //Free the loaded image
    SDL_FreeSurface(hello);
    //Quit SDL
    SDL_Quit(); 
    // Clean up
    controller->Stop();
    controller->WaitForTermination();
    cout << "Thread 1 terminated" << endl;
    //UserInterfaceThread *ui = new UserInterfaceThread(*this);
    //SerialInterfaceThread *si = new SerialInterfaceThread(*this);
    //ui->WaitForTermination();
    serial.Close();
    delete controller;
}

PBoolean ENikiBeNikiProcess::InitialiseSerial(PConfigArgs & args)
{
    PString flow;
    PINDEX dataBits;
    PINDEX stopBits;
    PString flowControlString;
    PINDEX baud;
    PString parity;
    PString portName;
    PSerialChannel::Parity pValue = PSerialChannel::DefaultParity;
    PSerialChannel::FlowControl flowControl = PSerialChannel::DefaultFlowControl;

    if (!args.HasOption("baud")) {
        baud = 115200;
        cout << "baud not specifed.          Using " << baud << endl;
    } else {
        baud = args.GetOptionString("baud").AsInteger();
        cout << "baud specified.             Using " << baud << endl;
    };
    if (!args.HasOption("databits")) {
        dataBits = 8;
        cout << "databits not specified.     Using " << dataBits << endl;
    } else {
        dataBits = args.GetOptionString("databits").AsInteger();
        cout << "databits specified.         Using " << dataBits << endl;
    };
    if (!args.HasOption("parity")) {
        parity = "none";
        cout << "parity not specified.       Using \"" << parity << "\"" << endl;
    } else {
        parity = args.GetOptionString("parity");
        cout << "parity specified            Using \"" << parity << "\"" << endl;
    };
    if (!args.HasOption("stopbits")) {
        stopBits = 1;
        cout << "stopbits not specified.     Using " << stopBits << endl;
    } else {
        stopBits = args.GetOptionString("stopbits").AsInteger();
        cout << "stopbits specified.         Using " << stopBits << endl;
    };
    if (!args.HasOption("flowcontrol")) {
        flow = "none";
        cout << "flow control not specified. Flow control set to " <<flow << endl;
    } else {
        flow = args.GetOptionString("flowcontrol");
        cout << "flow control is specified.  Flow control is set to " << flow << endl;
    };
    if ((flow *= "xonxoff") || (flow *= "rtscts") || (flow *= "none"))
        flowControlString = flow;
    else {
        cout << "valid args to flowcontrol are \"XonXoff\" or \"RtsCts\" or \"none\"" << endl;
        return PFalse;
    };
    if (!args.HasOption("serialport")) {
        PStringStream allNames;
        PStringList names = serial.GetPortNames();
        for(PINDEX i = 0; i < names.GetSize(); i++)
            allNames << names[i] << " ";
        cout << "example of available serial ports are " << allNames << endl;
        portName = names[0];
        cout << "serial port not specified. Serial port set to " << portName << endl;
    } else {
        portName = args.GetOptionString("serialport");
        cout << "serial port is specified. Serial port is set to " << portName << endl;
    };
    if (parity *= "none")
      pValue = PSerialChannel::NoParity;
    if (parity *= "even")
      pValue = PSerialChannel::EvenParity;
    if (parity *= "odd")
      pValue = PSerialChannel::OddParity;
    if (pValue == PSerialChannel::DefaultParity) {
      cout << "parity value of " << parity << " could not be interpreted" << endl;
      return PFalse;
    };
    if (flowControlString *= "xonxoff"){
      flowControl = PSerialChannel::XonXoff;
      PTRACE(3, "using xonxoff flow control");
    };
    if (flowControlString *= "rtscts") {
      flowControl = PSerialChannel::RtsCts;
      PTRACE(3, "using rts cts flow conrol ");
    };
    if (flowControlString *= "none") {
      flowControl = PSerialChannel::NoFlowControl;
      PTRACE(3, "not using any flow control of any sort");
    };
    if (!serial.Open(portName, baud, dataBits, pValue, stopBits, flowControl, flowControl)) {
      cout << "failed to open serial port " << endl;
      cout << "error code is \"" << serial.GetErrorText() << "\"" << endl;
      cout << "failed in attempt to open port /dev/" << portName << endl;
      return PFalse;
    };
 
    return PTrue;
}

// End of File ///////////////////////////////////////////////////////////////
// vim:ft=c:ts=4:sw=4
