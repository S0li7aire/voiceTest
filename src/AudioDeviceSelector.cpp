#include "AudioDeviceSelector.h"

#include <fftw3.h>

#include <QDateTime>
#include <QtConcurrent/QtConcurrent>

#include "ui_AudioDeviceSelector.h"

std::vector<std::pair<int, int>> detectSoundRegions(const SAMPLE* audioData,
                                                    int length,
                                                    int noiseThreshold) {
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
  if (soundRegions.size() == 0) return soundRegions;
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

AudioDeviceSelector::AudioDeviceSelector(
    const std::vector<q::audio_device>& devices, QWidget* parent)
    : QWidget(parent),
      ui(new Ui::AudioDeviceSelector),
      m_audioDevices(devices) {
  ui->setupUi(this);
  initGraph();
  for (auto& device : m_audioDevices) {
    ui->cb_deviceList->addItem(QString::fromStdString(device.name()) + " " +
                               QString::number(device.id()));
  }

  connect(ui->pb_record, &QPushButton::pressed, this,
          &AudioDeviceSelector::listSelected);
  connect(ui->pb_save, &QPushButton::pressed, this, [this]() {
    if (m_recorder->writeToFile()) {
      emit waveFileDone(m_recorder->moveWaveFile());
      this->close();
    };
  });
}

AudioDeviceSelector::~AudioDeviceSelector() {
  delete m_recorder;
  delete ui;
}

void AudioDeviceSelector::initGraph() {
  if (customPlot) delete customPlot;
  customPlot = new QCustomPlot(this);
  customPlot->addGraph();
  customPlot->setMinimumSize(QSize(500, 500));
  customPlot->xAxis->setTicks(false);
  customPlot->yAxis->setTicks(false);
  customPlot->xAxis->setTickLabels(false);
  customPlot->yAxis->setTickLabels(false);
  customPlot->xAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
  customPlot->yAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
  QLinearGradient plotGradient;
  plotGradient.setStart(0, 0);
  plotGradient.setFinalStop(0, 350);
  plotGradient.setColorAt(0, QColor(80, 80, 80));
  plotGradient.setColorAt(1, QColor(50, 50, 50));
  customPlot->setBackground(plotGradient);
  ui->gl_graph->addWidget(customPlot);
}

void AudioDeviceSelector::listSelected() {
  m_deviceIndex = ui->cb_deviceList->currentIndex();
  if (!m_recorder)
    m_recorder = new audioRecorder(&m_audioDevices[m_deviceIndex]);
  else
    m_recorder->reset(&m_audioDevices[m_deviceIndex]);
  m_errorFlag = ErrorCodes::UNINITIALIZES;
  initGraph();
  (void)QtConcurrent::run([this]() {
    m_errorFlag = static_cast<ErrorCodes>(m_recorder->record());
  });
  drawLoop();
  int audioWaveLength = m_recorder->getMaxFrameIndex() *
                        m_recorder->getNumChannels() * sizeof(SAMPLE);
  std::vector<std::pair<int, int>> soundRegions =
      detectSoundRegions(m_recorder->getAudioSamples(), audioWaveLength, 400);
  for (std::pair<int, int> index : soundRegions) {
    QCPItemStraightLine* line = new QCPItemStraightLine(customPlot);
    line->setPen(QPen(Qt::red));
    line->point1->setTypeX(QCPItemPosition::ptPlotCoords);
    line->point1->setCoords(index.first, customPlot->yAxis->range().lower);
    line->point2->setTypeX(QCPItemPosition::ptPlotCoords);
    line->point2->setCoords(index.first, customPlot->yAxis->range().upper);
    line = new QCPItemStraightLine(customPlot);
    line->setPen(QPen(Qt::green));
    line->point1->setTypeX(QCPItemPosition::ptPlotCoords);
    line->point1->setCoords(index.second, customPlot->yAxis->range().lower);
    line->point2->setTypeX(QCPItemPosition::ptPlotCoords);
    line->point2->setCoords(index.second, customPlot->yAxis->range().upper);
  }
  customPlot->replot();
}

static inline float max(float a, float b) { return a > b ? a : b; }

void AudioDeviceSelector::drawLoop() {
  while (m_errorFlag == ErrorCodes::UNINITIALIZES) {
    drawGraph();
  }
  QString errorMessage;
  switch (m_errorFlag) {
    case ErrorCodes::NO_INPUT_CHANNELS:
      errorMessage = "0 input channels";
      break;
    case ErrorCodes::SERVICE_INIT_FAULT:
      errorMessage = "Error initializing audio service";
      break;
    case ErrorCodes::AUDIO_STREAM_OPENNING_FAULT:
      errorMessage = "Error openning audio stream";
      break;
    case ErrorCodes::AUDIO_STREAM_STARTING_FAULT:
      errorMessage = "Error starting audio stream";
      break;
    case ErrorCodes::AUDIO_STREAM_STOPPING_FAULT:
      errorMessage = "Error stopping audio stream";
      break;
    case ErrorCodes::AUDIO_STREAM_CLOSING_FAULT:
      errorMessage = "Error closing audio stream";
      break;
    case ErrorCodes::SERVICE_TERMINATE_FAULT:
      errorMessage = "Error termination audio service";
      break;
    default:
      return;
  }
  QMessageBox::critical(this, "Error", errorMessage);
  m_errorFlag = ErrorCodes::UNINITIALIZES;
}

void AudioDeviceSelector::drawGraph() {
  customPlot->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom));
  auto initGraph = [this](QVector<double> data, int index, int direction) {
    QVector<double> indexes(data.size());
    for (int i = 0; i < data.size(); ++i) {
      indexes[i] = i;
    }
    for (auto& item : data) {
      item *= direction;
    }
    QColor color(20 + 200 / 4.0 * 1, 70 * (1.6 - (index + 1.0) / 4.0), 150,
                 150);
    customPlot->graph(index)->setLineStyle(QCPGraph::lsLine);
    customPlot->graph(index)->setPen(QPen(color.lighter(200)));
    customPlot->graph(index)->setBrush(QBrush(color));
    customPlot->graph(index)->setData(indexes, data, true);
  };

  double max = .0;
  if (!m_recorder->getGraphData().isEmpty()) {
    QVector<double> tmpdata = m_recorder->getGraphData();
    max = *std::max_element(tmpdata.constBegin(), tmpdata.constEnd()) * 3;
  }

  initGraph(m_recorder->getGraphData(), 0, 1);
  customPlot->yAxis->setRange(-max, max);
  customPlot->graph(0)->rescaleAxes(true);
  customPlot->replot();
  QCoreApplication::processEvents();
}
