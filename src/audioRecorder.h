#ifndef AUDIORECORDER_H
#define AUDIORECORDER_H

#include <portaudio.h>

#include <QObject>
#include <q_io/audio_device.hpp>
#include <q_io/audio_stream.hpp>
#include <vector>

#include "WavFile.h"

#define FRAMES_PER_BUFFER 512
#define SAMPLE_SILENCE (0.0f)
#define RECORDED_SECONDS (10)

namespace q = cycfi::q;

class audioRecorder {
  q::audio_device* m_audioDevice = nullptr;
  WavFile* m_waveFile = nullptr;
  int32_t sampleRate;

 public:
  explicit audioRecorder(q::audio_device* device);
  ~audioRecorder();
  int record();
  QVector<double> getGraphData() {
    QVector<double> graphData;
    int dataSize = m_waveFile->getFrameIndex();
    if (dataSize < FRAMES_PER_BUFFER) {
      return graphData;
    }
    for (int i = 0; i < m_waveFile->getFrameIndex(); i++) {
      graphData.push_back(m_waveFile->getData()[i]);
    }
    return graphData;
  }
  inline bool writeToFile(const std::string&& fileName = "testRecord.wav") {
    return m_waveFile->write(std::move(fileName));
  }
  WavFile* moveWaveFile() {
    WavFile* waveFile = m_waveFile;
    m_waveFile = nullptr;
    return waveFile;
  }

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
