PROG		= ENikiBENiki
SOURCES		:= UITest.cxx UI.cxx Controller.cxx main.cxx precompile.cxx

ifndef PTLIBDIR
PTLIBDIR=ptlib
endif

include $(PTLIBDIR)/make/ptlib.mak

docs: 
	doxygen docs.cfg

