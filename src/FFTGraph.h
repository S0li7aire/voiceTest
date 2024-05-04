#ifndef FFTGRAPH_H
#define FFTGRAPH_H

#include <QWidget>

#include "WavFile.h"
#include "qcustomplot.h"

namespace Ui {
class FFTGraph;
}

class FFTGraph : public QWidget {
  Q_OBJECT

 public:
  explicit FFTGraph(QWidget *parent = nullptr);
  ~FFTGraph();

 public slots:
  void drawFFT();
  inline void setWaveFile(WavFile *waveFile) { m_waveFile = waveFile; }

 private:
  void calculatePSD(double *data, int dataSize);

 private:
  Ui::FFTGraph *ui;
  QCustomPlot *m_plot;
  WavFile *m_waveFile = nullptr;
};

#endif  // FFTGRAPH_H
