TARGET = mm
OUTPUT = main.o
DEBUG = -g
MAKEFILE = makefile

$(TARGET): $(OUTPUT) $(MAKEFILE)
	gcc -o $(TARGET) $(DEBUG) $(OUTPUT)

main.o: main.c mem.h $(MAKEFILE)
	gcc -c $(DEBUG) main.c

clear:
	rm -f $(TARGET) $(OUTPUT)

rebuild:
	make clear
	make

