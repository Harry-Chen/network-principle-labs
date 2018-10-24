main : main.o  lookuproute.o checksum.o arpfind.o sendetherip.o recvroute.o
		cc -o main  main.o lookuproute.o checksum.o arpfind.o sendetherip.o recvroute.o -lpthread -g
main.o      : main.c      lookuproute.h checksum.h arpfind.h sendetherip.h recvroute.h
		cc -c main.c
lookuproute.o : lookuproute.c lookuproute.h
		cc -c lookuproute.c
checksum.o  : checksum.c  checksum.h
		cc -c checksum.c
arpfind.o   : arpfind.c   arpfind.h
		cc -c arpfind.c
sendetherip.o : sendetherip.c sendetherip.h
		cc -c sendetherip.c
recvroute.o : recvroute.c recvroute.h
		cc -c recvroute.c
clean :
		rm main  main.o lookuproute.o checksum.o arpfind.o sendetherip.o recvroute.o
SUBDIR = $(shell ls ./ -R | grep /)
SUBDIRS  = $(subst :,/,$(SUBDIR))
SOURCE = $(foreach dir, $(SUBDIRS),$(wildcard $(dir)*.o))
#clean:
#	    rm  -rf $(SOURCE)
