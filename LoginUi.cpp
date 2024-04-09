#include "LoginUi.h"
#include "ui_LoginUi.h"

LoginUi::LoginUi(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginUi)
{
    ui->setupUi(this);
}

LoginUi::~LoginUi()
{
    delete ui;
}
