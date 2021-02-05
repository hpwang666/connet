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

##### End of variables to change
SOURCES =  $(wildcard *.c) 
OBJFILES = $(SOURCES:%.c=obj/test_%.o)

.PHONY :clean  prepare all
TARGET = $(SOURCES:%.c=%_demo_bin)
all : prepare $(TARGET) 



prepare:
	@if ! [ -d obj ]; then mkdir obj; fi;
	
obj/test_%.o:%.c
	$(COMPILE_CC) 

%_demo_bin:obj/test_%.o
	$(LINK_CC)    

clean:
	rm -rf obj  *~ obj/db.o $(TARGET)

