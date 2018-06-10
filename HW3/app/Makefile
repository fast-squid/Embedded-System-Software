SOURCES=$(shell find . -type f -iname '*.c')
TARGET=app.out

all: $(TARGET)

$(TARGET): $(SOURCES)
	arm-none-linux-gnueabi-gcc -static --std=gnu99 $(SOURCES) -o $(TARGET)

clean: 
	rm -f $(TARGET)
