include ../../Makefile.pub
  
#-I../logger    -I./net/reactor
DEFS =
CFLAGS :=   -Wall   $(DEFS) 
CFLAGS +=  -I./    -I../
CXXFLAGS:=$(CFLAGS)
CCFLAGS:=$(CFLAGS)
LIBPATH := 
LIBS :=   ../connet.a\
			-Wl,-rpath=.
#注意lib的顺序，最底层的放在最后面，以便于被前面的库引用


SOURCES = $(wildcard *.c)
OBJS=${SOURCES:%.c=obj/%.o}

.PHONY :prepare all clean

all: prepare connet_demo_bin

prepare:
	@if ! [ -d obj ]; then mkdir obj; fi;
	
obj/%.o : %.c
	$(COMPILE_CC) 

connet_demo_bin:$(OBJS)
	$(LINK_CC)    

clean:
	rm -rf obj  *~ obj/db.o connet_demo_bin

