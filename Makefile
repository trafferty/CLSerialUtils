CC=g++
CPP=g++
LD=g++
CFLAGS = -DCLSERXXX_INC -g3 -std=c++11
LNKFLAGS = -L. -lclseremc -lm -lpthread -lstdc++

UTIL_OBJS=\
	BBP_utils.o \
	Euresys_Serial.o

all: \
	$(UTIL_OBJS) \
	setTapGeometry \
	terminal_extended

terminal_extended.o: terminal_extended.cpp
	$(CPP) $(CFLAGS) $(INCFLAGS) -c $?

setTapGeometry.o: setTapGeometry.cpp
	$(CPP) $(CFLAGS) $(INCFLAGS) -c $?

terminal_extended: terminal_extended.o $(UTIL_OBJS)
	$(LD) -o $@ $(UTIL_OBJS) $< $(LNKFLAGS)

setTapGeometry: setTapGeometry.o $(UTIL_OBJS)
	$(LD) -o $@ $(UTIL_OBJS) $< $(LNKFLAGS)

clean:
	rm -f *.bak *.o $(TARGET) *~

.cpp.o:
	$(CPP) $(CFLAGS) $(INCFLAGS) -c $?

.c.o:
	$(CC) $(CFLAGS) $(INCFLAGS) -c $?

