STDCCFLAGS    += -Iexternal/SDL -Iexternal/SDL_ttf -Iexternal/physfs
ENDLDLIBS     += -Lexternal/SDL/build/.libs -Lexternal/SDL_ttf/.libs/ -Lexternal/physfs -lSDL -lSDL_ttf -lphysfs

PROG		= ENikiBENiki
SOURCES		:= UITest.cxx UIDefault.cxx UI.cxx Controller.cxx  Resources.cxx ResourceRO.cxx ResourceWO.cxx Resource.cxx main.cxx precompile.cxx

ifndef PTLIBDIR
PTLIBDIR=external/ptlib
endif

include $(PTLIBDIR)/make/ptlib.mak

docs: 
	doxygen docs.cfg

