CC = gcc
CFLAGS = -std=c99 -W -Wall -Wextra -O3 `pkg-config --cflags glib-2.0`
LDFLAGS = -ltan90common -luv `pkg-config --libs glib-2.0` -L../common
TARGET = ../tan90-client
TARGET_O = main.o proxyserver.o trueserver.o

.PHONY : all clean

all : $(TARGET)

$(TARGET) : $(TARGET_O)
	gcc -o $(TARGET) $(TARGET_O) $(LDFLAGS)

main.o : main.c proxyserver.h trueserver.h
proxyserver.o : proxyserver.c proxyserver.h trueserver.h
trueserver.o : trueserver.c trueserver.h proxyserver.h

clean : 
	rm -f $(TARGET_O)