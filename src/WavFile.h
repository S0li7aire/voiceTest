#ifndef WAVFILE_H
#define WAVFILE_H

#include <stdint.h>

#include <string>

typedef int16_t SAMPLE;

#pragma pack(push, 4)
typedef struct waveHeader {
  const int8_t chunkId[4]{'R', 'I', 'F', 'F'};
  int32_t chunksize;
  const int8_t format[4]{'W', 'A', 'V', 'E'};
  const int8_t subchunk1Id[4]{'f', 'm', 't', ' '};
  int32_t subchunk1Size;
  int16_t audioFormat;
  int16_t numChannels;
  int32_t sampleRate;
  int32_t byteRate;
  int16_t blockAllign;
  int16_t bitsPerSample;
  const int8_t subchunk2Id[4]{'d', 'a', 't', 'a'};
  int32_t subchunk2Size;
} waveHeader;
#pragma pack(pop)

class WavFile {
  waveHeader waveFileHeader;
  bool empty = true;
  int32_t frameIndex = 0;
  int32_t maxFrameIndex = 0;
  SAMPLE* wave_data = nullptr;

 public:
  WavFile();
  explicit WavFile(int32_t sampleRate, int16_t numChannels, int16_t audioFormat,
                   int32_t recordedSeconds);
  ~WavFile();
  inline SAMPLE* getData() { return wave_data; }
  inline void setData() {
    if (wave_data == nullptr) {
      delete wave_data;
    }
    wave_data =
        new SAMPLE[maxFrameIndex * waveFileHeader.numChannels * sizeof(SAMPLE)]{
            0};
    empty = false;
  }
  inline int32_t getFrameIndex() { return frameIndex; }
  inline void setFrameIndex(int32_t value) { frameIndex = value; }
  inline int32_t getMaxFrameIndex() { return maxFrameIndex; }
  inline void setMaxFrameIndex(int32_t value) { maxFrameIndex = value; }
  inline int16_t getNumChannels() { return waveFileHeader.numChannels; }
  inline int32_t getSampleRate() { return waveFileHeader.sampleRate; }
  inline bool isEmpty() { return this->empty; }
  bool read(const std::string&& fileName);
  bool write(const std::string&& fileName = "record.wav");
  bool writeWaveFile(const std::string& fileName, int32_t sampleRate = 44100,
                     int16_t numChannels = 1, int16_t audioFormat = 1,
                     int32_t recordedSeconds = 0, SAMPLE* data = nullptr);
};

#endif  // WAVFILE_H
