// █▀▄
#ifndef AUDIODEVICESELECTOR_H
#define AUDIODEVICESELECTOR_H

#include <portaudio.h>

#include <QWidget>
#include <q_io/audio_device.hpp>
#include <q_io/audio_stream.hpp>
#include <vector>

#include "audioRecorder.h"
#include "qcustomplot.h"

namespace q = cycfi::q;

namespace Ui {
class AudioDeviceSelector;
}

enum class ErrorCodes {
  UNINITIALIZES = -1,
  No_ERROR,
  NO_INPUT_CHANNELS,
  SERVICE_INIT_FAULT,
  AUDIO_STREAM_OPENNING_FAULT,
  AUDIO_STREAM_STARTING_FAULT,
  AUDIO_STREAM_STOPPING_FAULT,
  AUDIO_STREAM_CLOSING_FAULT,
  SERVICE_TERMINATE_FAULT
};

class AudioDeviceSelector : public QWidget {
  Q_OBJECT

  Ui::AudioDeviceSelector* ui;
  audioRecorder* m_recorder = nullptr;
  QCustomPlot* customPlot = nullptr;
  std::vector<q::audio_device> m_audioDevices;
  int m_deviceIndex = -1;
  ErrorCodes m_errorFlag = ErrorCodes::UNINITIALIZES;

 public:
  explicit AudioDeviceSelector(const std::vector<q::audio_device>& devices,
                               QWidget* parent = nullptr);
  ~AudioDeviceSelector();

 private:
  void drawLoop();

 signals:
  void waveFileDone(WavFile* waveFile);

 private slots:
  void listSelected();
  void drawGraph();
};

#endif  // AUDIODEVICESELECTOR_H
