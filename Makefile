CC = em++ -std=c++11

index.js:main.cpp view.o MidiEventList.o MidiEvent.o MidiMessage.o Options.o MidiFile.o Binasc.o
	$(CC) main.cpp \
	view.o \
	editTable.o \
	hbb.o \
	midiMap.o \
	MidiEventList.o \
	MidiEvent.o \
	MidiMessage.o \
	Options.o \
	MidiFile.o \
	Binasc.o --bind -o index.js

view.o:view.h view.cpp editTable.o
	$(CC) view.cpp -c

editTable.o:editTable.h editTable.cpp midiMap.o
	$(CC) editTable.cpp -c

hbb.o:hbb.h hbb.cpp mempool.h
	$(CC) hbb.cpp -c

midiMap.o:midiMap.h midiMap.cpp hbb.o note.h
	$(CC) midiMap.cpp -c

MidiEventList.o:MidiEventList.cpp
	$(CC) MidiEventList.cpp -c

MidiEvent.o:MidiEvent.cpp
	$(CC) MidiEvent.cpp -c

MidiMessage.o:MidiMessage.cpp
	$(CC) MidiMessage.cpp -c

Options.o:Options.cpp
	$(CC) Options.cpp -c

MidiFile.o:MidiFile.cpp
	$(CC) MidiFile.cpp -c

Binasc.o:Binasc.cpp
	$(CC) Binasc.cpp -c

clear:
	rm *.o
