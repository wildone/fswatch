INCLUDE=-framework CoreServices
OBJECTS+=fswatch.o

all: clean fswatch install

fswatch: $(OBJECTS)
	gcc $(INCLUDE) -o fswatch $(OBJECTS)

clean:
	rm -f *.o fswatch

install:
	cp fswatch /usr/local/bin/
