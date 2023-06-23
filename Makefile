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
	setCLTapGeometry \
	terminal_extended

terminal_extended.o: terminal_extended.cpp
	$(CPP) $(CFLAGS) $(INCFLAGS) -c $?

setCLTapGeometry.o: setCLTapGeometry.cpp
	$(CPP) $(CFLAGS) $(INCFLAGS) -c $?

terminal_extended: terminal_extended.o $(UTIL_OBJS)
	$(LD) -o $@ $(UTIL_OBJS) $< $(LNKFLAGS)

setCLTapGeometry: setCLTapGeometry.o $(UTIL_OBJS)
	$(LD) -o $@ $(UTIL_OBJS) $< $(LNKFLAGS)

clean:
	rm -f *.bak *.o $(TARGET) *~

.cpp.o:
	$(CPP) $(CFLAGS) $(INCFLAGS) -c $?

.c.o:
	$(CC) $(CFLAGS) $(INCFLAGS) -c $?

