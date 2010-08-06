STDCCFLAGS    += -Iexternal/lua/etc -Iexternal/lua/src -Iexternal/SDL -Iexternal/SDL_ttf -Iexternal/physfs
ENDLDLIBS     += -Lexternal/SLB/lib -Lexternal/SDL/build/.libs -Lexternal/SDL_ttf/.libs/ -Lexternal/physfs -lSDL -lSDL_ttf -lphysfs -llua

PROG		= ENikiBENiki
SOURCES		:= UIXBoxBinding.cpp UIXBox.cpp UIConsole.cpp UITest.cpp UIDefault.cpp UI.cpp Controller.cpp  Resources.cpp ResourceRO.cpp ResourceWO.cpp Resource.cpp FakeSerial.cpp main.cpp precompile.cpp

ifndef PTLIBDIR
PTLIBDIR=external/ptlib
endif

include $(PTLIBDIR)/make/ptlib.mak

docs: 
	doxygen docs.cfg

