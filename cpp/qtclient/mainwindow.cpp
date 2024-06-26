/* -*- c++ -*- */
/*
 * Gqrx SDR: Software defined radio receiver powered by GNU Radio and Qt
 *           https://gqrx.dk/
 *
 * Copyright 2011-2014 Alexandru Csete OZ9AEC.
 * Copyright (C) 2013 by Elias Oenal <EliasOenal@gmail.com>
 *
 * Gqrx is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * Gqrx is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Gqrx; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */
#include <string>
#include <vector>
#include <volk/volk.h>

#include <QByteArray>
#include <QDateTime>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QFile>
#include <QGroupBox>
#include <QKeySequence>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QResource>
#include <QShortcut>
#include <QString>
#include <QSvgWidget>
#include <QTextBrowser>
#include <QTextCursor>
#include <QTextStream>
#include <QTimer>
#include <QVBoxLayout>
#include <QtGlobal>

#include "grpc/grpc_async_receiver.h"
#include "ioconfig.h"
#include "mainwindow.h"
#include "plotter/fftaverager.h"
#include "plotter/vfographicsitem.h"
#include "receiver_model.h"
#include "vfoopt.h"
#include "vfosopt.h"

/* Qt Designer files */
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    d_frameRequested(false),
    d_fftAvg(0.25),
    d_fftNormalizeEnergy(false),
    d_have_audio(true),
    fftAverager(new FftAverager(this))
{
    ui->setupUi(this);

    ui->splitter->setSizes({1000000000, 1000000000});

    /* Initialise default configuration directory */
    QByteArray xdg_dir = qgetenv("XDG_CONFIG_HOME");
    if (xdg_dir.isEmpty()) {
        // Qt takes care of conversion to native separators
        m_cfg_dir = QString("%1/.config/gqrx").arg(QDir::homePath());
    } else {
        m_cfg_dir = QString("%1/gqrx").arg(xdg_dir.data());
    }

    setWindowTitle("VioletRX");

    /* frequency control widget */
    ui->freqCtrl->setup(0, 0, 9999e6, 1, FCTL_UNIT_NONE);
    ui->freqCtrl->setFrequency(144500000);

    /* create receiver object */
    rxModel = new ReceiverModel(
        violetrx::GrpcAsyncReceiver::make("0.0.0.0:50050"), this);
    connectReceiver();

    /* FFT timer & data */
    iq_fft_timer = new QTimer(this);
    iq_fft_timer->setTimerType(Qt::PreciseTimer);
    connect(iq_fft_timer, SIGNAL(timeout()), this, SLOT(iqFftTimeout()));
    d_last_fft_ms = 0;
    d_avg_fft_rate = 0.0;
    d_frame_drop = false;

    /* create dock widgets */
    uiDockVfosOpt = new QDockWidget("VFOs", this);
    uiDockVfosOpt->setObjectName("DockVFOs");
    uiVfosOpt = new VfosOpt(rxModel);
    uiDockVfosOpt->setWidget(uiVfosOpt);

    uiDockInputCtl = new DockInputCtl(rxModel, this);
    uiDockFft = new DockFft(rxModel, this);

    // setup some toggle view shortcuts
    uiDockInputCtl->toggleViewAction()->setShortcut(
        QKeySequence(Qt::CTRL | Qt::Key_J));
    uiDockVfosOpt->toggleViewAction()->setShortcut(
        QKeySequence(Qt::CTRL | Qt::Key_R));
    uiDockFft->toggleViewAction()->setShortcut(
        QKeySequence(Qt::CTRL | Qt::Key_F));
    ui->mainToolBar->toggleViewAction()->setShortcut(
        QKeySequence(Qt::CTRL | Qt::Key_T));

    /* frequency setting shortcut */
    auto* freq_shortcut = new QShortcut(QKeySequence(Qt::Key_F), this);
    QObject::connect(freq_shortcut, &QShortcut::activated, this,
                     &MainWindow::frequencyFocusShortcut);

    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::BottomDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    /* Add dock widgets to main window. This should be done even for
       dock widgets that are going to be hidden, otherwise they will
       end up floating in their own top-level window and can not be
       docked to the mainwindow.
    */
    addDockWidget(Qt::RightDockWidgetArea, uiDockInputCtl);
    addDockWidget(Qt::RightDockWidgetArea, uiDockVfosOpt);
    addDockWidget(Qt::RightDockWidgetArea, uiDockFft);
    tabifyDockWidget(uiDockInputCtl, uiDockVfosOpt);
    tabifyDockWidget(uiDockVfosOpt, uiDockFft);
    uiDockVfosOpt->raise();

    /* Add dock widget actions to View menu. By doing it this way all
       signal/slot connections will be established automagially.
    */
    ui->menu_View->addAction(uiDockInputCtl->toggleViewAction());
    ui->menu_View->addAction(uiDockVfosOpt->toggleViewAction());
    ui->menu_View->addAction(uiDockFft->toggleViewAction());
    ui->menu_View->addSeparator();
    ui->menu_View->addAction(ui->mainToolBar->toggleViewAction());
    ui->menu_View->addSeparator();
    ui->menu_View->addAction(ui->actionFullScreen);

    /* connect signals and slots */
    connect(ui->freqCtrl, SIGNAL(requestNewFrequency(qint64)), this,
            SLOT(setNewFrequency(qint64)));
    connect(uiDockInputCtl, SIGNAL(ignoreLimitsChanged(bool)), this,
            SLOT(setIgnoreLimits(bool)));

    // FFT Dock
    connect(uiDockFft, SIGNAL(fftSizeChanged(int)), this,
            SLOT(setIqFftSize(int)));
    connect(uiDockFft, SIGNAL(fftRateChanged(int)), this,
            SLOT(setIqFftRate(int)));
    connect(uiDockFft, SIGNAL(fftWindowChanged(int)), this,
            SLOT(setIqFftWindow(int)));
    connect(uiDockFft, SIGNAL(wfSpanChanged(quint64)), this,
            SLOT(setWfTimeSpan(quint64)));
    connect(uiDockFft, SIGNAL(fftAvgChanged(float)), fftAverager,
            SLOT(setFftAvg(float)));
    connect(uiDockFft, SIGNAL(fftZoomChanged(float)), ui->plotter,
            SLOT(zoomOnXAxis(float)));
    connect(uiDockFft, SIGNAL(plotScaleChanged(int, bool)), fftAverager,
            SLOT(setPlotScale(int, bool)));
    connect(uiDockFft, SIGNAL(plotScaleChanged(int, bool)), this,
            SLOT(plotScaleChanged(int, bool)));
    connect(uiDockFft, SIGNAL(resetFftZoom()), ui->plotter,
            SLOT(resetHorizontalZoom()));
    connect(uiDockFft, SIGNAL(gotoFftCenter()), ui->plotter,
            SLOT(moveToCenterFreq()));
    connect(uiDockFft, SIGNAL(wfColormapChanged(const QString)), ui->waterfall,
            SLOT(setColormapFromName(const QString)));
    connect(uiDockFft, SIGNAL(pandapterRangeChanged(float, float)), ui->plotter,
            SLOT(setDbSpan(float, float)));
    connect(uiDockFft, SIGNAL(waterfallRangeChanged(float, float)),
            ui->waterfall, SLOT(setPowerRange(float, float)));
    connect(uiDockFft, SIGNAL(fftColorChanged(QColor)), this,
            SLOT(setFftColor(QColor)));
    connect(uiDockFft, SIGNAL(fftFillToggled(bool)), this,
            SLOT(enableFftFill(bool)));
    connect(uiDockFft, SIGNAL(fftMaxHoldToggled(bool)), ui->plotter,
            SLOT(enableMaxHold(bool)));
    connect(uiDockFft, SIGNAL(fftMinHoldToggled(bool)), ui->plotter,
            SLOT(enableMinHold(bool)));

    // Plotter
    connect(ui->plotter, SIGNAL(zoomLevelChanged(float)), uiDockFft,
            SLOT(setZoomLevel(float)));
    connect(ui->plotter, SIGNAL(viewportChanged(const QRectF&)), this,
            SLOT(onPlotterViewportChanged(const QRectF&)));
    connect(ui->plotter, SIGNAL(gridMouseRightButtonClicked(QPointF, QPoint)),
            this, SLOT(onPlotterRightClicked(QPointF, QPoint)));
    connect(ui->plotter,
            SIGNAL(gridMouseLeftButtonDoubleClicked(QPointF, QPoint)), this,
            SLOT(onPlotterDoubleClicked(QPointF, QPoint)));
    connect(ui->waterfall, SIGNAL(sizeChanged(const QSize&)), this,
            SLOT(setWfSize()));

    // Create list of input devices. This must be done before the configuration
    // is restored because device probing might change the device configuration
    CIoConfig::getDeviceList(devList);

    rxModel->subscribe();
}

MainWindow::~MainWindow()
{
    on_actionDSP_triggered(false);

    iq_fft_timer->stop();
    delete iq_fft_timer;
    delete ui;
    delete uiDockVfosOpt;
    delete uiDockFft;
    delete uiDockInputCtl;
    delete fftAverager;
}

/**
 * @brief Update hardware RF frequency range.
 * @param ignore_limits Whether ignore the hardware specd and allow DC-to-light
 *                      range.
 *
 * This function fetches the frequency range of the receiver. Useful when we
 * read a new configuration with a new input device or when the ignore_limits
 * setting is changed.
 */
void MainWindow::updateHWFrequencyRange(bool /* ignore_limits */) {}

/**
 * @brief Update available frequency range.
 *
 * This function sets the available frequency range based on the hardware
 * frequency range, the selected filter offset and the LNB LO.
 *
 * This function must therefore be called whenever the LNB LO or the filter
 * offset has changed.
 */
void MainWindow::updateFrequencyRange() {}

/**
 * @brief Slot for receiving frequency change signals.
 * @param[in] freq The new frequency.
 *
 * This slot is connected to the CFreqCtrl::newFrequency() signal and is used
 * to set new receive frequency.
 */
void MainWindow::setNewFrequency(qint64 rx_freq)
{
    rxModel->setCenterFreq(rx_freq);
}

void MainWindow::onCenterFreqChanged(qint64 center_freq)
{
    // update widgets
    ui->plotter->setCenterFreq(center_freq);
    ui->waterfall->setCenterFreq(center_freq);
    ui->freqCtrl->setFrequency(center_freq);
}

/**
 * @brief Ignore hardware limits.
 * @param ignore_limits Whether hardware limits should be ignored or not.
 *
 * This slot is triggered when the user changes the "Ignore hardware limits"
 * option. It will update the allowed frequency range and also update the
 * current RF center frequency, which may change when we switch from ignore to
 * don't ignore.
 */
void MainWindow::setIgnoreLimits(bool /* ignore_limits */)
{
    // TODO
}

/** Baseband FFT plot timeout. */
void MainWindow::iqFftTimeout()
{
    if (d_frameRequested)
        return;

    // Track the frame rate and warn if not keeping up. Since the interval is
    // ms, the timer can not be set exactly to all rates.
    const quint64 now_ms = QDateTime::currentMSecsSinceEpoch();
    const float expected_rate = 1000.0f / (float)iq_fft_timer->interval();
    const float last_fft_rate = 1000.0f / (float)(now_ms - d_last_fft_ms);
    const float alpha = std::pow(expected_rate, -0.75f);
    if (d_avg_fft_rate == 0.0f) {
        d_avg_fft_rate = expected_rate;
    } else {
        d_avg_fft_rate =
            (1.0f - alpha) * d_avg_fft_rate + alpha * last_fft_rate;
    }

    const bool drop = d_avg_fft_rate < expected_rate * 0.95f;
    if (drop != d_frame_drop) {
        if (drop) {
            uiDockFft->setActualFrameRate(d_avg_fft_rate, true);
        } else {
            uiDockFft->setActualFrameRate(d_avg_fft_rate, false);
        }
        d_frame_drop = drop;
    }

    d_frameRequested = true;
    rxModel->getIqFftData(&d_iqFrame)
        .then(this, std::bind_front(&MainWindow::refreshFft, this))
        .onFailed(
            [](const std::exception& e) { spdlog::error("{}", e.what()); });
}

void MainWindow::refreshFft()
{
    d_frameRequested = false;

    d_last_fft_ms = QDateTime::currentMSecsSinceEpoch();

    const size_t fftsize = d_iqFrame.fft_points.size();
    if (fftsize == 0) {
        /* nothing to do, wait until next activation. */
        return;
    }

    fftAverager->step(d_iqFrame.fft_points.data(), fftsize);

    const std::vector<float>& iirData = fftAverager->iirData();
    const std::vector<float>& data = fftAverager->data();

    ui->plotter->setNewFftData(iirData.data(), iirData.size());
    ui->waterfall->setNewFftData(data.data(), data.size());
}

/** FFT size has changed. */
void MainWindow::setIqFftSize(int size)
{
    qDebug() << "Changing baseband FFT size to" << size;
    rxModel->setIqFftSize(size);
}

/** Baseband FFT rate has changed. */
void MainWindow::setIqFftRate(int fps)
{
    int interval;

    d_fps = fps;

    if (fps == 0) {
        interval = 36e7; // 100 hours
        ui->plotter->setRunningState(false);
    } else {
        interval = 1000 / fps;

        fftAverager->setFftRate(fps);
        if (iq_fft_timer->isActive())
            ui->plotter->setRunningState(true);
    }

    // Limit to 250 fps
    if (interval > 3 && iq_fft_timer->isActive())
        iq_fft_timer->setInterval(interval);

    uiDockFft->setWfResolution(ui->waterfall->getTimeResolution());

    // Invalidate average frame rate
    d_avg_fft_rate = 0.0;
}

void MainWindow::setIqFftWindow(int type)
{
    d_fftWindowType = type;
    rxModel->setIqFftWindow((WindowType)d_fftWindowType, d_fftNormalizeEnergy);
}

void MainWindow::plotScaleChanged(int type, bool perHz)
{
    // PLOT_SCALE_DBFS (0) always uses amplitude normalization.

    // PLOT_SCALE_DBV (1) requires energy normalization for /sqrt(Hz) (1), but
    // not for RBW (0).

    // PLOT_SCALE_DBM (2) requires energy normalization of FFT window whether
    // or not perHz is specified.

    d_fftNormalizeEnergy = (type == 2) || (type == 1 && perHz);
    rxModel->setIqFftWindow((WindowType)d_fftWindowType, d_fftNormalizeEnergy);
}

/** Waterfall time span has changed. */
void MainWindow::setWfTimeSpan(quint64 span_ms)
{
    // set new time span, then send back new resolution to be shown by GUI label
    ui->waterfall->setTimeSpan(span_ms);
    uiDockFft->setWfResolution(ui->waterfall->getTimeResolution());
}

void MainWindow::setWfSize()
{
    uiDockFft->setWfResolution(ui->waterfall->getTimeResolution());
}

/** Set FFT plot color. */
void MainWindow::setFftColor(const QColor& color)
{
    ui->plotter->setFftColor(color);

    QColor fillColor = color;
    fillColor.setAlpha(80);
    ui->plotter->setFftFillColor(fillColor);
}

/** Enable/disable filling the aread below the FFT plot. */
void MainWindow::enableFftFill(bool enable)
{
    ui->plotter->enableFftFill(enable);
}

/**
 * @brief Start/Stop DSP processing.
 * @param checked Flag indicating whether DSP processing should be ON or OFF.
 */
void MainWindow::on_actionDSP_triggered(bool checked)
{
    // reset actionDSP
    ui->actionDSP->setChecked(!checked);

    if (checked) {
        /* start receiver */
        rxModel->start();
    } else {
        /* stop receiver */
        rxModel->stop();
    }
}

/**
 * @brief Action: I/O device configurator triggered.
 *
 * This slot is activated when the user selects "I/O Devices" in the
 * menu. It activates the I/O configurator and if the user closes the
 * configurator using the OK button, the new configuration is read and
 * sent to the receiver.
 */
int MainWindow::on_actionIoConfig_triggered()
{
    qDebug() << "Configure I/O devices.";

    CIoConfig ioconf{devList};
    auto confres = ioconf.exec();

    if (confres == QDialog::Accepted) {
        rxModel->setInputDevice(ioconf.getInputDevice());
    }

    return confres;
}

void MainWindow::onPlotterViewportChanged(const QRectF& viewport)
{
    uiDockFft->setPandapterRange(viewport.top(), viewport.bottom());
    ui->waterfall->setFreqRange(viewport.left(), viewport.right());
}

/** Full screen button or menu item toggled. */
void MainWindow::on_actionFullScreen_triggered(bool checked)
{
    if (checked) {
        ui->statusBar->hide();
        showFullScreen();
    } else {
        ui->statusBar->show();
        showNormal();
    }
}

#define DATA_BUFFER_SIZE 48000

/**
 * Show kbd-shortcuts.txt in a dialog window.
 */
void MainWindow::on_actionKbdShortcuts_triggered()
{
    showSimpleTextFile(":/textfiles/kbd-shortcuts.txt",
                       tr("Keyboard shortcuts"));
}

/**
 * Show simple text file in a window.
 */
void MainWindow::showSimpleTextFile(const QString& resource_path,
                                    const QString& window_title)
{
    QResource resource(resource_path);
    QFile file(resource.absoluteFilePath());

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Unable to open file: " << file.fileName()
                 << " because of error " << file.errorString();

        return;
    }

    QTextStream in(&file);
    auto content = in.readAll();
    file.close();

    auto* browser = new QTextBrowser();
    browser->setLineWrapMode(QTextEdit::NoWrap);
    browser->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    browser->append(content);
    browser->adjustSize();

    // scroll to the beginning
    auto cursor = browser->textCursor();
    cursor.setPosition(0);
    browser->setTextCursor(cursor);

    auto* layout = new QVBoxLayout();
    layout->addWidget(browser);

    auto* dialog = new QDialog(this);
    dialog->setWindowTitle(window_title);
    dialog->setLayout(layout);
    dialog->resize(800, 400);
    dialog->exec();

    delete dialog;
    // browser and layout deleted automatically
}

void MainWindow::on_actionAbout_triggered()
{
    // TODO
}

/**
 * @brief Action: About gqrx.
 *
 * This slot is called when the user activates the
 * Help|About menu item (or Gqrx|About on Mac)
 */
void MainWindow::on_actionAboutGqrx_triggered()
{
    QMessageBox::about(
        this, tr("About Gqrx"),
        tr("<p>Gqrx</p>"
           "<p>Copyright (C) 2011-2023 Alexandru Csete & contributors.</p>"
           "<p>Gqrx is a software defined radio (SDR) receiver powered by "
           "<a href='https://www.gnuradio.org/'>GNU Radio</a> and the Qt "
           "toolkit. "
           "<p>Gqrx uses the <a "
           "href='https://osmocom.org/projects/gr-osmosdr/wiki/"
           "GrOsmoSDR'>GrOsmoSDR</a> "
           "input source block and works with any input device supported by "
           "it, including "
           "Funcube Dongle, RTL-SDR, Airspy, HackRF, RFSpace, BladeRF and USRP "
           "receivers."
           "</p>"
           "<p>You can download the latest version from the "
           "<a href='https://gqrx.dk/'>Gqrx website</a>."
           "</p>"
           "<p>"
           "Gqrx is licensed under the <a "
           "href='https://www.gnu.org/licenses/gpl-3.0.html'>GNU General "
           "Public License</a>."
           "</p>"));
}

/**
 * @brief Action: About Qt
 *
 * This slot is called when the user activates the
 * Help|About Qt menu item (or Gqrx|About Qt on Mac)
 */
void MainWindow::on_actionAboutQt_triggered()
{
    QMessageBox::aboutQt(this, tr("About Qt"));
}

void MainWindow::frequencyFocusShortcut() { ui->freqCtrl->setFrequencyFocus(); }

void MainWindow::connectReceiver()
{
    connect(rxModel, &ReceiverModel::started, this, &MainWindow::onStarted);
    connect(rxModel, &ReceiverModel::stopped, this, &MainWindow::onStopped);
    connect(rxModel, &ReceiverModel::inputDeviceChanged, this,
            &MainWindow::onInputDeviceChanged);
    connect(rxModel, &ReceiverModel::inputRateChanged, this,
            &MainWindow::onInputRateChanged);
    connect(rxModel, &ReceiverModel::centerFreqChanged, this,
            &MainWindow::onCenterFreqChanged);

    connect(rxModel, &ReceiverModel::vfoAdded, this, &MainWindow::onVfoAdded);
}

void MainWindow::onStarted()
{
    /* start GUI timers */
    if (uiDockFft->fftRate()) {
        iq_fft_timer->start(1000 / uiDockFft->fftRate());
        ui->plotter->setRunningState(true);
        fftAverager->reset();
    } else {
        iq_fft_timer->start(36e7); // 100 hours
        ui->plotter->setRunningState(false);
        fftAverager->reset();
    }

    /* update menu text and button tooltip */
    ui->actionDSP->setToolTip(tr("Stop DSP processing"));
    ui->actionDSP->setText(tr("Stop DSP"));

    ui->actionDSP->setChecked(true);
}

void MainWindow::onStopped()
{
    /* stop GUI timers */
    iq_fft_timer->stop();

    /* update menu text and button tooltip */
    ui->actionDSP->setToolTip(tr("Start DSP processing"));
    ui->actionDSP->setText(tr("Start DSP"));

    ui->plotter->setRunningState(false);

    ui->actionDSP->setChecked(false);
}

void MainWindow::onInputDeviceChanged(const QString& device)
{
    // Update window title
    setWindowTitle(QString("VioletRX - %2").arg(device));

    ui->freqCtrl->setFrequency(rxModel->centerFreq());
}

void MainWindow::onInputRateChanged(qint64 rate)
{
    uiDockFft->setSampleRate((float)rate);
    ui->plotter->setSampleRate((float)rate);
    ui->waterfall->setSampleRate((float)rate);
    fftAverager->setSampleRate((float)rate);
}

void MainWindow::onVfoAdded(VFOChannelModel* vfo)
{
    VFOGraphicsItem* vfoGi = ui->plotter->addVFO();

    QColor backgroundColor = vfo->color();
    backgroundColor.setAlpha(0x4F);
    vfoGi->setBackgroundColor(backgroundColor);

    connect(vfoGi, &VFOGraphicsItem::loHiRequestChange, this,
            [vfo](qint64 low, qint64 high) {
                vfo->setFilter(low, high, vfo->filter().shape);
            });

    connect(vfoGi, &VFOGraphicsItem::offsetRequestChange, this,
            [vfo](qint64 offset) { vfo->setFilterOffset(offset); });

    connect(vfoGi, &VFOGraphicsItem::mousePressed, this,
            [vfo] { vfo->setActive(true); });

    connect(vfo, &VFOChannelModel::offsetChanged, this,
            [vfoGi](qint64 offset) { vfoGi->setOffset(offset); });

    connect(vfo, &VFOChannelModel::filterChanged, this,
            [vfoGi](qint64 low, qint64 high, violetrx::FilterShape) {
                vfoGi->setHiLowCutFrequencies(low, high);
            });

    connect(vfo, &VFOChannelModel::filterRangeChanged, this,
            [vfoGi](FilterRange range) {
                vfoGi->setDemodRanges(range.lowMin, range.lowMax, range.highMin,
                                      range.highMax, range.symmetric);
            });

    connect(vfo, &VFOChannelModel::activeStatusChanged, this,
            [vfoGi](bool value) { vfoGi->setActive(value); });

    connect(vfo, &VFOChannelModel::colorChanged, this, [vfoGi](QColor color) {
        color.setAlpha(0x4F);
        vfoGi->setBackgroundColor(color);
    });

    connect(vfo, &VFOChannelModel::removed, this,
            [this, vfoGi]() { ui->plotter->removeVFO(vfoGi); });
}

void MainWindow::onPlotterRightClicked(QPointF, QPoint)
{
    ui->plotter->resetHorizontalZoom();
}
void MainWindow::onPlotterDoubleClicked(QPointF scenePos, QPoint)
{
    VFOChannelModel* activeVfo = rxModel->activeVfo();
    if (!activeVfo)
        return;

    activeVfo->setFilterOffset((qint64)scenePos.x() - rxModel->centerFreq());

    auto vfoGis = ui->plotter->getVfos();
    auto it = std::find_if(vfoGis.begin(), vfoGis.end(),
                           [](auto* v) { return v->isActive(); });
    if (it != vfoGis.end()) {
        auto vfoGi = *it;
        vfoGi->grabFocus();
    }
}
