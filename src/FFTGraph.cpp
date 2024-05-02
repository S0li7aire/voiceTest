#include "FFTGraph.h"

#include <fftw3.h>

#include "ui_FFTGraph.h"

static void applyFFT(double* inputSignal, int signalLength,
                     double* outputSpectrum);
static void int16ToDouble(const int16_t* inputSignal, int signalLength,
                          double* outputSignal);
static void removeCloseElements(QVector<double>& vec, double threshold);
static QVector<double> findFrequenciesAboveMagnitude(
    const QVector<double>& frequencies, const QVector<double>& magnitudes,
    double threshold);
static double findMedianMagnitude(const QVector<double>& magnitudes);
static QVector<double> generateSineWave(const QVector<double>& frequencies,
                                        const QVector<double>& magnitudes,
                                        const QVector<double>& xValues);

FFTGraph::FFTGraph(QWidget* parent) : QWidget(parent), ui(new Ui::FFTGraph) {
  ui->setupUi(this);
  m_plot = new QCustomPlot(this);
  ui->gl_mainGraph->addWidget(m_plot);
}

FFTGraph::~FFTGraph() { delete ui; }

void calculatePSD(double* data, int dataSize, int32_t sampleRate,
                  QCustomPlot* customPlot) {
  // Initialize FFTW3 variables
  fftw_complex* fftResult;
  fftw_plan plan;
  fftResult = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * dataSize);
  plan = fftw_plan_dft_r2c_1d(dataSize, data, fftResult, FFTW_ESTIMATE);

  // Perform FFT
  fftw_execute(plan);

  // Calculate Power Spectral Density (PSD) in dB
  QVector<double> frequencies(dataSize / 2);
  QVector<double> psd(dataSize / 2);
  const double scale = 1.0 / dataSize;  // Scale factor for PSD calculation
  for (int i = 0; i < dataSize / 2; ++i) {
    double freq = i * (1.0 / dataSize);  // Frequency bin
    frequencies[i] =
        freq * sampleRate;  // Convert bin to Hz assuming sampleRate is defined
    double mag = sqrt(fftResult[i][0] * fftResult[i][0] +
                      fftResult[i][1] * fftResult[i][1]);  // Magnitude
    psd[i] = 10 * log10(scale * mag * mag);                // PSD in dB
  }

  // Plot on QCustomPlot
  customPlot->addGraph();
  customPlot->graph(0)->setData(frequencies, psd);
  customPlot->xAxis->setLabel("Frequency (Hz)");
  customPlot->yAxis->setLabel("PSD (dB)");
  customPlot->setInteraction(QCP::iRangeDrag, true);
  customPlot->setInteraction(QCP::iRangeZoom, true);
  customPlot->rescaleAxes();  // Adjust axes ranges
  customPlot->replot();       // Refresh plot
}

void FFTGraph::drawFFT() {
  const std::size_t audioWaveLength = m_waveFile->getMaxFrameIndex() *
                                      m_waveFile->getNumChannels() *
                                      sizeof(SAMPLE);
  double* audioWave = new double[audioWaveLength];
  int16ToDouble(m_waveFile->getData(), audioWaveLength, audioWave);
  calculatePSD(audioWave, audioWaveLength, m_waveFile->getSampleRate(), m_plot);
  delete[] audioWave;
}

// void FFTGraph::drawFFT() {
//   if (m_waveFile->isEmpty()) return;
//   const std::size_t audioWaveLength = m_waveFile->getMaxFrameIndex() *
//                                       m_waveFile->getNumChannels() *
//                                       sizeof(SAMPLE);
//   int32_t sampleRate = m_waveFile->getSampleRate();
//   double* spectrum = new double[audioWaveLength / 2 + 1];
//   for (int i = 0; i < audioWaveLength / 2 + 1; ++i) i[spectrum] = 0;
//   double* audioWave = new double[audioWaveLength];
//   int16ToDouble(m_waveFile->getData(), audioWaveLength, audioWave);
//   applyFFT(audioWave, audioWaveLength, spectrum);
//   delete[] audioWave;

//   m_plot->addGraph();

//   QVector<double> frequencies(audioWaveLength / 2 + 1),
//       magnitudes(audioWaveLength / 2 + 1);
//   double frequencyStep = sampleRate / 2. / (audioWaveLength - 1);
//   for (int i = 0; i < audioWaveLength / 2 + 1; ++i) {
//     frequencies[i] = i * frequencyStep;
//     magnitudes[i] = spectrum[i];
//   }
//   m_plot->graph(0)->setData(frequencies, magnitudes);

//   m_plot->xAxis->setLabel("Frequency(Hz)");
//   m_plot->yAxis->setLabel("Magnitude");

//   m_plot->xAxis->setRange(
//       0, *std::max_element(frequencies.constBegin(),
//       frequencies.constEnd()));
//   m_plot->yAxis->setRange(
//       0, *std::max_element(magnitudes.constBegin(), magnitudes.constEnd()));

//   m_plot->plotLayout()->insertRow(0);
//   m_plot->plotLayout()->addElement(
//       0, 0,
//       new QCPTextElement(m_plot, "Magnitude Spectrum",
//                          QFont("sans", 12, QFont::Bold)));

//   m_plot->setInteraction(QCP::iRangeDrag, true);
//   m_plot->setInteraction(QCP::iRangeZoom, true);

//   m_plot->replot();
//   m_plot->show();

//   QVector<double> sinwavesFreq = findFrequenciesAboveMagnitude(
//       frequencies, magnitudes, findMedianMagnitude(magnitudes));
//   removeCloseElements(sinwavesFreq, 10);

//   {
//     const QVector<double> targetValues = {32,   64,   125,  250,   500, 1000,
//                                           2000, 4000, 8000, 16000, 22000};
//     const double threshold = 10.0;

//     for (int i = 0; i < sinwavesFreq.size();) {
//       bool remove = true;
//       for (double target : targetValues) {
//         if (std::abs(sinwavesFreq[i] - target) <= threshold) {
//           remove = false;
//           break;
//         }
//       }
//       if (remove) {
//         sinwavesFreq.remove(i);
//       } else {
//         ++i;
//       }
//     }
//   }

//   for (int i = 0; i < sinwavesFreq.size(); i++) {
//     QCustomPlot* customPlot = new QCustomPlot();

//     QVector<double> xData, yData;
//     const int numPoints = 100;
//     const double duration = 2 * M_PI;
//     const double step = duration / numPoints;

//     for (int j = 0; j < numPoints; ++j) {
//       double x = j * step;
//       double y = magnitudes[i] * sin(2 * M_PI * sinwavesFreq[i] * x);
//       xData.append(x);
//       yData.append(y);
//     }

//     customPlot->addGraph();
//     customPlot->graph(0)->setData(xData, yData);

//     customPlot->xAxis->setLabel("Time");
//     customPlot->yAxis->setLabel("Amplitude");

//     customPlot->plotLayout()->insertRow(0);
//     customPlot->plotLayout()->addElement(
//         0, 0,
//         new QCPTextElement(customPlot, "Sine Wave",
//                            QFont("sans", 12, QFont::Bold)));

//     customPlot->xAxis->setRange(0, duration);
//     customPlot->yAxis->setRange(
//         *std::min_element(yData.constBegin(), yData.constEnd()),
//         *std::max_element(yData.constBegin(), yData.constEnd()));

//     ui->gl_sinewaves1->addWidget(customPlot, i - static_cast<int>((i % 2)),
//                                  static_cast<int>(!(i % 2)));
//     customPlot->replot();
//     QCoreApplication::processEvents();
//   }

//   delete[] spectrum;
// }

void applyFFT(double* inputSignal, int signalLength, double* outputSpectrum) {
  fftw_complex* fftInput = reinterpret_cast<fftw_complex*>(
      fftw_malloc(sizeof(fftw_complex) * signalLength));
  fftw_complex* fftOutput = reinterpret_cast<fftw_complex*>(
      fftw_malloc(sizeof(fftw_complex) * (signalLength / 2 + 1)));

  fftw_plan plan =
      fftw_plan_dft_r2c_1d(signalLength, inputSignal, fftOutput, FFTW_ESTIMATE);

  fftw_execute(plan);

  for (int i = 0; i < signalLength / 2 + 1; ++i) {
    double magnitude = std::abs(fftOutput[i][0] + fftOutput[i][1]);
    outputSpectrum[i] = magnitude;
  }

  fftw_destroy_plan(plan);
  fftw_free(fftInput);
  fftw_free(fftOutput);
}

void int16ToDouble(const int16_t* inputSignal, int signalLength,
                   double* outputSignal) {
  for (int i = 0; i < signalLength; ++i)
    outputSignal[i] = static_cast<double>(i[inputSignal]) / 32768.;
}

void removeCloseElements(QVector<double>& vec, double threshold) {
  if (vec.size() < 2) return;

  for (int i = 0; i < vec.size() - 1;) {
    if (std::abs(vec[i] - vec[i + 1]) < threshold) {
      vec.remove(i + 1);
    } else {
      ++i;
    }
  }
}

QVector<double> findFrequenciesAboveMagnitude(
    const QVector<double>& frequencies, const QVector<double>& magnitudes,
    double threshold) {
  QVector<double> aboveMedian;
  const int size = frequencies.size();
  for (int i = 0; i < size; ++i) {
    if (magnitudes[i] > threshold) {
      aboveMedian.push_back(frequencies[i]);
    }
  }
  return aboveMedian;
}

double findMedianMagnitude(const QVector<double>& magnitudes) {
  QVector<double> sortedMagnitudes = magnitudes;
  std::sort(sortedMagnitudes.begin(), sortedMagnitudes.end());

  const int size = sortedMagnitudes.size();
  if (size == 0) {
    return 0.0;
  } else if (size % 2 == 0) {
    return (sortedMagnitudes[size / 2 - 1] + sortedMagnitudes[size / 2]) / 2.0;
  } else {
    return sortedMagnitudes[size / 2];
  }
}

QVector<double> generateSineWave(const QVector<double>& frequencies,
                                 const QVector<double>& magnitudes,
                                 const QVector<double>& xValues) {
  QVector<double> yValues(xValues.size(), 0.0);

  for (int i = 0; i < frequencies.size(); ++i) {
    double frequency = frequencies[i];
    double magnitude = magnitudes[i];

    for (int j = 0; j < xValues.size(); ++j) {
      yValues[j] += magnitude * std::sin(2 * M_PI * frequency * xValues[j]);
    }
  }

  return yValues;
}
