.PHONY : all debug clean libcheck server client common 

all : libcheck
	cd common && $(MAKE)
	cd server && $(MAKE)
	cd client && $(MAKE)

debug : libcheck
	cd common && $(MAKE) opt=debug
	cd server && $(MAKE) opt=debug
	cd client && $(MAKE) opt=debug

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