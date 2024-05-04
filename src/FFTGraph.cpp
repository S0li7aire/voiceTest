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

QString frequencyToNote(double frequency) {
  const double A4 = 440.0;                      // Frequency of A4 (in Hz)
  const double C0 = A4 * std::pow(2.0, -4.75);  // Frequency of C0
  const int numNotes = 12;                      // Number of notes in an octave
  const QStringList notes = {"C",  "C#", "D",  "D#", "E",  "F",
                             "F#", "G",  "G#", "A",  "A#", "B"};

  if (frequency < C0) {
    return "Below Range";
  }

  double h = round(12.0 * log2(frequency / C0));
  int octave = h / numNotes;
  int noteIndex = static_cast<int>(h) % numNotes;
  return notes[noteIndex] + QString::number(octave);
}

QVector<double> detectPeaks(const QVector<double>& psd) {
  QVector<double> peaks;
  for (int i = 1; i < psd.size() - 1; ++i) {
    if (psd[i] > psd[i - 1] && psd[i] > psd[i + 1]) {
      peaks.push_back(i);  // Store index of peak
    }
  }
  return peaks;
}

void FFTGraph::calculatePSD(double* data, int dataSize) {
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
        freq * m_waveFile->getSampleRate();  // Convert bin to Hz assuming
                                             // sampleRate is defined
    double mag = sqrt(fftResult[i][0] * fftResult[i][0] +
                      fftResult[i][1] * fftResult[i][1]);  // Magnitude
    psd[i] = 10 * log10(scale * mag * mag);                // PSD in dB
  }

  // Plot on QCustomPlot
  m_plot->addGraph();
  m_plot->graph(0)->setData(frequencies, psd);
  m_plot->xAxis->setLabel("Frequency (Hz)");
  m_plot->yAxis->setLabel("PSD (dB)");
  m_plot->setInteraction(QCP::iRangeDrag, true);
  m_plot->setInteraction(QCP::iRangeZoom, true);
  m_plot->rescaleAxes();
  m_plot->replot();

  QVector<double> peaks = detectPeaks(psd);
  QStringList notes;
  QMap<QString, int> notesCount;
  for (double peak : peaks) {
    double frequency = peak * m_waveFile->getSampleRate() /
                       dataSize;  // Convert bin index to frequency
    notes.push_back(frequencyToNote(frequency));
  }
  auto countNotes = [](QStringList& notes) {
    QMap<QString, int> counts;
    foreach (const QString& note, notes) {
      counts[note]++;
    }
    return counts;
  };
  notesCount = countNotes(notes);
  for (auto it = notesCount.constBegin(); it != notesCount.constEnd(); ++it) {
    if (it.value() >= 200) {
      QString itemText =
          QString("Hz, Note: %1; Frequency: %2").arg(it.key()).arg(it.value());
      QListWidgetItem* item = new QListWidgetItem(itemText);
      ui->lw_notes->addItem(item);
    }
  }
}

void FFTGraph::drawFFT() {
  const std::size_t audioWaveLength = m_waveFile->getMaxFrameIndex() *
                                      m_waveFile->getNumChannels() *
                                      sizeof(SAMPLE);
  double* audioWave = new double[audioWaveLength];
  int16ToDouble(m_waveFile->getData(), audioWaveLength, audioWave);
  calculatePSD(audioWave, audioWaveLength);
  delete[] audioWave;
}

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
