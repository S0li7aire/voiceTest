#include "WavFile.h"

#include <QDebug>
#include <fstream>

static void initWaveHeader(waveHeader* header, int32_t sampleRate,
                           int16_t numChannels, int16_t audioFormat,
                           int32_t recordedSeconds) {
  header->audioFormat = audioFormat;
  header->numChannels = numChannels;
  header->sampleRate = sampleRate;
  header->bitsPerSample = sizeof(SAMPLE) * 8;
  header->byteRate =
      header->sampleRate * header->numChannels * (header->bitsPerSample / 8);
  header->blockAllign = header->numChannels * (header->bitsPerSample / 8);
  header->subchunk1Size =
      sizeof(header->audioFormat) + sizeof(header->numChannels) +
      sizeof(header->sampleRate) + sizeof(header->byteRate) +
      sizeof(header->blockAllign) + sizeof(header->bitsPerSample);
  int numSamples = recordedSeconds * header->sampleRate * header->numChannels;
  header->subchunk2Size =
      numSamples * header->numChannels * (header->bitsPerSample / 8);
  header->chunksize = sizeof(header->format) +
                      (sizeof(header->subchunk1Id) +
                       sizeof(header->subchunk1Size) + header->subchunk1Size) +
                      (sizeof(header->subchunk2Id) +
                       sizeof(header->subchunk2Size) + header->subchunk2Size);
}

WavFile::WavFile() { wave_data = nullptr; }

WavFile::WavFile(int32_t sampleRate, int16_t numChannels, int16_t audioFormat,
                 int32_t recordedSeconds) {
  initWaveHeader(&waveFileHeader, sampleRate, numChannels, audioFormat,
                 recordedSeconds);
  empty = true;
}

WavFile::~WavFile() { delete[] wave_data; }

bool WavFile::read(const std::string&& fileName) {
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

bool WavFile::write(const std::string&& fileName) {
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

  // delete[] wave_data;
  // wave_data = nullptr;
  return true;
}

bool WavFile::writeWaveFile(const std::string& fileName, int32_t sampleRate,
                            int16_t numChannels, int16_t audioFormat,
                            int32_t recordedSeconds, SAMPLE* data) {
  if (data == nullptr) {
    qDebug() << "Error data is NULL";
    return false;
  }
  waveHeader header;
  initWaveHeader(&header, sampleRate, numChannels, audioFormat,
                 recordedSeconds);

  std::ofstream writeWave(fileName, std::ios::ate | std::ios::binary);
  if (!writeWave) {
    qDebug() << "Error creating file: " + fileName;
    return false;
  }

  writeWave.write(reinterpret_cast<char*>(&header), sizeof(waveHeader));
  writeWave.write(reinterpret_cast<char*>(data), header.subchunk2Size);
  writeWave.close();
  if (!writeWave.good()) {
    qDebug() << "Error writing file: " + fileName;
    return false;
  }

  delete[] data;
  data = nullptr;
  return true;
}
