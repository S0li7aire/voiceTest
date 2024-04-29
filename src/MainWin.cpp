#include "MainWin.h"

#include "AudioDeviceSelector.h"
#include "FFTGraph.h"
#include "ui_MainWin.h"

MainWin::MainWin(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWin) {
  ui->setupUi(this);
  AudioDeviceSelector* selector =
      new AudioDeviceSelector(q::audio_device::list(), this);
  FFTGraph* graph = new FFTGraph(this);
  ui->gl_main->addWidget(selector);
  connect(selector, &AudioDeviceSelector::waveFileDone, graph,
          &FFTGraph::setWaveFile);
  connect(selector, &AudioDeviceSelector::accepted, graph, &FFTGraph::drawFFT);
  connect(selector, &AudioDeviceSelector::accepted, this,
          [graph]() { graph->show(); });
  selector->setAttribute(Qt::WA_DeleteOnClose, true);
  selector->show();
  ui->gl_main->addWidget(graph);
  graph->setAttribute(Qt::WA_DeleteOnClose, true);
  graph->hide();
}

MainWin::~MainWin() { delete ui; }
