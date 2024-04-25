#include "AudioDeviceSelector.h"

#include <QDateTime>
#include <iostream>
#include <vector>

#include "ui_AudioDeviceSelector.h"

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

  connect(ui->lv_deviceList, &QListWidget::doubleClicked, this,
          &AudioDeviceSelector::listSelected);
  // connect(ui->buttonBox, &QDialogButtonBox::accepted, this,
  // &AudioDeviceSelector::writeToFile);
  // connect(ui->buttonBox, &QDialogButtonBox::accepted, this,
  //         &AudioDeviceSelector::play);
}

AudioDeviceSelector::~AudioDeviceSelector() {
  delete customPlot;
  delete ui;
}

void AudioDeviceSelector::listSelected(const QModelIndex& index) {
  m_deviceIndex = index.row();
  if (startRecording() == 0) {
    writeToFile();
  }
  // this->drawGraph();
}

static inline float max(float a, float b) { return a > b ? a : b; }

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

void AudioDeviceSelector::drawLoop() {
  while (!m_stopFlag) {
    if (graphSize < m_leftChannelData.size()) drawGraph();
  }
}

void AudioDeviceSelector::drawGraph() {
  graphSize = m_leftChannelData.size();
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
  // customPlot->addGraph();
  // initGraph(m_rightChannelData, 1, -1);
  customPlot->graph(0)->rescaleAxes();
  // customPlot->graph(1)->rescaleAxes(true);
  customPlot->replot();
  QCoreApplication::processEvents();
}

int AudioDeviceSelector::startRecording() {
  initRecorder();
  return record();
}

//-------------------------------------------

void AudioDeviceSelector::initRecorder() {
  numChannels =
      (m_audioDevices[m_deviceIndex].input_channels() >= 2
           ? 2
           : (m_audioDevices[m_deviceIndex].input_channels() == 1 ? 1 : 0));
  sampleRate = m_audioDevices[m_deviceIndex].default_sample_rate();
  m_data = new paTestData();
}

int AudioDeviceSelector::recordCallback(
    const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags,
    void* userData) {
  AudioDeviceSelector* outerClass = (AudioDeviceSelector*)userData;
  paTestData* data = outerClass->m_data;
  const SAMPLE* rptr = (const SAMPLE*)inputBuffer;
  SAMPLE* wptr = &data->recordedSamples[data->frameIndex * data->numChannels];
  long framesToCalc;
  long i;
  int finished;
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

  // if (rptr == NULL) {
  //   for (i = 0; i < framesToCalc; i++) {
  //     *wptr++ = SAMPLE_SILENCE;                             /* left */
  //     if (data->numChannels == 2) *wptr++ = SAMPLE_SILENCE; /* right */
  //   }
  // } else {
  //   for (i = 0; i < framesToCalc; i++) {
  //     *wptr++ = *rptr++;                             /* left */
  //     if (data->numChannels == 2) *wptr++ = *rptr++; /* right */
  //   }
  // }
  for (unsigned long t = 0; t < data->numChannels; ++t) {
    for (i = 0; i < framesToCalc; ++i) {
      wptr[i] = rptr[data->numChannels * i + t];
      outerClass->m_leftChannelData.push_back(wptr[i]);
    }
  }
  data->frameIndex += framesToCalc;
  return finished;
}

static int checkErr(PaError err) {
  try {
    if (err != paNoError) {
      throw std::runtime_error("portaudio error:" +
                               std::string(Pa_GetErrorText(err)));
    }
  } catch (...) {
    return 1;
  }
  return 0;
}

void AudioDeviceSelector::terminate() {
  Pa_Terminate();
  if (m_data->recordedSamples) {
    delete[] m_data->recordedSamples;
  }
}

int AudioDeviceSelector::record() {
  if (numChannels == 0) {
    std::cout << "0 input channels";
    return 2;
  }
  m_data->numChannels = numChannels;
  PaError err = Pa_Initialize();
  if (checkErr(err) == 1) {
    terminate();
    return 1;
  }

  if (m_deviceIndex == -1) {
    std::cout << "Device not selected";
    return 1;
  }

  int numBytes;
  m_data->maxFrameIndex =
      RECORDED_SECONDS * m_audioDevices[m_deviceIndex].default_sample_rate();
  m_data->frameIndex = 0;
  numBytes = m_data->maxFrameIndex * numChannels * sizeof(SAMPLE);
  m_data->recordedSamples = new SAMPLE[numBytes];
  for (int i = 0; i < numBytes; i++) {
    m_data->recordedSamples[i] = 0;
  }

  std::cout << "Device id: " << m_audioDevices[m_deviceIndex].id() << "\n"
            << "  Name: " << m_audioDevices[m_deviceIndex].name() << "\n"
            << "  MaxInputChanels: "
            << m_audioDevices[m_deviceIndex].input_channels() << "\n"
            << "  MaxOutputChanels: "
            << m_audioDevices[m_deviceIndex].output_channels() << "\n"
            << "  DefaultSampleRate: "
            << m_audioDevices[m_deviceIndex].default_sample_rate() << "\n";

  PaStreamParameters inputParameters;
  inputParameters.channelCount = numChannels;
  inputParameters.device = m_audioDevices[m_deviceIndex].id();
  inputParameters.hostApiSpecificStreamInfo = NULL;
  inputParameters.sampleFormat = paInt16;
  inputParameters.suggestedLatency =
      Pa_GetDeviceInfo(m_audioDevices[m_deviceIndex].id())
          ->defaultLowInputLatency;

  PaStream* stream;
  err =
      Pa_OpenStream(&stream, &inputParameters, nullptr,
                    m_audioDevices[m_deviceIndex].default_sample_rate(),
                    FRAMES_PER_BUFFER, paDitherOff, this->recordCallback, this);

  if (checkErr(err) == 1) {
    terminate();
    return 1;
  }

  (void)QtConcurrent::run([=]() {
    Pa_StartStream(stream);
    Pa_Sleep(RECORDED_SECONDS * 1000);
    m_stopFlag = true;
  });
  drawLoop();
  if (checkErr(err) == 1) {
    terminate();
    return 1;
  }
  Pa_Sleep(RECORDED_SECONDS * 1000);

  err = Pa_StopStream(stream);
  if (checkErr(err) == 1) {
    terminate();
    return 1;
  }

  err = Pa_CloseStream(stream);
  if (checkErr(err) == 1) {
    terminate();
    return 1;
  }

  for (int i = 0; i < numBytes; ++i) {
    m_data->recordedSamples[i] /= 10;
  }

  err = Pa_Terminate();
  if (checkErr(err) == 1) {
    terminate();
    return 1;
  }
  return 0;
}
