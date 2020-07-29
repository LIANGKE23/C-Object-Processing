#
# File          : Makefile
# Description   : Build file for CMPSC473 project 1
#                 


# Environment Setup
#LIBDIRS=-L. -L/opt/local/lib
#INCLUDES=-I. -I/opt/local/include
CC=gcc 
CFLAGS=-c $(INCLUDES) -g -Wall 
LINK=gcc -g
LDFLAGS=$(LIBDIRS)
AR=ar rc
RANLIB=ranlib

# TASK 0
# Update with your student number
# BTW - No spaces after the number
SNUM=102

# Suffix rules
.c.o :
	${CC} ${CFLAGS} $< -o $@

#
# Setup builds

TARGETS=cmpsc473-p1
LIBS=

#
# Project Protections

p1 : $(TARGETS)

cmpsc473-p1 : cmpsc473-main.o cmpsc473-kvs.o cmpsc473-util.o
	$(LINK) $(LDFLAGS) cmpsc473-main.o cmpsc473-kvs.o cmpsc473-util.o $(LIBS) -o $@

clean:
	rm -f *.o *~ $(TARGETS)

BASENAME=p1-assign
tar: 
	tar cvfz $(BASENAME)-$(SNUM).tgz -C ..\
	    $(BASENAME)/Makefile \
            $(BASENAME)/cmpsc473-main.c \
	    $(BASENAME)/cmpsc473-kvs.c \
	    $(BASENAME)/cmpsc473-kvs.h \
	    $(BASENAME)/cmpsc473-util.c \
	    $(BASENAME)/cmpsc473-util.h \
	    $(BASENAME)/cmpsc473-format-$(SNUM).h 


 
