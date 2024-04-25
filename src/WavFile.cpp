#include "WavFile.h"

#include <QDebug>
#include <fstream>

WavFile::WavFile() { wave_data = nullptr; }

WavFile::WavFile(int32_t sampleRate, int16_t numChannels, int16_t audioFormat,
                 int32_t recordedSeconds, SAMPLE* data) {
  if (data == nullptr) {
    wave_data = nullptr;
  } else {
    initHeader(sampleRate, numChannels, audioFormat, recordedSeconds);
    wave_data = data;
    empty = false;
  }
}

WavFile::~WavFile() { delete[] wave_data; }

void WavFile::initHeader(int32_t sampleRate, int16_t numChannels,
                         int16_t audioFormat, int32_t recordedSeconds) {
  waveFileHeader.audioFormat = audioFormat;
  waveFileHeader.numChannels = numChannels;
  waveFileHeader.sampleRate = sampleRate;
  waveFileHeader.bitsPerSample = sizeof(SAMPLE) * 8;
  waveFileHeader.byteRate = waveFileHeader.sampleRate *
                            waveFileHeader.numChannels *
                            (waveFileHeader.bitsPerSample / 8);
  waveFileHeader.blockAllign =
      waveFileHeader.numChannels * (waveFileHeader.bitsPerSample / 8);
  waveFileHeader.subchunk1Size =
      sizeof(waveFileHeader.audioFormat) + sizeof(waveFileHeader.numChannels) +
      sizeof(waveFileHeader.sampleRate) + sizeof(waveFileHeader.byteRate) +
      sizeof(waveFileHeader.blockAllign) + sizeof(waveFileHeader.bitsPerSample);
  int numSamples =
      recordedSeconds * waveFileHeader.sampleRate * waveFileHeader.numChannels;
  waveFileHeader.subchunk2Size = numSamples * waveFileHeader.numChannels *
                                 (waveFileHeader.bitsPerSample / 8);
  waveFileHeader.chunksize =
      sizeof(waveFileHeader.format) +
      (sizeof(waveFileHeader.subchunk1Id) +
       sizeof(waveFileHeader.subchunk1Size) + waveFileHeader.subchunk1Size) +
      (sizeof(waveFileHeader.subchunk2Id) +
       sizeof(waveFileHeader.subchunk2Size) + waveFileHeader.subchunk2Size);
}

bool WavFile::readWaveFile(const std::string&& fileName) {
  if (!this->isEmpty()) {
    return false;
  }

  std::ifstream readWave(fileName, std::ios::in | std::ios::binary);
  if (!readWave) {
    qDebug() << "Error opening file: " + fileName;
    return false;
  }

  readWave.read(reinterpret_cast<char*>(&waveFileHeader), sizeof(waveHeader));
  SAMPLE tmpData[waveFileHeader.subchunk2Size];
  readWave.read(reinterpret_cast<char*>(&tmpData),
                waveFileHeader.subchunk2Size);
  readWave.close();
  if (!readWave.good()) {
    qDebug() << "Error reading file: " + fileName;
    return false;
  }

  wave_data = new SAMPLE[waveFileHeader.subchunk2Size];
  for (int i = 0; i < waveFileHeader.subchunk2Size; ++i) {
    wave_data[i] = tmpData[i];
  }
  return true;
}

bool WavFile::writeWaveFile(const std::string&& fileName) {
  if (this->isEmpty()) {
    return false;
  }

  std::ofstream writeWave(fileName, std::ios::ate | std::ios::binary);
  if (!writeWave) {
    qDebug() << "Error creating file: " + fileName;
    return false;
  }

  writeWave.write(reinterpret_cast<char*>(&waveFileHeader), sizeof(waveHeader));
  writeWave.write(reinterpret_cast<char*>(wave_data),
                  waveFileHeader.subchunk2Size);
  writeWave.close();
  if (!writeWave.good()) {
    qDebug() << "Error writing file: " + fileName;
    return false;
  }

  delete[] wave_data;
  wave_data = nullptr;
  return true;
}

bool WavFile::writeWaveFile(const std::string& fileName, int32_t sampleRate,
                            int16_t numChannels, int16_t audioFormat,
                            int32_t recordedSeconds, SAMPLE* data) {
  if (data == nullptr) {
    qDebug() << "Error data is NULL";
    return false;
  }
  initHeader(sampleRate, numChannels, audioFormat, recordedSeconds);
  wave_data = data;
  this->empty = false;
  return writeWaveFile(std::move(fileName));
}
