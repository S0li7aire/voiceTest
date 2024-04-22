#include "AudioDeviceSelector.h"
#include "ui_AudioDeviceSelector.h"

#include <q_io/audio_stream.hpp>
#include <q/support/audio_stream.hpp>

#include <vector>
#include <stdio.h>
#include <iostream>

AudioDeviceSelector::AudioDeviceSelector(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AudioDeviceSelector)
{
    ui->setupUi(this);
}

AudioDeviceSelector::AudioDeviceSelector(const std::vector<q::audio_device> &devices, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AudioDeviceSelector),
    m_audioDevices(devices)
{
    ui->setupUi(this);
    auto listModel = ui->lv_deviceList->model();
    for(auto& device : m_audioDevices)
    {
        QString deviceInfo("Device id: " + QString::number(device.id()) + "\n"
                           + "  Name: " + QString::fromStdString(device.name()) + "\n"
                           + "  MaxInputChanels: " + QString::number(device.input_channels()) + "\n"
                           + "  MaxOutputChanels: " + QString::number(device.output_channels()) + "\n"
                           + "  DefaultSampleRate: " + QString::number(device.default_sample_rate()));
        listModel->insertRow(listModel->rowCount());
        QModelIndex index = listModel->index(listModel->rowCount() - 1, 0);
        listModel->setData(index, deviceInfo);
    }
    connect(ui->lv_deviceList, &QListWidget::doubleClicked, this, &AudioDeviceSelector::listSelected);
}

AudioDeviceSelector::~AudioDeviceSelector()
{
    delete ui;
}

void AudioDeviceSelector::listSelected(const QModelIndex &index)
{
    m_deviceIndex = index.row();
    portaudiotest();
}

#include <portaudio.h>
static int checkErr(PaError err) {
    if(err != paNoError) {
        std::cout << "portaudio error:" << Pa_GetErrorText(err) << std::endl;
        return 1;
    }
    return 0;
}

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 512

static inline float max(float a, float b) {
    return a > b ? a : b;
}

static int patestCallback(
    const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags,
    void* userData
    ) {
    const float* in = static_cast<const float*>(inputBuffer);
    (void)outputBuffer;

    int dispSize = 100;
    printf("\r");

    float vol_l = 0;
    float vol_r = 0;

    for(unsigned long i = 0; i < framesPerBuffer * 2; i += 2) {
        vol_l = max(vol_l, std::abs(in[i]));
        vol_r = max(vol_r, std::abs(in[i+1]));
    }

    for(int i = 0; i < dispSize; ++i) {
        float barProportion = i / static_cast<float>(dispSize);
        if(barProportion <= vol_l && barProportion <= vol_r) {
            printf("█");
        } else if (barProportion <= vol_l) {
            printf("▀");
        } else if (barProportion <= vol_r) {
            printf("▄");
        } else {
            printf("");
        }
    }

    fflush(stdout);

    return 0;
}

int AudioDeviceSelector::portaudiotest()
{
    PaError err = Pa_Initialize();
    checkErr(err);

    if(m_deviceIndex == -1) {
        std::cout << "Device not selected" << std::endl;
        return 1;
    }

    q::audio_device device = m_audioDevices[m_deviceIndex];

    std::cout << "Device id: " << m_deviceIndex << std::endl;
    std::cout << "  Name: " << device.name() << std::endl;
    std::cout << "  MaxInputChanels: " << device.input_channels() << std::endl;
    std::cout << "  MaxOutputChanels: " << device.output_channels() << std::endl;
    std::cout << "  DefaultSampleRate: " << device.default_sample_rate() << std::endl;

    PaStreamParameters inputParameters;
    memset(&inputParameters, 0, sizeof(inputParameters));
    inputParameters.channelCount = device.input_channels();
    inputParameters.device = m_deviceIndex;
    inputParameters.hostApiSpecificStreamInfo = NULL;
    inputParameters.sampleFormat = paFloat32;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(m_deviceIndex)->defaultLowInputLatency;


    PaStreamParameters outputParameters;
    memset(&outputParameters, 0, sizeof(outputParameters));
    outputParameters.channelCount = device.output_channels();
    outputParameters.device = m_deviceIndex;
    outputParameters.hostApiSpecificStreamInfo = NULL;
    outputParameters.sampleFormat = paFloat32;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(m_deviceIndex)->defaultLowInputLatency;

    PaStream* stream;
    err = Pa_OpenStream(
            &stream,
            &inputParameters,
            &outputParameters,
            SAMPLE_RATE,
            FRAMES_PER_BUFFER,
            paNoFlag,
            patestCallback,
            NULL
        );
    checkErr(err);

    err = Pa_StartStream(stream);
    checkErr(err);

    Pa_Sleep(10 * 1000);

    err = Pa_StopStream(stream);
    checkErr(err);

    err = Pa_CloseStream(stream);
    checkErr(err);

    err = Pa_Terminate();
    checkErr(err);
    return 0;
}
