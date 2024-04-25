#ifndef LOGINUI_H
#define LOGINUI_H

#include <QWidget>
#include <QPainter>

namespace Ui {
class LoginUi;
}

class LoginUi : public QWidget
{
    Q_OBJECT

public:
    explicit LoginUi(QWidget *parent = nullptr);
    // LoginUi(const QString& imagePath, QWidget *parent = 0);
    ~LoginUi();

// protected:
//     void paintEvent(QPaintEvent *event)
//     {
//         QWidget::paintEvent(event);

//         QPainter painter(this);

//         // Calculate the logo position - the bottom right corner of the mdi area.
//         int x = width() - m_pixmap->width();
//         int y = height() - m_pixmap->height();
//         painter.drawPixmap(x, y, *m_pixmap);
//     }
private:
    // QPixmap *m_pixmap;
    Ui::LoginUi *ui;
};

#endif // LOGINUI_H
