CC = gcc
CFLAGS = -std=c99 -W -Wall -Wextra -O3 `pkg-config --cflags glib-2.0`
LDFLAGS = -ltan90common -luv `pkg-config --libs glib-2.0` -L../common
TARGET = ../tan90-server
TARGET_O = main.o proxyclient.o trueclient.o

.PHONY : all clean

all : $(TARGET)

$(TARGET) : $(TARGET_O)
	gcc -o $(TARGET) $(TARGET_O) $(LDFLAGS)

main.o : main.c proxyclient.h trueclient.h userdata.h
proxyclient.o : proxyclient.c proxyclient.h trueclient.h userdata.h
trueclient.o : trueclient.c trueclient.h proxyclient.h userdata.h

clean : 
	rm -f $(TARGET_O)