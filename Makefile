CC=g++
CPP=g++
LD=g++
CFLAGS = -DCLSERXXX_INC -g3 -std=c++11
LNKFLAGS = -L. -lclseremc -lm -lpthread -lstdc++

UTIL_OBJS=\
	cJSON.o \
    CNT_JSON.o \
    utils.o \
	BBP_utils.o \
	Euresys_Serial.o

all: \
	$(UTIL_OBJS) \
	readRegValues \
	writeRegValues \
	terminal_extended

#-------------------------------------------------

readRegValues: readRegValues.o $(UTIL_OBJS)
	$(LD) -o $@ $(UTIL_OBJS) $< $(LNKFLAGS)

writeRegValues: writeRegValues.o $(UTIL_OBJS)
	$(LD) -o $@ $(UTIL_OBJS) $< $(LNKFLAGS)

terminal_extended: terminal_extended.o $(UTIL_OBJS)
	$(LD) -o $@ $(UTIL_OBJS) $< $(LNKFLAGS)

clean:
	rm -f *.bak *.o $(TARGET) *~
	rm -f readRegValues
	rm -f writeRegValues
	rm -f terminal_extended

.cpp.o:
	$(CPP) $(CFLAGS) $(INCFLAGS) -c $?

.c.o:
	$(CC) $(CFLAGS) $(INCFLAGS) -c $?

