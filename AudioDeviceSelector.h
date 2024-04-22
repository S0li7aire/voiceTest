#ifndef AUDIODEVICESELECTOR_H
#define AUDIODEVICESELECTOR_H

#include <QDialog>

#include <q_io/audio_device.hpp>

namespace q = cycfi::q;
namespace Ui {
class AudioDeviceSelector;
}

class AudioDeviceSelector : public QDialog
{
    Q_OBJECT

public:
    explicit AudioDeviceSelector(QWidget *parent = nullptr);
    explicit AudioDeviceSelector(const std::vector<q::audio_device>& devices, QWidget *parent = nullptr);
    ~AudioDeviceSelector();

private slots:
    void listSelected(const QModelIndex &index);
    int portaudiotest();

private:
    Ui::AudioDeviceSelector *ui;
    std::vector<q::audio_device> m_audioDevices;
    int m_deviceIndex = -1;
};

#endif // AUDIODEVICESELECTOR_H
