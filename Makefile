#ipc : ipc.o proc_in.o
#	arm-none-gnueabi-gcc -static -o ipc.o proc_in.o
#*proc_in.o : proc_in.c
#	arm-none-gnueabi-gcc -static -c -o proc_in.o proc_in.c
#clean : 
#	rm *.o 

SOURCES=$(shell find . -type f -iname '*.c')
TARGET=HW1_20121625.out

all: $(TARGET)

$(TARGET): $(SOURCES)
	arm-none-linux-gnueabi-gcc -static --std=gnu99 $(SOURCES) -o $(TARGET)

clean: 
	rm -f $(TARGET)
