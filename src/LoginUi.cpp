#include "LoginUi.h"
#include "ui_LoginUi.h"

LoginUi::LoginUi(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginUi)
{
    ui->setupUi(this);
    ui->label->setStyleSheet("QLabel { background-color : red; color : white; }");
    ui->lineEdit->setStyleSheet("QLineEdit { background-color : red; color : white; }");
    ui->pushButton->setStyleSheet("QPushButton { background-color : red; color : white; }");
}



LoginUi::~LoginUi()
{
    delete ui;
}
