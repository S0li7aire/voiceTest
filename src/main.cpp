#include <QApplication>
#include <iostream>

#include "MainWin.h"

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  MainWin w;
  w.show();
  return a.exec();
}