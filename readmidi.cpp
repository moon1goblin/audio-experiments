#include <bit>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <format>
#include <vector>

// https://ccrma.stanford.edu/~craig/14q/midifile/MidiFileFormat.html
// https://www.music.mcgill.ca/~ich/classes/mumt306/StandardMIDIfileformat.html
// https://faydoc.tripod.com/formats/mid.htm
// http://www.somascape.org/midi/tech/mfile.html

auto print_bin = [](auto fuck) {
	std::cout << std::format("{:08b}", fuck) << std::endl;
};

struct MidiHeader {
	char ChunkID[4]; // "MThd"
	// uint8_t HeaderLengthBytes[4]; // "MThd"
	uint32_t HeaderLengthBytes; // always 6 bytes long, the size of the next three fields
	uint16_t Format; // = 0;
	// 0 = single track file format
	// 1 = multiple track file format
	// 2 = multiple song file format
	uint16_t NumOfTracks; // number of tracks that follow
	int16_t Division; // unit of time for delta timing
	// 	bit 15	bits 14 thru 8	         bits 7 thru 0
	//  0	    ticks per quarter-note
	//  1	    negative SMPTE format	 ticks per frame
};

struct MidiTrackChunkHeader {
	uint8_t ChunkID[4]; // "MTrk"
	uint32_t LengthBytes; // the number of bytes in the track chunk following this number.
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

template <typename T>
uint32_t ReadBigEndian(std::ifstream& ifs, T& fuckshit) {
	typeof(fuckshit) buffer;
  	ifs.read(reinterpret_cast<char*>(&buffer), sizeof(buffer));
	// print_bin(buffer);
	fuckshit = std::byteswap(buffer);
	return sizeof(buffer);
}

template <typename T>
uint32_t ReadLittleEndian(std::ifstream& ifs, T& fuckshit) {
  	ifs.read(reinterpret_cast<char*>(&fuckshit), sizeof(fuckshit));
	return sizeof(fuckshit);
}

uint32_t ReadVariableLength(std::ifstream& ifs, uint32_t& fuckbruh) {
	// std::cout << "reading variable length" << std::endl;
	uint32_t value = 0;
	uint32_t bytes_read = 0;
	uint8_t buffer; 

	do {
		ifs.read(reinterpret_cast<char*>(&buffer), sizeof(uint8_t));
		++bytes_read;
		print_bin(buffer);
		value = (value << 7) + (buffer & ~(1 << 7));
		buffer = buffer & (1 << 7);
	} while (buffer & (1 << 7));

	fuckbruh = value;
	// std::cout << "variable length: " << fuckbruh << std::endl;
	// std::cout << "bytes_read at variable length: " << bytes_read << std::endl;
	return bytes_read;
}

int main() {
	std::ifstream ifs("../thelick.mid", std::ios::binary);

	MidiHeader midi_header;

	// couldnt figure out how to read char[], this works and im not fucking with endiannes any more
	ReadLittleEndian(ifs, midi_header.ChunkID);
	ReadBigEndian(ifs, midi_header.HeaderLengthBytes);
	// std::cout << "header length: " << midi_header.HeaderLengthBytes << std::endl;
	ReadBigEndian(ifs, midi_header.Format);
	// std::cout << "format: " << midi_header.Format << std::endl;
	ReadBigEndian(ifs, midi_header.NumOfTracks);
	// std::cout << "num of tracks: " << midi_header.NumOfTracks << std::endl;
	ReadBigEndian(ifs, midi_header.Division);

	// TODO: add multitracking
	for (size_t i = 0; i < midi_header.NumOfTracks; ++i) {
		MidiTrackChunkHeader track_header;
		ReadLittleEndian(ifs, track_header.ChunkID);
		ReadBigEndian(ifs, track_header.LengthBytes);
		// std::cout << "track length " << track_header.LengthBytes << std::endl;
		// ifs.ignore(track_header.LengthBytes);

		for (uint32_t bytes_read = 0; bytes_read < track_header.LengthBytes;) {
		// for (uint32_t bytes_read = 0; bytes_read < 0;) {
			uint32_t delta_time;
			std::cout << "delta time: ";
			bytes_read += ReadVariableLength(ifs, delta_time);
			uint8_t buffer;
			ifs.read(reinterpret_cast<char*>(&buffer), sizeof(buffer));
			++bytes_read;
			// std::cout << "buffer: ";
			// print_bin(buffer);

			// sysex event
			// F0 <length> <sysex_data>
			// F7 <length> <any_data>
			if (buffer == 0xF0 || buffer == 0xF7) {
				std::cout << "sysex event" << std::endl;
				// uint32_t sysex_event_length;
				// bytes_read += ReadVariableLength(ifs, sysex_event_length);
				// ifs.ignore(sysex_event_length);
				do {
					ifs.read(reinterpret_cast<char*>(&buffer), sizeof(buffer));
					++bytes_read;
					print_bin(buffer);
				} while (buffer != 0xF7);
			}
			// meta event
			// FF <type> <length> <data>
			else if (buffer == 0xFF) {
				std::cout << "meta event" << std::endl;

				ifs.read(reinterpret_cast<char*>(&buffer), sizeof(buffer));
				std::cout << "meta type: ";
				print_bin(buffer);
				++bytes_read;

				uint32_t meta_event_length;
				std::cout << "meta event length ";
				bytes_read += ReadVariableLength(ifs, meta_event_length);
				for (uint32_t i = 0; i < meta_event_length; ++i) {
					ifs.read(reinterpret_cast<char*>(&buffer), sizeof(buffer));
					print_bin(buffer);
					++bytes_read;
				}

				// ifs.ignore(meta_event_length);
			}
			// else midi event 
			else {
				std::cout << "midi event" << std::endl;
				static uint16_t last_midi_command;
				static uint16_t last_midi_track_number;
				MidiEvent blyat;

				if (buffer & (1 << 7)) {
					blyat.TrackNumber = buffer & (0b00001111);
					blyat.Command = (buffer & (0b01110000)) >> 4;
					ifs.read(reinterpret_cast<char*>(&buffer), sizeof(buffer));
					++bytes_read;
				}
				// im assuming running midi event, check what was the last event if this doesnt work
				else {
					blyat.Command = last_midi_command;
					blyat.TrackNumber = last_midi_track_number;
				}

				// assuming 2 bytes, some are 1 byte but i hope this works
				blyat.NotePitch = buffer;
				ifs.read(reinterpret_cast<char*>(&blyat.NoteVelocity), sizeof(blyat.NoteVelocity));
				++++bytes_read;
				
				if (blyat.Command == 0) {
					std::cout << "note off" << std::endl;
				}
				if (blyat.Command == 1) {
					std::cout << "note on" << std::endl;
				}
				// std::cout << "pitch: " << blyat.NotePitch << " velocity: " << blyat.NoteVelocity << std::endl;
				print_bin(blyat.NotePitch);
				print_bin(blyat.NoteVelocity);
			}
		}
	}
	return 0;
}
