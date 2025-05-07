#include <cstdint>
#include <iostream>
#include <fstream>

struct WavHeader {
	// Master RIFF chunk
	uint8_t ChunkID[4] = {'R', 'I', 'F', 'F'};
	uint32_t ChunkSizeBytes; // Overall file size minus 8 bytes
	uint8_t Format[4] = {'W', 'A', 'V', 'E'};
	// Chunk describing the data format
	uint8_t Subchunk1ID[4] = {'f', 'm', 't', ' '};
	uint32_t Subchunk1Size = 16; // Chunk size minus 8 bytes, which is 16 bytes here
	uint16_t AudioFormat = 1; // Audio format 1=PCM,6=mulaw,7=alaw,     257=IBM
								   // Mu-Law, 258=IBM A-Law, 259=ADPCM
	uint16_t NumChannels = 1; // Number of channels 1=Mono 2=Sterio
	uint32_t SampleRateHZ = 44100; // Sampling Frequency in Hz
	uint32_t BytesPerSecond = 44100 * 2; // Number of bytes to read per second (Frequency * BytePerBloc).
	uint16_t BlockAlign = 2; // 2=16-bit mono, 4=16-bit stereo
								  // Number of bytes per block (NbrChannels * BitsPerSample / 8).
	uint16_t BitsPerSample = 16; // Number of bits per sample
	// Chunk containing the sampled data
	uint8_t DataBlocID[4] = {'d', 'a', 't', 'a'};
	uint32_t DataSizeBytes;
};

const unsigned short frequency_hz = 440;
// const uint16_t amplitude = 65535; // max value
const uint16_t amplitude = 10000;
const float duration_seconds = 3.0;

int main() {
	std::ofstream ofs("test.wav", std::ios::out | std::ios::binary);

	WavHeader wav_header;
	wav_header.DataSizeBytes = wav_header.BytesPerSecond * duration_seconds;
	wav_header.ChunkSizeBytes = sizeof(WavHeader) + wav_header.DataSizeBytes - 8;

	ofs.write(reinterpret_cast<const char*>(&wav_header), sizeof(WavHeader));

	float cur_arg = 0.0;
	size_t bytes_written = 0;
	const float arg_increment = 2.0 * M_PI * frequency_hz / static_cast<float>(wav_header.SampleRateHZ);

	while (bytes_written < wav_header.DataSizeBytes) {
		uint16_t sample = static_cast<std::uint16_t>(amplitude * sin(cur_arg));
		ofs.write(reinterpret_cast<const char*>(&sample), 2);
		cur_arg += 2.0 * M_PI * frequency_hz / static_cast<float>(wav_header.SampleRateHZ);
		bytes_written += 2;
	}

	ofs.close();

	return 0;
}
