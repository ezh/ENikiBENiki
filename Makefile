PROG		= ENikiBENiki
SOURCES		:= Controller.cxx main.cxx precompile.cxx

ifndef PTLIBDIR
PTLIBDIR=ptlib
endif

include $(PTLIBDIR)/make/ptlib.mak

docs: 
	doxygen docs.cfg

