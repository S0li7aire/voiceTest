#ifndef CALCULATEFFT_H
#define CALCULATEFFT_H

#include <QVector>
#include <cstdint>
namespace FFT {
typedef struct voicePrint {
  double averageAmplitude;
  double spectralCentroid;
  double spectralSpread;
  double zeroCrossingRate;
  double spectralBandwidth;
  double pitch;
  double harmonicNoise;
  std::vector<std::pair<int, int>> segments;
} voicePrint;
inline void int16ToDouble(const int16_t* inputSignal, int signalLength,
                          double* outputSignal) {
  for (int i = 0; i < signalLength; ++i)
    outputSignal[i] = static_cast<double>(i[inputSignal]) / 32768.;
}
std::vector<voicePrint> calculateVoicePrint(
    QVector<QVector<double>>& frequencies, QVector<QVector<double>>& psd,
    double* data, int32_t sampleRate,
    std::vector<std::pair<int, int>> segments);
QMap<QString, int> countNotes(const QVector<double>& psd, int32_t sampleRate,
                              int dataSize);
}  // namespace FFT
#endif  // CALCULATEFFT_H
