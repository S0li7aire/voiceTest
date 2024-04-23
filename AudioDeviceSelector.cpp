#include "AudioDeviceSelector.h"

#include <stdio.h>

#include <QDateTime>
#include <iostream>
#include <vector>

#include "ui_AudioDeviceSelector.h"

AudioDeviceSelector::AudioDeviceSelector(QWidget* parent)
    : QDialog(parent), ui(new Ui::AudioDeviceSelector) {
  ui->setupUi(this);
}

AudioDeviceSelector::AudioDeviceSelector(
    const std::vector<q::audio_device>& devices, QWidget* parent)
    : QDialog(parent),
      ui(new Ui::AudioDeviceSelector),
      m_audioDevices(devices) {
  ui->setupUi(this);

  customPlot = new QCustomPlot(this);
  customPlot->setMinimumSize(QSize(500, 500));
  customPlot->xAxis->setTicks(false);
  customPlot->yAxis->setTicks(false);
  customPlot->xAxis->setTickLabels(false);
  customPlot->yAxis->setTickLabels(false);
  customPlot->xAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
  customPlot->yAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
  QLinearGradient plotGradient;
  plotGradient.setStart(0, 0);
  plotGradient.setFinalStop(0, 350);
  plotGradient.setColorAt(0, QColor(80, 80, 80));
  plotGradient.setColorAt(1, QColor(50, 50, 50));
  customPlot->setBackground(plotGradient);

  ui->gl_graph->addWidget(customPlot);
  auto listModel = ui->lv_deviceList->model();
  for (auto& device : m_audioDevices) {
    QString deviceInfo(
        "Device id: " + QString::number(device.id()) + "\n" +
        "  Name: " + QString::fromStdString(device.name()) + "\n" +
        "  MaxInputChanels: " + QString::number(device.input_channels()) +
        "\n" +
        "  MaxOutputChanels: " + QString::number(device.output_channels()) +
        "\n" + "  DefaultSampleRate: " +
        QString::number(device.default_sample_rate()));
    listModel->insertRow(listModel->rowCount());
    QModelIndex index = listModel->index(listModel->rowCount() - 1, 0);
    listModel->setData(index, deviceInfo);
  }

  data = new paTestData();

  connect(ui->lv_deviceList, &QListWidget::doubleClicked, this,
          &AudioDeviceSelector::listSelected);
  connect(ui->buttonBox, &QDialogButtonBox::accepted, this,
          &AudioDeviceSelector::writeToWav);
  // connect(ui->buttonBox, &QDialogButtonBox::accepted, this,
  //         &AudioDeviceSelector::play);
}

AudioDeviceSelector::~AudioDeviceSelector() {
  delete customPlot;
  delete ui;
}

void AudioDeviceSelector::listSelected(const QModelIndex& index) {
  m_deviceIndex = index.row();
  if (portaudiotest() == 0) {
    for (auto& value : m_leftChannelData) {
      std::cout << value << " ";
    }
    std::cout << std::endl;
    for (auto& value : m_leftChannelData) {
      std::cout << value << " ";
    }
  }
  this->drawGraph();
}

static int checkErr(PaError err) {
  try {
    if (err != paNoError) {
      throw std::runtime_error("portaudio error:" +
                               std::string(Pa_GetErrorText(err)));
    }
  } catch (const std::runtime_error& error) {
    QMessageBox::critical(nullptr, "Error", error.what());
    return 1;
  }
  return 0;
}

static inline float max(float a, float b) { return a > b ? a : b; }

int AudioDeviceSelector::recordCallback(
    const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags,
    void* userData) {
  const float* rptr = static_cast<const float*>(inputBuffer);
  long framesToCalc;
  long i;
  int finished;
  AudioDeviceSelector* th = (AudioDeviceSelector*)userData;
  paTestData* data = th->data;
  SAMPLE* wptr = &data->recordedSamples[data->frameIndex *
                                        th->m_audioDevices[th->m_deviceIndex]
                                            .input_channels()];
  unsigned long framesLeft = data->maxFrameIndex - data->frameIndex;

  (void)outputBuffer; /* Prevent unused variable warnings. */
  (void)timeInfo;
  (void)statusFlags;

  if (framesLeft < framesPerBuffer) {
    framesToCalc = framesLeft;
    finished = paComplete;
  } else {
    framesToCalc = framesPerBuffer;
    finished = paContinue;
  }

  int dispSize =
      max(th->ui->pb_volume_l->maximum(), th->ui->pb_volume_r->maximum());
  float vol_l = SAMPLE_SILENCE;
  float vol_r = SAMPLE_SILENCE;

  for (i = 0; i < framesPerBuffer * 2; i += 2) {
    vol_l = max(vol_l, std::abs(rptr[i]));
    vol_r = max(vol_r, std::abs(rptr[i + 1]));
  }

  std::cout << *rptr << std::endl;

  if (inputBuffer == NULL) {
    for (i = 0; i < framesToCalc; i++) {
      *wptr++ = SAMPLE_SILENCE; /* left */
      if (th->m_audioDevices[th->m_deviceIndex].input_channels() >= 2)
        *wptr++ = SAMPLE_SILENCE; /* right */
    }
  } else {
    for (i = 0; i < framesToCalc; i++) {
      *wptr++ = *rptr++; /* left */
      if (th->m_audioDevices[th->m_deviceIndex].input_channels() >= 2)
        *wptr++ = *rptr++; /* right */
    }
  }
  data->frameIndex += framesToCalc;

  int bar_l_value = 0;
  int bar_r_value = 0;

  for (i = 0; i < dispSize; ++i) {
    float barProportion = i / static_cast<float>(dispSize);
    if (barProportion <= vol_l && barProportion <= vol_r) {
      bar_l_value++;
      bar_r_value++;
    } else if (barProportion <= vol_l) {
      bar_l_value++;
    } else if (barProportion <= vol_r) {
      bar_r_value++;
    }
  }

  th->ui->pb_volume_l->setValue(bar_l_value);
  th->ui->pb_volume_r->setValue(bar_r_value);
  th->m_leftChannelData.push_back(bar_l_value);
  th->m_rightChannelData.push_back(bar_r_value);
  th->drawGraph();
  QCoreApplication::processEvents();

  return finished;
}

static int playCallback(const void* inputBuffer, void* outputBuffer,
                        unsigned long framesPerBuffer,
                        const PaStreamCallbackTimeInfo* timeInfo,
                        PaStreamCallbackFlags statusFlags, void* userData) {
  // paTestData* data = (paTestData*)userData;
  // SAMPLE* rptr = &data->recordedSamples[data->frameIndex * NUM_CHANNELS];
  // SAMPLE* wptr = (SAMPLE*)outputBuffer;
  // unsigned int i;
  // int finished;
  // unsigned int framesLeft = data->maxFrameIndex - data->frameIndex;

  // (void)inputBuffer; /* Prevent unused variable warnings. */
  // (void)timeInfo;
  // (void)statusFlags;
  // (void)userData;

  // if (framesLeft < framesPerBuffer) {
  //   /* final buffer... */
  //   for (i = 0; i < framesLeft; i++) {
  //     *wptr++ = *rptr++;                        /* left */
  //     if (NUM_CHANNELS == 2) *wptr++ = *rptr++; /* right */
  //   }
  //   for (; i < framesPerBuffer; i++) {
  //     *wptr++ = 0;                        /* left */
  //     if (NUM_CHANNELS == 2) *wptr++ = 0; /* right */
  //   }
  //   data->frameIndex += framesLeft;
  //   finished = paComplete;
  // } else {
  //   for (i = 0; i < framesPerBuffer; i++) {
  //     *wptr++ = *rptr++;                        /* left */
  //     if (NUM_CHANNELS == 2) *wptr++ = *rptr++; /* right */
  //   }
  //   data->frameIndex += framesPerBuffer;
  //   finished = paContinue;
  // }
  // return finished;
}

int AudioDeviceSelector::portaudiotest() {
  PaError err = Pa_Initialize();
  if (checkErr(err)) terminate();

  if (m_deviceIndex == -1) {
    std::cout << "Device not selected" << std::endl;
    return 1;
  }
  int totalFrames;
  int numSamples;
  int numBytes;
  data->maxFrameIndex = totalFrames =
      RECORDED_SECONDS * SAMPLE_RATE; /* Record for a few seconds. */
  data->frameIndex = 0;
  numSamples = totalFrames * m_audioDevices[m_deviceIndex].input_channels();
  numBytes = numSamples * sizeof(SAMPLE);
  data->recordedSamples = (SAMPLE*)malloc(
      numBytes); /* From now on, recordedSamples is initialised. */
  if (data->recordedSamples == NULL) {
    printf("Could not allocate record array.\n");
    return 1;
  }
  for (int i = 0; i < numSamples; i++) data->recordedSamples[i] = 0;

  q::audio_device device = m_audioDevices[m_deviceIndex];

  std::cout << "Device id: " << m_deviceIndex << std::endl;
  std::cout << "  Name: " << device.name() << std::endl;
  std::cout << "  MaxInputChanels: " << device.input_channels() << std::endl;
  std::cout << "  MaxOutputChanels: " << device.output_channels() << std::endl;
  std::cout << "  DefaultSampleRate: " << device.default_sample_rate()
            << std::endl;

  PaStreamParameters inputParameters;
  memset(&inputParameters, 0, sizeof(inputParameters));
  inputParameters.channelCount = device.input_channels();
  inputParameters.device = m_deviceIndex;
  inputParameters.hostApiSpecificStreamInfo = NULL;
  inputParameters.sampleFormat = paFloat32;
  inputParameters.suggestedLatency =
      Pa_GetDeviceInfo(m_deviceIndex)->defaultLowInputLatency;

  // PaStreamParameters outputParameters;
  // memset(&outputParameters, 0, sizeof(outputParameters));
  // outputParameters.channelCount = device.output_channels();
  // outputParameters.device = m_deviceIndex;
  // outputParameters.hostApiSpecificStreamInfo = NULL;
  // outputParameters.sampleFormat = paFloat32;
  // outputParameters.suggestedLatency =
  //     Pa_GetDeviceInfo(m_deviceIndex)->defaultLowInputLatency;

  PaStream* stream;
  err = Pa_OpenStream(&stream, &inputParameters, nullptr/*&outputParameters*/, SAMPLE_RATE,
                      FRAMES_PER_BUFFER, paClipOff, this->recordCallback, this);
  if (checkErr(err)) terminate();

  err = Pa_StartStream(stream);
  if (checkErr(err)) terminate();

  Pa_Sleep(RECORDED_SECONDS * 1000);

  err = Pa_StopStream(stream);
  if (checkErr(err)) terminate();

  err = Pa_CloseStream(stream);
  if (checkErr(err)) terminate();

  err = Pa_Terminate();
  if (checkErr(err)) return 1;
  return 0;
}

void AudioDeviceSelector::play() {
  //   data.frameIndex = 0;

  //   PaStreamParameters outputParameters;
  //   memset(&outputParameters, 0, sizeof(outputParameters));
  //   outputParameters.device = m_deviceIndex;
  //   outputParameters.hostApiSpecificStreamInfo = NULL;
  //   outputParameters.sampleFormat = paFloat32;
  //   outputParameters.suggestedLatency =
  //       Pa_GetDeviceInfo(m_deviceIndex)->defaultLowInputLatency;

  //   outputParameters.device =
  //       Pa_GetDefaultOutputDevice(); /* default output device */
  //   if (outputParameters.device == paNoDevice) {
  //     fprintf(stderr, "Error: No default output device.\n");
  //     terminate();
  //   }
  //   outputParameters.channelCount = 2; /* stereo output */
  //   outputParameters.sampleFormat = paFloat32;
  //   outputParameters.suggestedLatency =
  //       Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
  //   outputParameters.hostApiSpecificStreamInfo = NULL;

  //   printf("\n=== Now playing back. ===\n");
  //   fflush(stdout);
  //   PaStream* stream;
  //   PaError err = Pa_OpenStream(&stream, NULL, /* no input */
  //                               &outputParameters, SAMPLE_RATE,
  //                               FRAMES_PER_BUFFER, paClipOff, /* we won't
  //                               output out of range samples
  //                                           so don't bother clipping them */
  //                               playCallback, &data);
  //   if (err != paNoError) terminate();

  //   if (stream) {
  //     err = Pa_StartStream(stream);
  //     if (err != paNoError) terminate();

  //     printf("Waiting for playback to finish.\n");
  //     fflush(stdout);

  //     while ((err = Pa_IsStreamActive(stream)) == 1) Pa_Sleep(100);
  //     if (err < 0) terminate();

  //     err = Pa_CloseStream(stream);
  //     if (err != paNoError) terminate();

  //     printf("Done.\n");
  //     fflush(stdout);
  //   }
}

void AudioDeviceSelector::terminate() {
  Pa_Terminate();
  if (data->recordedSamples) /* Sure it is NULL or valid. */
    free(data->recordedSamples);
  // if (err != paNoError) {
  // fprintf(stderr, "An error occured while using the portaudio stream\n");
  // fprintf(stderr, "Error number: %d\n", err);
  // fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
  //   err = 1; /* Always return 0 or 1, but no other return codes. */
  // }
  this->close();
}

void AudioDeviceSelector::writeToWav() {
  // FILE* fid;
  // fid = fopen("recorded.raw", "wb");
  // if (fid == NULL) {
  //   printf("Could not open file.");
  // } else {
  //   fwrite(data.recordedSamples, NUM_CHANNELS * sizeof(SAMPLE),
  //          data.maxFrameIndex, fid);
  //   fclose(fid);
  //   printf("Wrote data to 'recorded.raw'\n");
  // }
    std::ofstream wavF("recorded.wav", std::ios::out | std::ios::binary | std::ios::app);
    if(!wavF)
        throw std::runtime_error("CannotOpenFile");
    wavFile file;
    file.numChannels = NUM_CHANNELS;
    file.sampleRate = SAMPLE_RATE;
    file.bitsPerSample = sizeof(SAMPLE) * 8;
    int NUM_SAMPLES = (file.sampleRate * RECORDED_SECONDS);
    file.byteRate = file.sampleRate * NUM_CHANNELS * file.bitsPerSample / 8;
    file.blockAllign = NUM_CHANNELS * file.bitsPerSample / 8;
    file.subchunk1Size = sizeof(file.audioFormat) + sizeof(file.numChannels) +
                         sizeof(file.sampleRate) + sizeof(file.byteRate) +
                         sizeof(file.blockAllign) + sizeof(file.blockAllign);
    file.subchunk2Size = NUM_SAMPLES + NUM_CHANNELS + file.bitsPerSample / 8;
    file.chunkSize = 4 + (8 + file.subchunk1Size) + (8 + file.subchunk2Size);
    file.data = data->recordedSamples;

    std::vector<float> tdata;
    wavF.write((char*)&file, sizeof(wavFile));
    for(int i = 0; i < NUM_SAMPLES; ++i) {
        wavF.write((char*)&file.data, sizeof(SAMPLE));
        tdata.push_back(file.data[i]);
    }
    tdata;
    wavF.close();
    if(!wavF.good()) {
        std::cout << "Error occurred at writing time!" << std::endl;
    }
}

void AudioDeviceSelector::drawGraph() {
  customPlot->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom));

  auto initGraph = [this](QVector<double> data, int index, int direction) {
    QVector<double> indexes(data.size());
    for (int i = 0; i < data.size(); ++i) {
      indexes[i] = i;
    }
    for (auto& item : data) {
      item *= direction;
    }
    QColor color(20 + 200 / 4.0 * 1, 70 * (1.6 - (index + 1.0) / 4.0), 150,
                 150);
    customPlot->graph(index)->setLineStyle(QCPGraph::lsLine);
    customPlot->graph(index)->setPen(QPen(color.lighter(200)));
    customPlot->graph(index)->setBrush(QBrush(color));
    customPlot->graph(index)->setData(indexes, data, true);
  };

  customPlot->addGraph();
  initGraph(m_leftChannelData, 0, 1);
  customPlot->addGraph();
  initGraph(m_rightChannelData, 1, -1);
  customPlot->graph(0)->rescaleAxes();
  customPlot->graph(1)->rescaleAxes(true);
  customPlot->replot();
}
