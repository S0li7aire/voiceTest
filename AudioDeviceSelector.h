// █▀▄
#ifndef AUDIODEVICESELECTOR_H
#define AUDIODEVICESELECTOR_H

#include <portaudio.h>

#include <QDialog>
#include <fstream>
#include <q_io/audio_device.hpp>
#include <q_io/audio_stream.hpp>

#include "qcustomplot.h"

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 512
typedef float SAMPLE;
#define NUM_CHANNELS (2)
#define SAMPLE_SILENCE (0.0f)

typedef struct {
  int frameIndex; /* Index into sample array. */
  int maxFrameIndex;
  SAMPLE* recordedSamples;
} paTestData;

typedef struct wavFile {
    const char chunkId[4] = {'R', 'I', 'F', 'F'};
    long chunkSize = 0;                             // == 4 + (8 + SubChunk1Size) + (8 + SubChunk2Size)
    const char format[4] = {'W', 'A', 'V', 'E'};
    const char subchunk1Id[4] = {'f', 'm', 't', ' '};
    long subchunk1Size = 0;                         // size from this to blockAllign included
    short int audioFormat = 1;                      // 1 - no compression(so will be it)
    short int numChannels = 1;                      // 1 - Mono 2 - Stereo
    long sampleRate;                                // 8000, 44100, etc.
    long byteRate;                                  // == SampleRate * NumChannels * BitsPerSample/8
    short int blockAllign;                          // == NumChannels * BitsPerSample/8
    short int bitsPerSample;                        // 8bits = 8, 16bits = 16, etc.
    const char subchunk2Id[4] = {'d', 'a', 't', 'a'};
    long subchunk2Size;                             // == NumSamples * NumChannels * BitsPerSample/8
    SAMPLE* data;                                   // raw sound data ---- remove and use from another place!!!!!!
} wavFile;

namespace q = cycfi::q;

namespace Ui {
class AudioDeviceSelector;
}

class AudioDeviceSelector : public QDialog {
  Q_OBJECT

 public:
  explicit AudioDeviceSelector(QWidget* parent = nullptr);
  explicit AudioDeviceSelector(const std::vector<q::audio_device>& devices,
                               QWidget* parent = nullptr);
  ~AudioDeviceSelector();

 signals:
  void setValue(int, int);

 private slots:
  void listSelected(const QModelIndex& index);
  int portaudiotest();
  static int recordCallback(const void* inputBuffer, void* outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags, void* userData);
  void drawGraph();
  void terminate();
  void play();
  void writeToWav();

 private:
  Ui::AudioDeviceSelector* ui;
  std::vector<q::audio_device> m_audioDevices;
  int m_deviceIndex = -1;
  QVector<double> m_leftChannelData;
  QVector<double> m_rightChannelData;
  QCustomPlot* customPlot;
  paTestData data;
};

#endif  // AUDIODEVICESELECTOR_H
