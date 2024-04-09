#ifndef MAINWIN_H
#define MAINWIN_H

#include <QMainWindow>

#include "MdiArea.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWin;
}
QT_END_NAMESPACE

class MainWin : public QMainWindow
{
    Q_OBJECT

public:
    MainWin(QWidget *parent = nullptr);
    ~MainWin();

private:
    MdiArea* m_mdiArea;
    Ui::MainWin *ui;
};
#endif // MAINWIN_H
