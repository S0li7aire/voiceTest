// █▀▄
#ifndef AUDIODEVICESELECTOR_H
#define AUDIODEVICESELECTOR_H

#include <portaudio.h>

#include <QDialog>
#include <QtConcurrent/QtConcurrent>
#include <q_io/audio_device.hpp>
#include <q_io/audio_stream.hpp>

#include "WavFile.h"
#include "qcustomplot.h"
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

namespace Ui {
class AudioDeviceSelector;
}

class AudioDeviceSelector : public QDialog {
  Q_OBJECT

  paTestData* m_data;
  std::vector<q::audio_device> m_audioDevices;
  int m_deviceIndex = -1;
  int16_t numChannels;
  int32_t sampleRate;
  WavFile wavFile;

 public:
  explicit AudioDeviceSelector(const std::vector<q::audio_device>& devices,
                               QWidget* parent = nullptr);
  ~AudioDeviceSelector();

 private:
  void initRecorder();
  inline void destroyRecorder() { delete m_data; }
  int record();
  inline bool writeToFile(const std::string& fileName = "testRecord.wav") {
    return wavFile.writeWaveFile(fileName, sampleRate, numChannels, 1,
                                 RECORDED_SECONDS, m_data->recordedSamples);
  }
  static int recordCallback(const void* inputBuffer, void* outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags, void* userData);
  void terminate();
  void drawLoop();
 signals:
  void setValue(int, int);

 private slots:
  void listSelected(const QModelIndex& index);
  int startRecording();
  void drawGraph();
  void play();

 private:
  Ui::AudioDeviceSelector* ui;
  QCustomPlot* customPlot;
  QVector<double> m_leftChannelData;
  bool m_stopFlag = false;
  int graphSize = 0;
};

#endif  // AUDIODEVICESELECTOR_H
