#ifndef MDIAREA_H
#define MDIAREA_H

#include <QWidget>
#include <QMdiArea>
#include <QPainter>

namespace Ui {
class MdiArea;
}

class MdiArea : public QMdiArea
{
public:
    MdiArea(QWidget*);
    MdiArea(const QString& imagePath, QWidget *parent = 0)
        :
            QMdiArea(parent),
            m_pixmap(imagePath)
    {}
    ~MdiArea();
protected:
    void paintEvent(QPaintEvent *event)
    {
        QMdiArea::paintEvent(event);

        QPainter painter(viewport());

        // Calculate the logo position - the bottom right corner of the mdi area.
        int x = width() - m_pixmap.width();
        int y = height() - m_pixmap.height();
        painter.drawPixmap(x, y, m_pixmap);
    }
private:
    // Store the logo image.
    QPixmap m_pixmap;
    Ui::MdiArea *ui;
};
#endif // MDIAREA_H
