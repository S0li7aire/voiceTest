#include "MdiArea.h"
#include "ui_MdiArea.h"

MdiArea::MdiArea(QWidget *parent)
  : QMdiArea(parent),
    ui(new Ui::MdiArea)
{
    ui->setupUi(this);
}

MdiArea::MdiArea(const QString& imagePath, QWidget *parent) :
    QMdiArea(parent),
    ui(new Ui::MdiArea)
{
    ui->setupUi(this);
    m_pixmap = new QPixmap(imagePath);
}

MdiArea::~MdiArea()
{
    delete m_pixmap;
    delete ui;
}
