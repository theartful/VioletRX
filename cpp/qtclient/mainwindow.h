/* -*- c++ -*- */
/*
 * Gqrx SDR: Software defined radio receiver powered by GNU Radio and Qt
 *           https://gqrx.dk/
 *
 * Copyright 2011-2014 Alexandru Csete OZ9AEC.
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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QColor>
#include <QFileDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QPointer>
#include <QSettings>
#include <QString>
#include <QSvgWidget>
#include <QTimer>

#include "qtclient/dockfft.h"
#include "qtclient/dockinputctl.h"

#include "qtclient/receiver_model.h"

namespace Ui
{
class MainWindow; /*! The main window UI */
}

class FftAverager;
class VFOGraphicsItem;
class VfosOpt;
class ReceiverModel;
class VFOChannelModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

public slots:
    /* Receiver */
    void onStarted();
    void onStopped();
    void onInputDeviceChanged(const QString& device);
    void onInputRateChanged(qint64 rate);
    void onVfoAdded(VFOChannelModel*);
    void onCenterFreqChanged(qint64 center_freq);

    /* Plotter */
    void onPlotterRightClicked(QPointF, QPoint);
    void onPlotterDoubleClicked(QPointF, QPoint);

public slots:
    void setNewFrequency(qint64 rx_freq);

private:
    Ui::MainWindow* ui;

    QString m_cfg_dir; /*!< Default config dir, e.g. XDG_CONFIG_HOME. */
    QString m_last_dir;

    FftFrame d_iqFrame;
    bool d_frameRequested;
    float d_fftAvg; /*!< FFT averaging parameter set by user (not the true
                       gain). */
    float d_fps;
    int d_fftWindowType;
    bool d_fftNormalizeEnergy;

    bool d_have_audio; /*!< Whether we have audio (i.e. not with demod_off. */

    /* dock widgets */
    VfosOpt* uiVfosOpt;
    QDockWidget* uiDockVfosOpt;

    DockInputCtl* uiDockInputCtl;
    DockFft* uiDockFft;

    /* data decoders */
    bool dec_rds{};

    QTimer* iq_fft_timer;
    quint64 d_last_fft_ms;
    float d_avg_fft_rate;
    bool d_frame_drop;

    ReceiverModel* rxModel;

    std::map<QString, QVariant> devList;
    FftAverager* fftAverager;

private:
    void updateHWFrequencyRange(bool ignore_limits);
    void updateFrequencyRange();
    void showSimpleTextFile(const QString& resource_path,
                            const QString& window_title);
    /* key shortcuts */
    void frequencyFocusShortcut();

    void connectReceiver();

private slots:
    /* baseband receiver */
    void setIgnoreLimits(bool ignore_limits);

    /* FFT settings */
    void setIqFftSize(int size);
    void setIqFftRate(int fps);
    void setIqFftWindow(int type);
    void plotScaleChanged(int type, bool perHz);
    void setFftColor(const QColor& color);
    void enableFftFill(bool enable);
    void setWfTimeSpan(quint64 span_ms);
    void setWfSize();

    /* FFT plot */
    void onPlotterViewportChanged(const QRectF&);

    /* menu and toolbar actions */
    void on_actionDSP_triggered(bool checked);
    int on_actionIoConfig_triggered();
    void on_actionFullScreen_triggered(bool checked);
    void on_actionKbdShortcuts_triggered();
    void on_actionAbout_triggered();
    void on_actionAboutQt_triggered();
    void on_actionAboutGqrx_triggered();

    /* cyclic processing */
    void iqFftTimeout();
    void refreshFft();
};

#endif // MAINWINDOW_H
