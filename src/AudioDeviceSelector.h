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

class AudioDeviceSelector : public QWidget {
  Q_OBJECT

  Ui::AudioDeviceSelector* ui;
  audioRecorder* m_recorder = nullptr;
  QCustomPlot* customPlot = nullptr;
  std::vector<q::audio_device> m_audioDevices;
  int m_deviceIndex = -1;
  bool m_stopFlag = false;

 public:
  explicit AudioDeviceSelector(const std::vector<q::audio_device>& devices,
                               QWidget* parent = nullptr);
  ~AudioDeviceSelector();

 private:
  void drawLoop();

 signals:
  void waveFileDone(WavFile* waveFile);

 private slots:
  void listSelected(int index);
  void drawGraph();
};

#endif  // AUDIODEVICESELECTOR_H
