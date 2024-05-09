#ifndef AUDIORECORDER_H
#define AUDIORECORDER_H

#include <portaudio.h>

#include <QVector>
#include <q_io/audio_device.hpp>
#include <q_io/audio_stream.hpp>

#include "WavFile.h"

#define FRAMES_PER_BUFFER 512
#define SAMPLE_SILENCE (0.0f)
#define RECORDED_SECONDS (2)

namespace q = cycfi::q;

class audioRecorder {
  q::audio_device* m_audioDevice = nullptr;
  WavFile* m_waveFile = nullptr;
  int32_t sampleRate = 0;

 public:
  explicit audioRecorder(q::audio_device* device);
  ~audioRecorder();
  int record();
  void reset(q::audio_device* device);
  QVector<double> getGraphData();
  inline bool writeToFile(const std::string&& fileName = "testRecord.wav") {
    return m_waveFile->write(std::move(fileName));
  }
  inline WavFile* moveWaveFile() {
    WavFile* waveFile = m_waveFile;
    m_waveFile = nullptr;
    return waveFile;
  }
  inline const SAMPLE* getAudioSamples() const { return m_waveFile->getData(); }
  inline int32_t getSampleRate() { return sampleRate; }
  inline int32_t getFrameIndex() { return m_waveFile->getFrameIndex(); }
  inline int32_t getMaxFrameIndex() { return m_waveFile->getMaxFrameIndex(); }
  inline int16_t getNumChannels() { return m_waveFile->getNumChannels(); }

 private:
  static int recordCallback(const void* inputBuffer, void* outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags, void* userData);
  void terminate();
 signals:
  void updateGraph();
};

#endif  // AUDIORECORDER_H
