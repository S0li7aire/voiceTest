#include <QMdiSubWindow>

#include "MainWin.h"
#include "ui_MainWin.h"
#include "AudioDeviceSelector.h"

#include <typeinfo>

MainWin::MainWin(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWin)
{
    ui->setupUi(this);
    AudioDeviceSelector* selector = new AudioDeviceSelector(q::audio_device::list(), this);
    ui->gl_main->addWidget(selector);
    selector->setAttribute(Qt::WA_DeleteOnClose, true);
    selector->show();
}

MainWin::~MainWin()
{
    delete ui;
}
