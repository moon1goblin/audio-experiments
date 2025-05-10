#include <bit>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <format>
#include <vector>

// https://ccrma.stanford.edu/~craig/14q/midifile/MidiFileFormat.html
// https://www.music.mcgill.ca/~ich/classes/mumt306/StandardMIDIfileformat.html
// https://faydoc.tripod.com/formats/mid.htm - the best one

struct MidiHeader {
	char ChunkID[4]; // "MThd"
	// uint8_t HeaderLengthBytes[4]; // "MThd"
	uint32_t HeaderLengthBytes; // always 6 bytes long, the size of the next three fields
	uint16_t Format; // = 0;
	// 0 = single track file format
	// 1 = multiple track file format
	// 2 = multiple song file format
	uint8_t NumOfTracks; // number of tracks that follow
	int8_t Division; // unit of time for delta timing
	// 	bit 15	bits 14 thru 8	         bits 7 thru 0
	//  0	    ticks per quarter-note
	//  1	    negative SMPTE format	 ticks per frame
};

struct MidiTrackChunkHeader {
	uint8_t ChunkID[4]; // "MTrk"
	uint32_t LenthBytes; // the number of bytes in the track chunk following this number.
};

struct MidiEvent {
	uint8_t Command = 0;
	uint8_t TrackNumber = 0;
	// uint8_t Data1;
	// uint8_t Data2;
	// only proccessing the note on/off events for now
	uint8_t NotePitch;
	uint8_t NoteVelocity;
	// 	1000nnnn 0kkkkkkk 0vvvvvvv Note Off event
	// 	1001nnnn 0kkkkkkk 0vvvvvvv Note On event
};

uint32_t ReadMidiEvent(std::ifstream& ifs, MidiEvent& blyat) {
	uint32_t bytes_read = 0;

	static uint16_t last_midi_command = blyat.Command;
	static uint16_t last_midi_track_number = blyat.TrackNumber;

	uint8_t buffer = 0;
	std::vector<uint8_t> vecbuf;

  	ifs.read(reinterpret_cast<char*>(&buffer), sizeof(buffer));
	++bytes_read;
	if (buffer & (1 << 7)) {
		blyat.TrackNumber = buffer & (0b00001111);
		blyat.TrackNumber = buffer & (0b01110000);
	}
	else {
		vecbuf.push_back(buffer);
		blyat.Command = last_midi_command;
		blyat.TrackNumber = last_midi_track_number;
	}

	uint16_t counter = 0;
	while (!(buffer & (1 << 7)) && counter < 2) {
		ifs.read(reinterpret_cast<char*>(&buffer), sizeof(buffer));
		vecbuf.push_back(buffer);
		++bytes_read;
		++counter;
	}
	
	if (blyat.Command == 0 || blyat.Command == 1) {
		std::cout << "note command, nice" << std::endl;
		blyat.NotePitch = vecbuf[0];
		blyat.NoteVelocity = vecbuf[1];
	}

	std::cout << "bytes_read at midi event: " << bytes_read << std::endl;
	return bytes_read;
}

struct MidiTrackEvent {
	uint32_t DeltaTime; // number of ticks after which the midi event is to be executed
   // If byte is greater or equal to 80h (128 decimal) 
   // 	then the next byte is also part of the VLV,
   // else byte is the last byte in a VLV.
	MidiEvent MidiEvent; // there are other event types but i really really hope this is enough
};

template <typename T>
void ReadBigEndian(std::ifstream& ifs, T& fuckshit) {
	typeof(fuckshit) buffer;
  	ifs.read(reinterpret_cast<char*>(&buffer), sizeof(buffer));
	fuckshit = std::byteswap(buffer);
}

template <typename T>
uint32_t ReadLittleEndian(std::ifstream& ifs, T& fuckshit) {
  	ifs.read(reinterpret_cast<char*>(&fuckshit), sizeof(fuckshit));
	return sizeof(fuckshit);
}

auto print_bin = [](auto fuck) {
	std::cout << std::format("{:08b}", fuck) << std::endl;
	// std::cout << fuck << std::endl;
};

uint32_t ReadVariableLength(std::ifstream& ifs, uint32_t& fuckbruh) {
	uint32_t value = 0;
	uint32_t bytes_read = 0;
	uint8_t buffer; 

	do {
		ifs.read(reinterpret_cast<char*>(&buffer), sizeof(uint8_t));
		++bytes_read;
		value = (value << 7) + (buffer & ~(1 << 7));
		buffer = buffer & (1 << 7);
	} while (buffer & (1 << 7));

	fuckbruh = value;
	std::cout << "variable length: " << fuckbruh << std::endl;
	std::cout << "bytes_read at variable length: " << bytes_read << std::endl;
	return bytes_read;
}

int main() {
	std::ifstream ifs("../test.mid", std::ios::binary);

	MidiHeader midi_header;

	// couldnt figure out how to read char[], this works and im not fucking with endiannes any more
	ReadLittleEndian(ifs, midi_header.ChunkID);
	ReadBigEndian(ifs, midi_header.HeaderLengthBytes);
	ReadBigEndian(ifs, midi_header.Format);
	ReadBigEndian(ifs, midi_header.NumOfTracks);
	ReadBigEndian(ifs, midi_header.Division);
	std::cout << "division: " << std::endl;
	print_bin(midi_header.Division);

	// TODO: add multitracking
	// for (size_t i = 0; i < midi_header.NumOfTracks; ++i) {
	// // blablabla
	// }

	MidiTrackChunkHeader track_header;
	ReadLittleEndian(ifs, track_header.ChunkID);
	ReadLittleEndian(ifs, track_header.LenthBytes);

	MidiTrackEvent midi_event;
	// for (uint32_t bytes_read = 0; bytes_read < track_header.LenthBytes;) {
	for (uint32_t bytes_read = 0; bytes_read < 100;) {
		bytes_read += ReadVariableLength(ifs, midi_event.DeltaTime);
		bytes_read += ReadMidiEvent(ifs, midi_event.MidiEvent);
	}

	return 0;
}
