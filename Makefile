.PHONY : all clean libcheck server client common

all : libcheck
	cd common && $(MAKE)
	cd server && $(MAKE)
	cd client && $(MAKE)

server : libcheck
	cd server && $(MAKE) clean
	cd server && $(MAKE)

client : libcheck
	cd client && $(MAKE) clean
	cd client && $(MAKE)

common : libcheck
	cd common && $(MAKE) clean
	cd common && $(MAKE)

libcheck :
	@$(LD) -luv `pkg-config --libs glib-2.0` -o temp.out && rm temp.out

clean :
	cd common && $(MAKE) clean
	cd server && $(MAKE) clean
	cd client && $(MAKE) clean