CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -pedantic -O2
LDLIBS  = -lm
OBJS    = main.o artemis.o fisica.o validaciones.o reportes.o escenarios.o archivos.o
TARGET  = artemis

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

main.o: main.c artemis.h validaciones.h reportes.h escenarios.h archivos.h
artemis.o: artemis.c artemis.h fisica.h validaciones.h reportes.h
fisica.o: fisica.c artemis.h fisica.h
validaciones.o: validaciones.c validaciones.h
reportes.o: reportes.c artemis.h fisica.h reportes.h
escenarios.o: escenarios.c artemis.h fisica.h validaciones.h reportes.h escenarios.h
archivos.o: archivos.c artemis.h fisica.h reportes.h archivos.h

run: all
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all run clean
