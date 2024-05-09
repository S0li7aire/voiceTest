#include "calculateFFT.h"

#include <fftw3.h>
#include <omp.h>

#include <QMap>
namespace FFT {
void calculatePSD(QVector<double>& frequencies, QVector<double>& psd,
                  int32_t sampleRate, int dataSize, fftw_complex* spectrum);
double calculateFrequency(fftw_complex* fft_result, int sample_rate, int n);
double noiceTestValue(double* data, int dataSize, double offset);
double calculateRMS(double* audio, int start, int end);
std::vector<std::pair<int, int>> divideIntoSegments(double* audioData,
                                                    int length,
                                                    double threshold);
double findMostCommonLowestValue(const double* data, int dataSize);
double calculateAverageAmplitude(double* audio, int size);
void performFFT(const double* signal, fftw_complex* fftResult,
                int signalLength);
double calculatePitch(const QVector<double>& harmonicFrequencies);
namespace notes {
inline QString frequencyToNote(double frequency);
inline QVector<double> detectPeaks(const QVector<double>& psd);
double calculateMapMedian(const QMap<QString, int>& data);
}  // namespace notes
QMap<QString, int> countNotes(const QVector<double>& psd, int32_t sampleRate,
                              int dataSize);
double spectralCentroid(fftw_complex* spectrum, int spectrumLength,
                        double sampleRate);
double spectralSpread(fftw_complex* spectrum, int spectrumLength,
                      double sampleRate, double centroid);
double zeroCrossingRate(double* signal, int signalLength);
double spectralBandwidth(fftw_complex* spectrum, int spectrumLength,
                         double sampleRate);
double estimateAmplitude(double* signal, int dataSize, double frequency,
                         int32_t sampleRate);
namespace pitch {
void autocorrelation(const double* signal, double* autocorr, int signalLength);
int findPitchPeak(const double* autocorr, int signalLength, int minPeriod,
                  int maxPeriod);
}  // namespace pitch
}  // namespace FFT

std::vector<FFT::voicePrint> FFT::calculateVoicePrint(
    QVector<QVector<double>>& frequencies, QVector<QVector<double>>& psd,
    double* data, int32_t sampleRate,
    std::vector<std::pair<int, int>> dataSize) {
  std::vector<voicePrint> print;
  int i = 0;
  for (auto& pair : dataSize) {
    fftw_complex* fftResult;
    int size = pair.second - pair.first;
    if (size <= 512) continue;
    fftResult = reinterpret_cast<fftw_complex*>(
        fftw_malloc(sizeof(fftw_complex) * size));
    performFFT(data, fftResult, size);
    frequencies.push_back({});
    psd.push_back({});
    calculatePSD(frequencies[i], psd[i], sampleRate, size, fftResult);
    print.emplace_back();
    print.back().averageAmplitude =
        calculateAverageAmplitude(data + pair.first, size);
    print.back().spectralCentroid =
        spectralCentroid(fftResult, size, sampleRate);
    print.back().spectralSpread = spectralSpread(fftResult, size, sampleRate,
                                                 print.back().spectralCentroid);
    print.back().zeroCrossingRate = zeroCrossingRate(data + pair.first, size);
    print.back().spectralBandwidth =
        spectralBandwidth(fftResult, size, sampleRate);
    fftw_free(fftResult);
    i++;
  }
  return print;
}

double FFT::calculateFrequency(fftw_complex* fft_result, int sample_rate,
                               int n) {
  double max_magnitude = 0;
  int max_index = 0;
  for (int i = 0; i < n / 2; ++i) {
    double magnitude = sqrt(fft_result[i][0] * fft_result[i][0] +
                            fft_result[i][1] * fft_result[i][1]);
    if (magnitude > max_magnitude) {
      max_magnitude = magnitude;
      max_index = i;
    }
  }
  double frequency = static_cast<double>(max_index) * sample_rate / n;
  return frequency;
}

double FFT::noiceTestValue(double* data, int dataSize, double offset) {
  std::unordered_map<double, int> countMap;
  for (int i = 0; i < dataSize; ++i) {
    double absValue = std::abs(data[i]);
    double closestValue = std::round(absValue / 0.00002) * 0.00002;
    countMap[closestValue]++;
  }
  double mostCommonValue = 0;
  int maxCount = 0;
  for (const auto& pair : countMap) {
    if (pair.second > maxCount && std::abs(pair.first) > offset) {
      mostCommonValue = pair.first;
      maxCount = pair.second;
    }
  }
  return mostCommonValue;
}

double FFT::calculateRMS(double* audio, int start, int end) {
  double sum = 0.0;
  for (int i = start; i < end; ++i) {
    sum += audio[i] * audio[i];
  }
  return sqrt(sum / (end - start));
}

std::vector<std::pair<int, int>> FFT::divideIntoSegments(double* audioData,
                                                         int length,
                                                         double threshold) {
  std::vector<std::pair<int, int>> segments;
  bool inSegment = false;
  int segmentStart = 0;
  for (int i = 0; i < length; ++i) {
    if (audioData[i] > threshold) {
      if (!inSegment) {
        inSegment = true;
        segmentStart = i;
      }
    } else {
      if (inSegment) {
        segments.push_back(std::make_pair(segmentStart, i));
        inSegment = false;
      }
    }
  }
  if (inSegment) {
    segments.push_back(std::make_pair(segmentStart, length - 1));
  }
  return segments;
}

double FFT::findMostCommonLowestValue(const double* data, int dataSize) {
  if (dataSize <= 0) {
    return 0.0;
  }
  double lowestValue = data[0];
  for (int i = 1; i < dataSize; ++i) {
    if (data[i] < lowestValue) {
      lowestValue = data[i];
    }
  }
  int count = 0;
  for (int i = 0; i < dataSize; ++i) {
    if (data[i] == lowestValue) {
      count++;
    }
  }
  return count;
}

double FFT::calculateAverageAmplitude(double* audio, int size) {
  double sum = 0.0;
  for (int i = 0; i < size; ++i) {
    sum += std::abs(audio[i]);
  }
  return sum / size;
}

void FFT::performFFT(const double* signal, fftw_complex* fftResult,
                     int signalLength) {
  fftw_plan plan = fftw_plan_dft_r2c_1d(
      signalLength, const_cast<double*>(signal), fftResult, FFTW_ESTIMATE);
  fftw_execute(plan);
  fftw_destroy_plan(plan);
}

void FFT::calculatePSD(QVector<double>& frequencies, QVector<double>& psd,
                       int32_t sampleRate, int dataSize,
                       fftw_complex* spectrum) {
  const double scale = 1.0 / dataSize;
  for (int i = 0; i < dataSize / 2; ++i) {
    double freq = i * (1.0 / dataSize);
    frequencies.push_back(freq * sampleRate);
    double mag = std::sqrt(spectrum[i][0] * spectrum[i][0] +
                           spectrum[i][1] * spectrum[i][1]);
    psd.push_back(10 * std::log10(scale * mag * mag));
  }
}

QMap<QString, int> FFT::countNotes(const QVector<double>& psd,
                                   int32_t sampleRate, int dataSize) {
  QVector<double> peaks = FFT::notes::detectPeaks(psd);
  QMap<QString, int> counts;
  for (const double& peak : peaks) {
    counts[FFT::notes::frequencyToNote(peak * sampleRate / dataSize)]++;
  }
  double median = notes::calculateMapMedian(counts);
  QMap<QString, int> result;
  for (auto it = counts.constBegin(); it != counts.constEnd(); ++it) {
    if (it.value() >= median) {
      result.insert(it.key(), it.value());
    }
  }
  return result;
}

inline QString FFT::notes::frequencyToNote(double frequency) {
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

inline QVector<double> FFT::notes::detectPeaks(const QVector<double>& psd) {
  QVector<double> peaks;
  for (int i = 1; i < psd.size() - 1; ++i) {
    if (psd[i] > psd[i - 1] && psd[i] > psd[i + 1]) {
      peaks.push_back(i);  // Store index of peak
    }
  }
  return peaks;
}

double FFT::spectralCentroid(fftw_complex* spectrum, int spectrumLength,
                             double sampleRate) {
  double sum = 0.0;
  double totalEnergy = 0.0;
  for (int i = 0; i < spectrumLength; ++i) {
    double frequency = i * sampleRate / spectrumLength;
    double magnitude =
        sqrt(spectrum[i][0] * spectrum[i][0] + spectrum[i][1] * spectrum[i][1]);
    sum += frequency * magnitude;
    totalEnergy += magnitude;
  }
  return sum / totalEnergy;
}

double FFT::spectralSpread(fftw_complex* spectrum, int spectrumLength,
                           double sampleRate, double centroid) {
  double sum = 0.0;
  double totalEnergy = 0.0;
  for (int i = 0; i < spectrumLength; ++i) {
    double frequency = i * sampleRate / spectrumLength;
    double magnitude = std::sqrt(spectrum[i][0] * spectrum[i][0] +
                                 spectrum[i][1] * spectrum[i][1]);
    sum += std::pow(frequency - centroid, 2) * magnitude;
    totalEnergy += magnitude;
  }
  return std::sqrt(sum / totalEnergy);
}

double FFT::zeroCrossingRate(double* signal, int signalLength) {
  int crossings = 0;
  for (int i = 1; i < signalLength; ++i) {
    if ((signal[i] >= 0 && signal[i - 1] < 0) ||
        (signal[i] < 0 && signal[i - 1] >= 0)) {
      crossings++;
    }
  }
  return static_cast<double>(crossings) / (signalLength - 1);
}

double FFT::spectralBandwidth(fftw_complex* spectrum, int spectrumLength,
                              double sampleRate) {
  double centroid = spectralCentroid(spectrum, spectrumLength, sampleRate);
  double sum = 0.0;
  double totalEnergy = 0.0;
  for (int i = 0; i < spectrumLength; ++i) {
    double frequency = i * sampleRate / spectrumLength;
    double magnitude =
        sqrt(spectrum[i][0] * spectrum[i][0] + spectrum[i][1] * spectrum[i][1]);
    sum += pow(frequency - centroid, 2) * magnitude;
    totalEnergy += magnitude;
  }

  return sqrt(sum / totalEnergy);
}

double FFT::notes::calculateMapMedian(const QMap<QString, int>& data) {
  int n = data.size();

  QVector<int> sortedValues;
  for (auto it = data.begin(); it != data.end(); ++it) {
    sortedValues.append(it.value());
  }
  std::sort(sortedValues.begin(), sortedValues.end());

  if (n % 2 == 0) {
    int mid1 = sortedValues[n / 2 - 1];
    int mid2 = sortedValues[n / 2];
    return (mid1 + mid2) / 2.0;
  } else {
    return sortedValues[n / 2];
  }
}

void FFT::pitch::autocorrelation(const double* signal, double* autocorr,
                                 int signalLength) {
  for (int lag = 0; lag < signalLength; ++lag) {
    autocorr[lag] = 0.0;
    for (int i = 0; i < signalLength - lag; ++i) {
      autocorr[lag] += signal[i] * signal[i + lag];
    }
  }
}

int FFT::pitch::findPitchPeak(const double* autocorr, int signalLength,
                              int minPeriod, int maxPeriod) {
  int maxIndex = minPeriod;
  double maxCorr = autocorr[minPeriod];
  for (int lag = minPeriod + 1; lag <= maxPeriod; ++lag) {
    if (autocorr[lag] > maxCorr) {
      maxCorr = autocorr[lag];
      maxIndex = lag;
    }
  }
  return maxIndex;
}

double FFT::estimateAmplitude(double* signal, int dataSize, double frequency,
                              int32_t sampleRate) {
  double maxAmplitude = 0.0;
  double period = sampleRate / frequency;
  for (size_t i = 0; i < dataSize; ++i) {
    if (i + period < dataSize) {
      if (signal[i] > signal[i - 1] && signal[i] > signal[i + 1]) {
        if (signal[i] > maxAmplitude) {
          maxAmplitude = signal[i];
        }
      }
    }
  }
  return maxAmplitude;
}
