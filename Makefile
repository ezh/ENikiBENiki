STDCCFLAGS    += -Iexternal/SDL -Iexternal/SDL_ttf -Iexternal/physfs
ENDLDLIBS     += -Lexternal/SDL/build/.libs -Lexternal/SDL_ttf/.libs/ -Lexternal/physfs -lSDL -lSDL_ttf -lphysfs

PROG		= ENikiBENiki
SOURCES		:= UITest.cpp UIDefault.cpp UI.cpp Controller.cpp  Resources.cpp ResourceRO.cpp ResourceWO.cpp Resource.cpp main.cpp precompile.cpp

ifndef PTLIBDIR
PTLIBDIR=external/ptlib
endif

include $(PTLIBDIR)/make/ptlib.mak

docs: 
	doxygen docs.cfg

