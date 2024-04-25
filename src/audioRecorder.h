#ifndef AUDIORECORDER_H
#define AUDIORECORDER_H

#include <portaudio.h>

#include <QObject>
#include <q_io/audio_device.hpp>
#include <q_io/audio_stream.hpp>
#include <vector>

#include "WavFile.h"

// #define SAMPLE_RATE 44100
// #define NUM_CHANNELS (2)
#define FRAMES_PER_BUFFER 512
#define SAMPLE_SILENCE (0.0f)
#define RECORDED_SECONDS (10)

namespace q = cycfi::q;

typedef struct {
  int frameIndex; /* Index into sample array. */
  int maxFrameIndex;
  int16_t numChannels;
  SAMPLE* recordedSamples;
} paTestData;

class audioRecorder : public QObject {
  Q_OBJECT

  paTestData* m_data;
  std::vector<q::audio_device> m_audioDevices;
  int m_deviceIndex = -1;
  int16_t numChannels;
  int32_t sampleRate;
  WavFile wavFile;
  QVector<double>* graphData;

 public:
  // explicit audioRecorder(q::audio_device* device);
  explicit audioRecorder(int deviceIndex, QVector<double>* graphData);
  ~audioRecorder();
  int record();
  inline bool writeToFile(const std::string& fileName = "testRecord.wav") {
    return wavFile.writeWaveFile(fileName, sampleRate, numChannels, 1,
                                 RECORDED_SECONDS, m_data->recordedSamples);
  }

 private:
  static int recordCallback(const void* inputBuffer, void* outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags, void* userData);
  void terminate();
  void sendUpdateGraph() { emit updateGraph(); }
 signals:
  void updateGraph();
};

#endif  // AUDIORECORDER_H
