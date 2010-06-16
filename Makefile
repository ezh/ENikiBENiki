PROG		= ENikiBENiki
SOURCES		:= UITest.cxx UIDefault.cxx UI.cxx Controller.cxx main.cxx precompile.cxx

ifndef PTLIBDIR
PTLIBDIR=external/ptlib
endif

include $(PTLIBDIR)/make/ptlib.mak

docs: 
	doxygen docs.cfg

