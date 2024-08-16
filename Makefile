CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS=
LDFLAGS=
LDLIBS=-lncurses -pthread

SRCS=main.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

all: $(OBJS)
	$(CXX) $(LDFLAGS) -o main $(OBJS) $(LDLIBS)

clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) all
