#include <cstdint>
#include <fstream>
#include <iostream>

struct VAW_HEADER {
	// Master RIFF chunk
	std::uint8_t ChunkID[4] = {'R', 'I', 'F', 'F'};
	std::uint32_t ChunkSize; // Overall file size minus 8 bytes
	std::uint8_t Format[4] = {'W', 'A', 'V', 'E'};
	// Chunk describing the data format
	std::uint8_t Subchunk1ID[4] = {'f', 'm', 't', ' '};
	std::uint32_t Subchunk1Size = 16; // Chunk size minus 8 bytes, which is 16 bytes here
	std::uint16_t AudioFormat = 1; // Audio format 1=PCM,6=mulaw,7=alaw,     257=IBM
								   // Mu-Law, 258=IBM A-Law, 259=ADPCM
	std::uint16_t NumChannels = 1; // Number of channels 1=Mono 2=Sterio
	std::uint32_t SampleRate = 44100; // Sampling Frequency in Hz
	std::uint32_t ByteRate = 44100 * 2; // Number of bytes to read per second (Frequency * BytePerBloc).
	std::uint16_t BlockAlign = 2; // 2=16-bit mono, 4=16-bit stereo
								  // Number of bytes per block (NbrChannels * BitsPerSample / 8).
	std::uint16_t BitsPerSample = 16; // Number of bits per sample
	// Chunk containing the sampled data
	std::uint8_t DataBlocID[4] = {'d', 'a', 't', 'a'};
	std::uint32_t DataSize; // Sampling Frequency in Hz
};

int main() {
	VAW_HEADER vaw_header;
	const std::uint32_t sample_rate = vaw_header.SampleRate;

	const unsigned short frequency_hz = 50;
	const float amplitude = 1000; // from 0 to 1
	const float duration_seconds = 5.0;
	const float time_increment = 1/static_cast<float>(sample_rate);

	std::uint32_t sample_chunk_size_bytes = sample_rate * duration_seconds;
	std::uint32_t file_size_bytes = sizeof(VAW_HEADER) + sample_chunk_size_bytes;
	vaw_header.DataSize = sample_chunk_size_bytes;
	vaw_header.ChunkSize = file_size_bytes + sizeof(VAW_HEADER) - 8;

	std::ofstream fs("test.wav", std::ios::out | std::ios::binary);
	fs.write(reinterpret_cast<const char*>(&vaw_header), sizeof vaw_header);

	for (float cur_time = 0; cur_time < duration_seconds; cur_time += time_increment) {
		std::uint16_t sample = static_cast<std::uint16_t>(
				amplitude * sin(2 * M_PI * frequency_hz * cur_time));
		fs.write(reinterpret_cast<const char*>(&sample), sizeof(std::uint16_t));
	}

	fs.close();

	return 0;
}
