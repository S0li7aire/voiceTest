#ifndef FFTGRAPH_H
#define FFTGRAPH_H

#include <QWidget>

#include "WavFile.h"
#include "calculateFFT.h"
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
  void drawPSD(double *data, int dataSize,
               std::vector<std::pair<int, int>> segments);
  void writeToFile(const QVector<QVector<double>> &freq,
                   const QVector<QVector<double>> &psd,
                   const QMap<QString, int> &notes,
                   const std::vector<FFT::voicePrint> &print);
  void readFile(const std::string &name, const QVector<QVector<double>> &freq,
                const QVector<QVector<double>> &psd,
                const QMap<QString, int> &notes,
                const std::vector<FFT::voicePrint> &print);
  double compareVectors(const QVector<QVector<double>> &vec1,
                        const QVector<QVector<double>> &vec2);
  double compareMaps(const QMap<QString, int> &map1,
                     const QMap<QString, int> &map2);
  double calculatePercentageDifference(const std::vector<FFT::voicePrint> &vp1,
                                       const std::vector<FFT::voicePrint> &vp2);

 private:
  Ui::FFTGraph *ui;
  WavFile *m_waveFile = nullptr;
};

#endif  // FFTGRAPH_H
