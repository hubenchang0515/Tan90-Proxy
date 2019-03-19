.PHONY : all clean libcheck

all : libcheck
	cd common && $(MAKE)
	cd server && $(MAKE)

libcheck :
	$(LD) -luv `pkg-config --libs glib-2.0`

clean :
	cd common && $(MAKE) clean
	cd server && $(MAKE) clean