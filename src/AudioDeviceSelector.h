// █▀▄
#ifndef AUDIODEVICESELECTOR_H
#define AUDIODEVICESELECTOR_H

#include <portaudio.h>

#include <QDialog>
#include <QtConcurrent/QtConcurrent>
#include <q_io/audio_device.hpp>
#include <q_io/audio_stream.hpp>

#include "audioRecorder.h"
#include "qcustomplot.h"

namespace q = cycfi::q;

namespace Ui {
class AudioDeviceSelector;
}

class AudioDeviceSelector : public QDialog {
  Q_OBJECT

  Ui::AudioDeviceSelector* ui;
  audioRecorder* m_recorder = nullptr;
  std::vector<q::audio_device> m_audioDevices;
  int m_deviceIndex = -1;
  bool m_stopFlag = false;
  QCustomPlot* customPlot = nullptr;

 public:
  explicit AudioDeviceSelector(const std::vector<q::audio_device>& devices,
                               QWidget* parent = nullptr);
  ~AudioDeviceSelector();

 private:
  void drawLoop();

 signals:
  void waveFileDone(WavFile* waveFile);

 private slots:
  void listSelected(const QModelIndex& index);
  void drawGraph();
};

#endif  // AUDIODEVICESELECTOR_H