#ifndef WAVFILE_H
#define WAVFILE_H

#include <stdint.h>

#include <string>

typedef int16_t SAMPLE;

#pragma pack(push, 4)
typedef struct waveHeader {
  int8_t chunkId[4] = {'R', 'I', 'F', 'F'};
  int32_t chunksize;
  int8_t format[4] = {'W', 'A', 'V', 'E'};
  int8_t subchunk1Id[4] = {'f', 'm', 't', ' '};
  int32_t subchunk1Size;
  int16_t audioFormat;
  int16_t numChannels;
  int32_t sampleRate;
  int32_t byteRate;
  int16_t blockAllign;
  int16_t bitsPerSample;
  int8_t subchunk2Id[4] = {'d', 'a', 't', 'a'};
  int32_t subchunk2Size;
} waveHeader;
#pragma pack(pop)

class WavFile {
  waveHeader waveFileHeader;
  bool empty = true;
  SAMPLE* wave_data;

 public:
  explicit WavFile(int32_t sampleRate, int16_t numChannels, int16_t audioFormat,
                   int32_t recordedSeconds, SAMPLE* data);
  WavFile();
  ~WavFile();
  inline bool isEmpty() { return this->empty; }
  bool readWaveFile(const std::string&& fileName);
  bool writeWaveFile(const std::string&& fileName = "record.wav");
  bool writeWaveFile(const std::string& fileName, int32_t sampleRate = 44100,
                     int16_t numChannels = 1, int16_t audioFormat = 1,
                     int32_t recordedSeconds = 0, SAMPLE* data = nullptr);

 private:
  void initHeader(int32_t sampleRate, int16_t numChannels, int16_t audioFormat,
                  int32_t recordedSeconds);
};

#endif  // WAVFILE_H
