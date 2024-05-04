#include "AudioDeviceSelector.h"

#include <QDateTime>
#include <QtConcurrent/QtConcurrent>

#include "ui_AudioDeviceSelector.h"

AudioDeviceSelector::AudioDeviceSelector(
    const std::vector<q::audio_device> &devices, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::AudioDeviceSelector),
      m_audioDevices(devices) {
  ui->setupUi(this);
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

  for (auto &device : m_audioDevices) {
    ui->cb_deviceList->addItem(QString::fromStdString(device.name()));
  }

  connect(ui->cb_deviceList, &QComboBox::currentIndexChanged, this,
          &AudioDeviceSelector::listSelected);
  connect(ui->pb_save, &QPushButton::pressed, this, [this]() {
    if (m_recorder->writeToFile()) {
      emit waveFileDone(m_recorder->moveWaveFile());
      emit accepted();
      this->close();
    };
  });
}

AudioDeviceSelector::~AudioDeviceSelector() {
  delete m_recorder;
  delete ui;
}

void AudioDeviceSelector::listSelected(int index) {
  m_deviceIndex = index;
  m_recorder = new audioRecorder(&m_audioDevices[m_deviceIndex]);
  (void)QtConcurrent::run([this]() {
    if (m_recorder->record() != 1) {
      m_stopFlag = true;
    }
  });
  drawLoop();
}

static inline float max(float a, float b) { return a > b ? a : b; }

void AudioDeviceSelector::drawLoop() {
  while (!m_stopFlag) {
    drawGraph();
  }
}

void AudioDeviceSelector::drawGraph() {
  customPlot->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom));
  auto initGraph = [this](QVector<double> data, int index, int direction) {
    QVector<double> indexes(data.size());
    for (int i = 0; i < data.size(); ++i) {
      indexes[i] = i;
    }
    for (auto &item : data) {
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
