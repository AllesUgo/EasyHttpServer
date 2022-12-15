CFF= -c -ldl   -O3 -std=c++11
webserver: main.o handClient.o connectClient.o init.o cJSON.o str.o addone.o log.o ConnectionControl.o
	g++  main.o handClient.o ConnectionControl.o cJSON.o connectClient.o init.o str.o log.o addone.o -o webserver -ldl -lpthread -lm

main.o:main.cpp ThreadInfo.h cJSON.h addone.h log.h ConnectionControl.h
	g++ $(CFF)  main.cpp -o main.o
	

handClient.o:handClient.cpp ThreadInfo.h cJSON.h str.h define.h addone.h log.h ConnectionControl.h
	g++ $(CFF) handClient.cpp -o handClient.o

connectClient.o:connectClient.cpp ThreadInfo.h
	g++ $(CFF) connectClient.cpp -o connectClient.o

init.o:init.cpp cJSON.h ThreadInfo.h addone.h
	g++ $(CFF) init.cpp -o init.o

addone.o:addone.h define.h str.h addone.cpp
	g++ $(CFF) addone.cpp -o addone.o

str.o:str.h define.h str.cpp
	g++ $(CFF) str.cpp -o str.o

log.o:log.h log.cpp
	g++ $(CFF) log.cpp -o log.o
cJSON.o:cJSON.cpp cJSON.h
	g++ $(CFF) cJSON.cpp -o cJSON.o 
ConnectionControl.o:ConnectionControl.cpp ConnectionControl.h define.h ThreadInfo.h log.h
	g++ $(CFF) ConnectionControl.cpp -o ConnectionControl.o

install:webserver
	mv webserver ../webserver/

clear:
	rm *.o
clean: 
	rm *.o webserver
