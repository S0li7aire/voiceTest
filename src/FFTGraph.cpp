#include "FFTGraph.h"

#include <fstream>

#include "ui_FFTGraph.h"

std::vector<std::pair<int, int>> detectRegions(const SAMPLE* audioData,
                                               int length, int noiseThreshold) {
  std::vector<std::pair<int, int>> soundRegions;
  bool inSoundRegion = false;
  int startIdx = 0;
  for (int i = 0; i < length; ++i) {
    if (std::abs(audioData[i]) > noiseThreshold) {
      if (!inSoundRegion) {
        startIdx = i;
        inSoundRegion = true;
      }
    } else {
      if (inSoundRegion) {
        soundRegions.push_back(std::make_pair(startIdx, i - 1));
        inSoundRegion = false;
      }
    }
  }
  if (inSoundRegion) {
    soundRegions.push_back(std::make_pair(startIdx, length - 1));
  }

  std::vector<std::pair<int, int>> nSoundRegions;
  nSoundRegions.push_back(std::make_pair(soundRegions[0].first, 0));
  int last = nSoundRegions[0].first;
  for (int i = 0; i < soundRegions.size(); ++i) {
    if (soundRegions[i].second > last + 10000) {
      nSoundRegions[nSoundRegions.size() - 1].second =
          soundRegions[i - 1].second;
      nSoundRegions.push_back(std::make_pair(soundRegions[i].first, 0));
    }
    last = soundRegions[i].second;
  }
  nSoundRegions[nSoundRegions.size() - 1].second =
      soundRegions[soundRegions.size() - 1].second;
  return nSoundRegions;
}

FFTGraph::FFTGraph(QWidget* parent) : QWidget(parent), ui(new Ui::FFTGraph) {
  ui->setupUi(this);
}

FFTGraph::~FFTGraph() { delete ui; }

void FFTGraph::drawPSD(double* data, int dataSize,
                       std::vector<std::pair<int, int>> segments) {
  QVector<QVector<double>> frequencies;
  QVector<QVector<double>> psd;
  std::vector<FFT::voicePrint> print = FFT::calculateVoicePrint(
      frequencies, psd, data, m_waveFile->getSampleRate(), segments);
  QMap<QString, int> notesCount;
  for (int i = 0; i < frequencies.size(); ++i) {
    QCustomPlot* m_plot = new QCustomPlot(this);
    ui->gl_mainGraph->addWidget(m_plot);
    m_plot->addGraph();
    m_plot->graph(0)->setData(frequencies[i], psd[i]);
    m_plot->xAxis->setLabel("Frequency (Hz)");
    m_plot->yAxis->setLabel("PSD (dB)");
    m_plot->setInteraction(QCP::iRangeDrag, true);
    m_plot->setInteraction(QCP::iRangeZoom, true);
    m_plot->rescaleAxes();
    m_plot->replot();
    notesCount = FFT::countNotes(psd[i], m_waveFile->getSampleRate(), dataSize);
    for (auto it = notesCount.constBegin(); it != notesCount.constEnd(); ++it) {
      QString itemText =
          QString("Hz, Note: %1; Frequency: %2").arg(it.key()).arg(it.value());
      QListWidgetItem* item = new QListWidgetItem(itemText);
      ui->lw_notes->addItem(item);
    }
  }
  ui->lw_notes->addItem(new QListWidgetItem(
      QString::number(print.back().spectralCentroid)));  //+-2000
  ui->lw_notes->addItem(new QListWidgetItem(
      QString::number(print.back().spectralSpread)));  //+-300
  ui->lw_notes->addItem(new QListWidgetItem(
      QString::number(print.back().zeroCrossingRate)));  //+-0.01
  ui->lw_notes->addItem(new QListWidgetItem(
      QString::number(print.back().spectralBandwidth)));  //+-300
  ui->lw_notes->addItem(
      new QListWidgetItem(QString::number(print.back().pitch)));
  // writeToFile(frequencies, psd, notesCount, print);
  readFile("test.bin", frequencies, psd, notesCount, print);
}

void FFTGraph::writeToFile(const QVector<QVector<double>>& freq,
                           const QVector<QVector<double>>& psd,
                           const QMap<QString, int>& notes,
                           const std::vector<FFT::voicePrint>& print) {
  std::ofstream outFile("test.bin", std::ios::binary);
  if (!outFile.is_open()) {
    return;
  }
  size_t freqSize = freq.size();
  outFile.write(reinterpret_cast<const char*>(&freqSize), sizeof(freqSize));
  for (const auto& innerVec : freq) {
    size_t innerSize = innerVec.size();
    outFile.write(reinterpret_cast<const char*>(&innerSize), sizeof(innerSize));
    outFile.write(reinterpret_cast<const char*>(innerVec.data()),
                  innerSize * sizeof(double));
  }
  size_t psdSize = psd.size();
  outFile.write(reinterpret_cast<const char*>(&psdSize), sizeof(psdSize));
  for (const auto& innerVec : psd) {
    size_t innerSize = innerVec.size();
    outFile.write(reinterpret_cast<const char*>(&innerSize), sizeof(innerSize));
    outFile.write(reinterpret_cast<const char*>(innerVec.data()),
                  innerSize * sizeof(double));
  }
  size_t notesSize = notes.size();
  outFile.write(reinterpret_cast<const char*>(&notesSize), sizeof(notesSize));

  for (int i = 0; i < notes.size(); ++i) {
    size_t keySize = (notes.begin() + i).key().size();
    outFile.write(reinterpret_cast<const char*>(&keySize), sizeof(keySize));
    outFile.write((notes.begin() + i).key().toStdString().c_str(), keySize);
    outFile.write(reinterpret_cast<const char*>(&(notes.begin() + i).value()),
                  sizeof((notes.begin() + i).value()));
  }
  size_t printSize = print.size();
  outFile.write(reinterpret_cast<const char*>(&printSize), sizeof(printSize));
  for (const auto& voice : print) {
    outFile.write(reinterpret_cast<const char*>(&voice),
                  sizeof(FFT::voicePrint));
  }

  outFile.close();
}

void FFTGraph::readFile(const std::string& name,
                        const QVector<QVector<double>>& freq,
                        const QVector<QVector<double>>& psd,
                        const QMap<QString, int>& notes,
                        const std::vector<FFT::voicePrint>& print) {
  std::ifstream inFile(name, std::ios::binary);
  if (!inFile.is_open()) {
    return;
  }
  size_t freqSize;
  inFile.read(reinterpret_cast<char*>(&freqSize), sizeof(freqSize));
  QVector<QVector<double>> vFreq(freqSize);
  for (size_t i = 0; i < freqSize; ++i) {
    size_t innerSize;
    inFile.read(reinterpret_cast<char*>(&innerSize), sizeof(innerSize));
    vFreq[i].resize(innerSize);
    inFile.read(reinterpret_cast<char*>(vFreq[i].data()),
                innerSize * sizeof(double));
  }
  size_t psdSize;
  inFile.read(reinterpret_cast<char*>(&psdSize), sizeof(psdSize));
  QVector<QVector<double>> vPsd(psdSize);
  for (size_t i = 0; i < psdSize; ++i) {
    size_t innerSize;
    inFile.read(reinterpret_cast<char*>(&innerSize), sizeof(innerSize));
    vPsd[i].resize(innerSize);
    inFile.read(reinterpret_cast<char*>(vPsd[i].data()),
                innerSize * sizeof(double));
  }
  QMap<QString, int> mNotes;
  size_t notesSize;
  inFile.read(reinterpret_cast<char*>(&notesSize), sizeof(notesSize));
  for (size_t i = 0; i < notesSize; ++i) {
    size_t keySize;
    inFile.read(reinterpret_cast<char*>(&keySize), sizeof(keySize));
    std::string keyStr(keySize, '\0');
    inFile.read(&keyStr[0], keySize);
    QString key = QString::fromStdString(keyStr);
    int value;
    inFile.read(reinterpret_cast<char*>(&value), sizeof(value));
    mNotes.insert(key, value);
  }
  size_t printSize;
  inFile.read(reinterpret_cast<char*>(&printSize), sizeof(printSize));
  std::vector<FFT::voicePrint> sPrint(printSize);
  for (size_t i = 0; i < printSize; ++i) {
    inFile.read(reinterpret_cast<char*>(&sPrint[i]), sizeof(FFT::voicePrint));
  }
  inFile.close();

  double fPer = compareVectors(freq, vFreq);
  double pPer = compareVectors(psd, vPsd);
  double mPer = compareMaps(notes, mNotes);
  double prPer = calculatePercentageDifference(print, sPrint);
  if (fPer + pPer + mPer + prPer < 0) return;
  const int MAX_DIFFERENCE = 18;
  if ((fPer + pPer + mPer + prPer) / 4 > MAX_DIFFERENCE) return;
  QMessageBox::critical(this, "Done", "LoggedIn");
}

double FFTGraph::compareVectors(const QVector<QVector<double>>& vec1,
                                const QVector<QVector<double>>& vec2) {
  if (vec1.size() != vec2.size() || vec1.isEmpty()) {
    qWarning() << "Vectors have different sizes or are empty";
    return -1.0;
  }
  int totalElements = 0;
  double totalDifference = 0.0;
  for (int i = 0; i < vec1.size(); ++i) {
    int size = std::min(vec1[i].size(), vec2[i].size());

    for (int j = 0; j < size; ++j) {
      totalDifference += qAbs(vec1[i][j] - vec2[i][j]);
      ++totalElements;
    }
  }
  if (totalElements == 0) {
    qWarning() << "No elements found in the vectors";
    return -1.0;
  }
  double averageDifference = totalDifference / totalElements;
  return (averageDifference / totalElements) * 100.0;
}

double FFTGraph::compareMaps(const QMap<QString, int>& map1,
                             const QMap<QString, int>& map2) {
  QStringList keyList2;
  for (auto it = map2.begin(); it != map2.end(); ++it) {
    keyList2.append(it.key());
  }
  int diff = 0;
  for (auto& key : keyList2) {
    if (map1.contains(key)) {
      int value2 = map2.value(key);
      int value1 = map1.value(key);
      if (value2 > value1 + 0.1 * value1 || value2 < value1 - 0.1 * value1) {
        diff++;
      }
    } else {
      diff++;
    }
  }
  double percentageDifference =
      (static_cast<double>(diff) / keyList2.size()) * 100.0;
  return percentageDifference;
}

double FFTGraph::calculatePercentageDifference(
    const std::vector<FFT::voicePrint>& vp1,
    const std::vector<FFT::voicePrint>& vp2) {
  double resPers = .0;
  for (int i = 0; i < vp1.size(); ++i) {
    const double epsilon = 1e-6;
    double percentageDifference = 0.0;
    percentageDifference +=
        std::abs(vp1[i].averageAmplitude - vp2[i].averageAmplitude) /
        std::max(epsilon, std::abs(vp2[i].averageAmplitude)) * 100;
    percentageDifference +=
        std::abs(vp1[i].spectralCentroid - vp2[i].spectralCentroid) /
        std::max(epsilon, std::abs(vp2[i].spectralCentroid)) * 100;
    percentageDifference +=
        std::abs(vp1[i].spectralSpread - vp2[i].spectralSpread) /
        std::max(epsilon, std::abs(vp2[i].spectralSpread)) * 100;
    percentageDifference +=
        std::abs(vp1[i].zeroCrossingRate - vp2[i].zeroCrossingRate) /
        std::max(epsilon, std::abs(vp2[i].zeroCrossingRate)) * 100;
    percentageDifference +=
        std::abs(vp1[i].spectralBandwidth - vp2[i].spectralBandwidth) /
        std::max(epsilon, std::abs(vp2[i].spectralBandwidth)) * 100;
    percentageDifference += std::abs(vp1[i].pitch - vp2[i].pitch) /
                            std::max(epsilon, std::abs(vp2[i].pitch)) * 100;
    percentageDifference +=
        std::abs(vp1[i].harmonicNoise - vp2[i].harmonicNoise) /
        std::max(epsilon, std::abs(vp2[i].harmonicNoise)) * 100;
    percentageDifference /= 7.0;
    if (percentageDifference > resPers) resPers = percentageDifference;
  }
  return resPers;
}

void FFTGraph::drawFFT() {
  if (!m_waveFile) return;
  const std::size_t audioWaveLength = m_waveFile->getMaxFrameIndex() *
                                      m_waveFile->getNumChannels() *
                                      sizeof(SAMPLE);
  double* audioWave = new double[audioWaveLength];
  FFT::int16ToDouble(m_waveFile->getData(), audioWaveLength, audioWave);
  drawPSD(audioWave, audioWaveLength,
          detectRegions(m_waveFile->getData(), audioWaveLength, 400));
  delete[] audioWave;
}
