#include <QMdiSubWindow>

#include "MainWin.h"
#include "ui_MainWin.h"
#include "LoginUi.h"

MainWin::MainWin(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWin)
{
    ui->setupUi(this);
    QString imagePath = "/ico/1.jpg";
    // m_mdiArea = new MdiArea(imagePath, this);
    LoginUi* test = new LoginUi();
    ui->gridLayout->addWidget(test);
    // QMdiSubWindow* sub = m_mdiArea->addSubWindow(test); // todo: mdiArea class for background support

    // sub->setWindowFlags(Qt::FramelessWindowHint);
    QCoreApplication::processEvents();
    // int x = (this->geometry().width()) / 2  - m_mdiArea->geometry().width();
    // int y = (this->geometry().height()) / 2 - m_mdiArea->geometry().height();
    // test->move(x, y);
    test->show();
}

MainWin::~MainWin()
{
    delete ui;
}
