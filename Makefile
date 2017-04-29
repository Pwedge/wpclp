CC=gcc
CFLAGS=-c -Wall -I.
LDFLAGS=
SOURCES=wpc_lamp_patcher.c wpc_rom.c wpc_lampmatrix_patch.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=WPCGhost
LIBS=

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf ${OBJECTS} ${EXECUTABLE} ${EXECUTABLE}.exe
