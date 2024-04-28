#include "audioRecorder.h"

#include <iostream>

audioRecorder::audioRecorder(q::audio_device* device) : m_audioDevice(device) {
  int numChannels = (m_audioDevice->input_channels() >= 2
                         ? 2
                         : (m_audioDevice->input_channels() == 1 ? 1 : 0));
  sampleRate = m_audioDevice->default_sample_rate();
  m_waveFile = new WavFile(sampleRate, numChannels, 1, RECORDED_SECONDS);
}

audioRecorder::~audioRecorder() {
  if (m_waveFile != nullptr) {
    delete m_waveFile;
  }
}

int audioRecorder::recordCallback(const void* inputBuffer, void* outputBuffer,
                                  unsigned long framesPerBuffer,
                                  const PaStreamCallbackTimeInfo* timeInfo,
                                  PaStreamCallbackFlags statusFlags,
                                  void* userData) {
  WavFile* wavFile = (WavFile*)userData;
  const SAMPLE* rptr = (const SAMPLE*)inputBuffer;
  SAMPLE* wptr =
      &wavFile->getData()[wavFile->getFrameIndex() * wavFile->getNumChannels()];
  long framesToCalc;
  long i;
  int finished;
  unsigned long framesLeft =
      wavFile->getMaxFrameIndex() - wavFile->getFrameIndex();

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

  if (rptr == NULL) {
    for (i = 0; i < framesToCalc; i++) {
      *wptr++ = SAMPLE_SILENCE;                                     /* left */
      if (wavFile->getNumChannels() == 2) *wptr++ = SAMPLE_SILENCE; /* right */
    }
  } else {
    for (i = 0; i < framesToCalc; i++) {
      *wptr++ = *rptr++;                                     /* left */
      if (wavFile->getNumChannels() == 2) *wptr++ = *rptr++; /* right */
    }
  }
  wavFile->setFrameIndex(wavFile->getFrameIndex() + framesToCalc);
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

void audioRecorder::terminate() { Pa_Terminate(); }

int audioRecorder::record() {
  if (m_waveFile->getNumChannels() == 0) {
    std::cout << "0 input channels";
    return 2;
  }
  PaError err = Pa_Initialize();
  if (checkErr(err) == 1) {
    terminate();
    return 1;
  }

  m_waveFile->setMaxFrameIndex(RECORDED_SECONDS *
                               m_audioDevice->default_sample_rate());
  m_waveFile->setFrameIndex(0);
  m_waveFile->setData();

  PaStreamParameters inputParameters;
  inputParameters.channelCount = m_waveFile->getNumChannels();
  inputParameters.device = m_audioDevice->id();
  inputParameters.hostApiSpecificStreamInfo = NULL;
  inputParameters.sampleFormat = paInt16;
  inputParameters.suggestedLatency =
      Pa_GetDeviceInfo(m_audioDevice->id())->defaultLowInputLatency;

  PaStream* stream;
  err = Pa_OpenStream(&stream, &inputParameters, nullptr,
                      m_audioDevice->default_sample_rate(), FRAMES_PER_BUFFER,
                      paDitherOff, recordCallback, m_waveFile);
  if (checkErr(err) == 1) {
    terminate();
    return 1;
  }

  err = Pa_StartStream(stream);
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

  err = Pa_Terminate();
  if (checkErr(err) == 1) {
    return 1;
  }
  return 0;
}
