#include "AudioDeviceSelector.h"
#include "ui_AudioDeviceSelector.h"

#include <vector>

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
        listModel->insertRow(listModel->rowCount());
        QModelIndex index = listModel->index(listModel->rowCount() - 1, 0);
        listModel->setData(index, QString::fromStdString(device.name()));
    }
    connect(ui->lv_deviceList, &QListWidget::doubleClicked, this, &AudioDeviceSelector::listSelected);
}

AudioDeviceSelector::~AudioDeviceSelector()
{
    delete ui;
}

void AudioDeviceSelector::listSelected(const QModelIndex &index)
{

}
