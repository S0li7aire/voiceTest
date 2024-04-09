#include "MdiArea.h"
#include "ui_MdiArea.h"

MdiArea::MdiArea(QWidget *parent) :
    QMdiArea(parent),
    ui(new Ui::MdiArea)
{
    ui->setupUi(this);
}

MdiArea::~MdiArea()
{
    delete ui;
}
