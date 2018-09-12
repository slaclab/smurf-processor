CC=g++


all :smurftestserver smurfpipetest  
 

smurftestserver: smurftestserver.o
	$(CC) -o smurftestserver smurftestserver.o

smurfpipetest: smurfpipetest.o ../../common/smurf2mce.h
	$(CC) -o smurfpipetest smurfpipetest.o

smurftestserver.o: smurftestserver.cpp ../../common/smurf2mce.h 
	$(CC) -c smurftestserver.cpp

smurfpipetest.o: smurfpipetest.cpp ../../common/smurf2mce.h 
	$(CC) -c smurfpipetest.cpp
