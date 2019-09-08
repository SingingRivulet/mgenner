CC = em++ -std=c++11

index.js:main.cpp view.o
	$(CC) main.cpp view.o editTable.o hbb.o midiMap.o -o index.js

view.o:view.h view.cpp editTable.o
	$(CC) view.cpp -c

editTable.o:editTable.h editTable.cpp midiMap.o
	$(CC) editTable.cpp -c

hbb.o:hbb.h hbb.cpp mempool.h
	$(CC) hbb.cpp -c

midiMap.o:midiMap.h midiMap.cpp hbb.o note.h
	$(CC) midiMap.cpp -c

clear:
	rm *.o