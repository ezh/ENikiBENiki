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

#include <ptlib.h>
#include <map>
#include <string>


#include "main.h"
#include "version.h"
#include "FakeSerial.h"
#include "Controller.h"
#include "Resources.h"

#include "UI.h"
#include "UIXBox.h"
#include "UIConsole.h"
#include "UITest.h"
#include "UIDefault.h"

#define new PNEW


PCREATE_PROCESS(ENikiBeNikiProcess);

///////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Value-Defintions of the different String values
enum UIStringValue { uiDefault, uiTest, uiConsole, uiXBox };
// Map to associate the ui strings with the enum values
static std::map<std::string, UIStringValue> mapUIStringValues;

ENikiBeNikiProcess::ENikiBeNikiProcess()
  : PProcess("ENiki and BeNiki", "gamepad fun mod", MAJOR_VERSION, MINOR_VERSION, BUILD_TYPE, BUILD_NUMBER)
{
    mapUIStringValues["default"] = uiDefault;
    mapUIStringValues["test"] = uiTest;
    mapUIStringValues["console"] = uiConsole;
    mapUIStringValues["xbox"] = uiXBox;
}

void ENikiBeNikiProcess::Main()
{
    ControllerThread *controller;
    UI *ui;
    Resources *resources;
    PConfigArgs args(GetArguments());
    PStringStream progName;
    PString resourceExt("res");
    PString appName(GetName());
    PString appExec(GetFile());
    PDirectory homeDir = PXGetHomeDir();
    PFilePath configPath(homeDir + "." + appName + homeDir[homeDir.GetLength()-1] + "profile.ini"); // $HOME/.NNN/profile.ini
    PConfig config(configPath, "Options");

    args.Parse(
#if PTRACING
            "t-trace."              "-no-trace."
            "o-output:"             "-no-output."
#endif
#ifdef PMEMORY_CHECK
            "-setallocationbreakpoint:"
#endif
            "u-ui:"
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
            <<  "-u   --ui                        UI type" << endl
            <<  "     --baud                      Set the data rate for serial comms" << endl
            <<  "     --databits                  Set the number of data bits (5, 6, 7, 8)" << endl
            <<  "     --parity                    Set parity, even, odd or none " << endl
            <<  "     --stopbits                  Set the number of stop bits (0, 1, 2) " << endl
            <<  "     --flowcontrol               Specifiy flow control, (none rtscts, xonxoff)" << endl
            <<  "     --serialport                Which serial port to use (fake, COM1, ttyUSB2, ttya) ..." << endl
            <<  "-v   --version                   Print version information and exit" << endl
            <<  "-h   --help                      Write this help out.                   " << endl
            << endl;
        return;
    };

    PTRACE(1, "Load ENikiBENiki settings from " << configPath);
    // serial communication
    if (!InitializeSerial(args)) {
        cout << "failed to initialize the program" << endl;
        PThread::Sleep(100);
        return;
    };
    cout << "timer resolution reported as " << PTimer::Resolution() << "ms" << endl;
    controller = new ControllerThread(pserial, &config);
    resources = new Resources(resourceExt);
    if (resources->Open(appExec, appName)) {
        switch(mapUIStringValues[(const char *)args.GetOptionString('u')]) {
            case uiXBox:
                ui = new UIXBox(controller, resources, &config);
                break;
            case uiConsole:
                ui = new UIConsole(controller, resources, &config);
                break;
            case uiTest:
                ui = new UITest(controller, resources, &config);
                break;
            default:
                ui = new UIDefault(controller, resources, &config);
                break;
        };
        if (ui->Initialize()) {
            ui->Main();
        };
        // Clean up
        delete ui;
        resources->Close();
    };
    delete resources;
    controller->Stop();
    controller->WaitForTermination();
    delete controller;
    pserial->Close();
    cout << "main thread terminated successful" << endl;
}

PBoolean ENikiBeNikiProcess::InitializeSerial(PConfigArgs & args)
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

    if (!args.HasOption("serialport")) {
        PStringStream allNames;
        pserial = new PSerialChannel();
        PStringList names = pserial->GetPortNames();
        for(PINDEX i = 0; i < names.GetSize(); i++)
            allNames << names[i] << " ";
        cout << "example of available serial ports are " << allNames << endl;
        portName = names[0];
        cout << "serial port not specified. Serial port set to " << portName << endl;
    } else {
        portName = args.GetOptionString("serialport");
        cout << "serial port is specified. Serial port is set to " << portName << endl;
        if (portName *= "fake"){
            pserial = new FakeSerial(); 
        } else {
            pserial = new PSerialChannel();
        };
    };
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
    if (!pserial->Open(portName, baud, dataBits, pValue, stopBits, flowControl, flowControl)) {
        cout << "failed to open serial port " << endl;
        cout << "error code is \"" << pserial->GetErrorText() << "\"" << endl;
        cout << "failed in attempt to open port /dev/" << portName << endl;
        return PFalse;
    };
    pserial->SetReadTimeout(10); // timeout 10 ms
    pserial->SetWriteTimeout(10); // timeout 10 ms

    return PTrue;
}

// End of File ///////////////////////////////////////////////////////////////
// vim:ft=c:ts=4:sw=4
