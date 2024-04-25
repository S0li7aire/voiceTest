#include "audioRecorder.h"

#include <QCoreApplication>
#include <iostream>

// audioRecorder::audioRecorder(q::audio_device* device) {
//   m_audioDevice = device;
//   numChannels = (m_audioDevices[m_deviceIndex].input_channels() >= 2
//                      ? 2
//                      : (m_audioDevices[m_deviceIndex].input_channels() == 1 ?
//                      1 : 0));
//   sampleRate = m_audioDevices[m_deviceIndex].default_sample_rate();
//   m_data = new paTestData();
// }

audioRecorder::audioRecorder(int deviceIndex, QVector<double>* graphData)
    : m_deviceIndex(deviceIndex), graphData(graphData) {
  m_audioDevices = q::audio_device::list();
  numChannels =
      (m_audioDevices[m_deviceIndex].input_channels() >= 2
           ? 2
           : (m_audioDevices[m_deviceIndex].input_channels() == 1 ? 1 : 0));
  sampleRate = m_audioDevices[m_deviceIndex].default_sample_rate();
  m_data = new paTestData();
}

audioRecorder::~audioRecorder() { delete m_data; }

int audioRecorder::recordCallback(const void* inputBuffer, void* outputBuffer,
                                  unsigned long framesPerBuffer,
                                  const PaStreamCallbackTimeInfo* timeInfo,
                                  PaStreamCallbackFlags statusFlags,
                                  void* userData) {
  audioRecorder* outerClass = (audioRecorder*)userData;
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
      outerClass->graphData->push_back(wptr[i]);
    }
  }
  outerClass->sendUpdateGraph();
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

void audioRecorder::terminate() {
  Pa_Terminate();
  if (m_data->recordedSamples) {
    delete[] m_data->recordedSamples;
  }
}

int audioRecorder::record() {
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
  inputParameters.sampleFormat = paFloat32;
  inputParameters.suggestedLatency =
      Pa_GetDeviceInfo(m_audioDevices[m_deviceIndex].id())
          ->defaultLowInputLatency;

  PaStream* stream;
  err = Pa_OpenStream(&stream, &inputParameters, nullptr,
                      m_audioDevices[m_deviceIndex].default_sample_rate(),
                      FRAMES_PER_BUFFER, paClipOff, this->recordCallback, this);

  if (checkErr(err) == 1) {
    terminate();
    return 1;
  }

  err = Pa_StartStream(stream);
  if (checkErr(err) == 1) {
    terminate();
    return 1;
  }
  QCoreApplication::processEvents();
  Pa_Sleep(RECORDED_SECONDS * 1000);
  QCoreApplication::processEvents();
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
