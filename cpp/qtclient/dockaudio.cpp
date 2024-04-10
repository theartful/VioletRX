/* -*- c++ -*- */
/*
 * Gqrx SDR: Software defined radio receiver powered by GNU Radio and Qt
 *           https://gqrx.dk/
 *
 * Copyright 2011-2016 Alexandru Csete OZ9AEC.
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
#include <cmath>
#include <QDebug>
#include <QDateTime>
#include <QShortcut>
#include <QDir>
#include "dockaudio.h"
#include "ui_dockaudio.h"

DockAudio::DockAudio(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::DockAudio),
    autoSpan(true),
    rx_freq(144000000)
{
    ui->setupUi(this);

    audioOptions = new CAudioOptions(this);

    connect(audioOptions, SIGNAL(newRecDirSelected(QString)), this, SLOT(setNewRecDir(QString)));
    connect(audioOptions, SIGNAL(newUdpHost(QString)), this, SLOT(setNewUdpHost(QString)));
    connect(audioOptions, SIGNAL(newUdpPort(int)), this, SLOT(setNewUdpPort(int)));
    connect(audioOptions, SIGNAL(newUdpStereo(bool)), this, SLOT(setNewUdpStereo(bool)));

    QShortcut *rec_toggle_shortcut = new QShortcut(QKeySequence(Qt::Key_R), this);
    QShortcut *mute_toggle_shortcut = new QShortcut(QKeySequence(Qt::Key_M), this);
    QShortcut *audio_gain_increase_shortcut1 = new QShortcut(QKeySequence(Qt::Key_Plus), this);
    QShortcut *audio_gain_decrease_shortcut1 = new QShortcut(QKeySequence(Qt::Key_Minus), this);

    QObject::connect(rec_toggle_shortcut, &QShortcut::activated, this, &DockAudio::recordToggleShortcut);
    QObject::connect(mute_toggle_shortcut, &QShortcut::activated, this, &DockAudio::muteToggleShortcut);
    QObject::connect(audio_gain_increase_shortcut1, &QShortcut::activated, this, &DockAudio::increaseAudioGainShortcut);
    QObject::connect(audio_gain_decrease_shortcut1, &QShortcut::activated, this, &DockAudio::decreaseAudioGainShortcut);
}

DockAudio::~DockAudio()
{
    delete ui;
}

/*! \brief Set new audio gain.
 *  \param gain the new audio gain in tens of dB (0 dB = 10)
 */
void DockAudio::setAudioGain(int gain)
{
    ui->audioGainSlider->setValue(gain);
}

/*! \brief Set new audio gain.
 *  \param gain the new audio gain in dB
 */
void DockAudio::setAudioGainDb(float gain)
{
    ui->audioGainSlider->setValue(int(std::round(gain*10.0f)));
}


/*! \brief Get current audio gain.
 *  \returns The current audio gain in tens of dB (0 dB = 10).
 */
int  DockAudio::audioGain()
{
    return ui->audioGainSlider->value();
}

/*! Public slot to trig audio recording by external events (e.g. satellite AOS).
 *
 * If a recording is already in progress we ignore the event.
 */
void DockAudio::startAudioRecorder(void)
{
    if (ui->audioRecButton->isChecked())
    {
        qDebug() << __func__ << "An audio recording is already in progress";
        return;
    }

    // emulate a button click
    ui->audioRecButton->click();
}

/*! Public slot to stop audio recording by external events (e.g. satellite LOS).
 *
 * The event is ignored if no audio recording is in progress.
 */
void DockAudio::stopAudioRecorder(void)
{
    if (ui->audioRecButton->isChecked())
        ui->audioRecButton->click(); // emulate a button click
    else
        qDebug() << __func__ << "No audio recording in progress";
}

/*! Public slot to set new RX frequency in Hz. */
void DockAudio::setRxFrequency(qint64 freq)
{
    rx_freq = freq;
}

/*! \brief Audio gain changed.
 *  \param value The new audio gain value in tens of dB (because slider uses int)
 */
void DockAudio::on_audioGainSlider_valueChanged(int value)
{
    float gain = float(value) / 10.0f;

    // update dB label
    ui->audioGainDbLabel->setText(QString("%1 dB").arg(gain, 5, 'f', 1));
    if (!ui->audioMuteButton->isChecked())
        emit audioGainChanged(gain);
}

/*! \brief Streaming button clicked.
 *  \param checked Whether streaming is ON or OFF.
 */
void DockAudio::on_audioStreamButton_clicked(bool checked)
{
    if (checked)
        emit audioStreamingStarted(udp_host, udp_port, udp_stereo);
    else
        emit audioStreamingStopped();
}

/*! \brief Record button clicked.
 *  \param checked Whether recording is ON or OFF.
 *
 * We use the clicked signal instead of the toggled which allows us to change the
 * state programmatically using toggle() without triggering the signal.
 */
void DockAudio::on_audioRecButton_clicked(bool checked)
{
    if (checked) {
        // FIXME: option to use local time
        // use toUTC() function compatible with older versions of Qt.
        QString file_name = QDateTime::currentDateTime().toUTC().toString("gqrx_yyyyMMdd_hhmmss");
        last_audio = QString("%1/%2_%3.wav").arg(rec_dir).arg(file_name).arg(rx_freq);
        QFileInfo info(last_audio);

        // emit signal and start timer
        emit audioRecStarted(last_audio);

        ui->audioRecLabel->setText(info.fileName());
        ui->audioRecButton->setToolTip(tr("Stop audio recorder"));
        ui->audioPlayButton->setEnabled(false); /* prevent playback while recording */
    }
    else {
        ui->audioRecLabel->setText("<i>DSP</i>");
        ui->audioRecButton->setToolTip(tr("Start audio recorder"));
        emit audioRecStopped();

        ui->audioPlayButton->setEnabled(true);
    }
}

/*! \brief Playback button clicked.
 *  \param checked Whether playback is ON or OFF.
 *
 * We use the clicked signal instead of the toggled which allows us to change the
 * state programmatically using toggle() without triggering the signal.
 */
void DockAudio::on_audioPlayButton_clicked(bool checked)
{
    if (checked) {
        QFileInfo info(last_audio);

        if(info.exists()) {
            // emit signal and start timer
            emit audioPlayStarted(last_audio);

            ui->audioRecLabel->setText(info.fileName());
            ui->audioPlayButton->setToolTip(tr("Stop audio playback"));
            ui->audioRecButton->setEnabled(false); // prevent recording while we play
        }
        else {
            ui->audioPlayButton->setChecked(false);
            ui->audioPlayButton->setEnabled(false);
        }
    }
    else {
        ui->audioRecLabel->setText("<i>DSP</i>");
        ui->audioPlayButton->setToolTip(tr("Start playback of last recorded audio file"));
        emit audioPlayStopped();

        ui->audioRecButton->setEnabled(true);
    }
}

/*! \brief Configure button clicked. */
void DockAudio::on_audioConfButton_clicked()
{
    audioOptions->show();
}

/*! \brief Mute audio. */
void DockAudio::on_audioMuteButton_clicked(bool checked)
{
    if (checked)
    {
        emit audioGainChanged(-INFINITY);
    }
    else
    {
        int value = ui->audioGainSlider->value();
        float gain = float(value) / 10.0f;
        emit audioGainChanged(gain);
    }
}

/*! \brief Set status of audio record button. */
void DockAudio::setAudioRecButtonState(bool checked)
{
    if (checked == ui->audioRecButton->isChecked()) {
        /* nothing to do */
        return;
    }

    // toggle the button and set the state of the other buttons accordingly
    ui->audioRecButton->toggle();
    bool isChecked = ui->audioRecButton->isChecked();

    ui->audioRecButton->setToolTip(isChecked ? tr("Stop audio recorder") : tr("Start audio recorder"));
    ui->audioPlayButton->setEnabled(!isChecked);
    //ui->audioRecConfButton->setEnabled(!isChecked);
}

/*! \brief Set status of audio record button. */
void DockAudio::setAudioPlayButtonState(bool checked)
{
    if (checked == ui->audioPlayButton->isChecked()) {
        // nothing to do
        return;
    }

    // toggle the button and set the state of the other buttons accordingly
    ui->audioPlayButton->toggle();
    bool isChecked = ui->audioPlayButton->isChecked();

    ui->audioPlayButton->setToolTip(isChecked ? tr("Stop audio playback") : tr("Start playback of last recorded audio file"));
    ui->audioRecButton->setEnabled(!isChecked);
    //ui->audioRecConfButton->setEnabled(!isChecked);
}

/*! \brief Slot called when a new valid recording directory has been selected
 *         in the audio conf dialog.
 */
void DockAudio::setNewRecDir(const QString &dir)
{
    rec_dir = dir;
}

/*! \brief Slot called when a new network host has been entered. */
void DockAudio::setNewUdpHost(const QString &host)
{
    if (host.isEmpty())
        udp_host = "localhost";
    else
        udp_host = host;
}

/*! \brief Slot called when a new network port has been entered. */
void DockAudio::setNewUdpPort(int port)
{
    udp_port = port;
}

/*! \brief Slot called when the mono/stereo streaming setting changes. */
void DockAudio::setNewUdpStereo(bool enabled)
{
    udp_stereo = enabled;
}

void DockAudio::recordToggleShortcut() {
    ui->audioRecButton->click();
}

void DockAudio::muteToggleShortcut() {
    ui->audioMuteButton->click();
}

void DockAudio::increaseAudioGainShortcut() {
	ui->audioGainSlider->triggerAction(QSlider::SliderPageStepAdd);
}

void DockAudio::decreaseAudioGainShortcut() {
	ui->audioGainSlider->triggerAction(QSlider::SliderPageStepSub);
}
