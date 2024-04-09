#include <QMdiSubWindow>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "LoginUi.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    LoginUi* test = new LoginUi();
    // QMdiSubWindow* sub = ui->mdiArea->addSubWindow(test); // todo: mdiArea class for background support
    sub->setWindowFlags(Qt::FramelessWindowHint);
    QCoreApplication::processEvents();
    int x = (this->geometry().width()) / 2  - ui->mdiArea->geometry().width();
    int y = (this->geometry().height()) / 2 - ui->mdiArea->geometry().height();
    sub->move(x, y);
    test->show();
}

MainWindow::~MainWindow()
{
    delete ui;
}
