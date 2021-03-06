#
# Makefile for bogosps program
#


# use these flags if using gcc
CC	= gcc
CFLAGS	= -Wall -O3 -g -fomit-frame-pointer -static
LDFLAGS = -lm


bogosps.nat bogosps.arm: bogosps.c Makefile
	$(CC) $(DEFS) $(CFLAGS) -o bogosps.nat -Wa,-adhlns=bogosps.nat.lst bogosps.c $(LDFLAGS)
	objdump -d --source bogosps.nat > bogosps.nat.asm
	arm-linux-gnueabi-gcc $(DEFS) $(CFLAGS) -o bogosps.arm  -Wa,-adhlns=bogosps.arm.lst bogosps.c $(LDFLAGS)
	arm-linux-gnueabi-objdump -d --source bogosps.arm > bogosps.arm.asm

clean:
	$(RM) bogosps.nat bogosps.arm

install: bogosps bogosps.1
	cp bogosps /usr/local/bin/bogosps
	cp bogosps.1 /usr/local/man/man1/bogosps.1

dist:
	cd .. ; \
	/bin/bash -c "tar -czvf bogosps-1.0.tar.gz \
	bogosps-1.0/{bogosps.c,bogosps.1,README,INSTALLING,Makefile,.gitignore}"
