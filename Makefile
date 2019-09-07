CC = em++ -std=c++11
editTable.o:editTable.h editTable.cpp midiMap.o
	$(CC) editTable.cpp -c

hbb.o:hbb.h hbb.cpp mempool.h
	$(CC) hbb.cpp -c

midiMap.o:midiMap.h midiMap.cpp hbb.o note.h
	$(CC) midiMap.cpp -c
