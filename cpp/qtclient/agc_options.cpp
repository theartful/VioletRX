/* -*- c++ -*- */
/*
 * Gqrx SDR: Software defined radio receiver powered by GNU Radio and Qt
 *           https://gqrx.dk/
 *
 * Copyright 2012-2013 Alexandru Csete OZ9AEC.
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
#include "agc_options.h"
#include "receiver_model.h"
#include "ui_agc_options.h"

#include <QDebug>
#include <QString>
#include <QTimer>

CAgcOptions::CAgcOptions(VFOChannelModel* vfo_, QWidget* parent) :
    QDialog(parent), ui(new Ui::CAgcOptions), vfo(vfo_)
{
    ui->setupUi(this);

    connect(vfo, &VFOChannelModel::agcOnChanged, this,
            &CAgcOptions::agcParamChanged);
    connect(vfo, &VFOChannelModel::agcSlopeChanged, this,
            &CAgcOptions::agcParamChanged);
    connect(vfo, &VFOChannelModel::agcDecayChanged, this,
            &CAgcOptions::agcParamChanged);
    connect(vfo, &VFOChannelModel::agcHangChanged, this,
            &CAgcOptions::agcParamChanged);
    connect(vfo, &VFOChannelModel::agcThresholdChanged, this,
            &CAgcOptions::agcParamChanged);
    connect(vfo, &VFOChannelModel::agcManualGainChanged, this,
            &CAgcOptions::agcParamChanged);

    agcParamChanged();
}

CAgcOptions::~CAgcOptions() { delete ui; }

void CAgcOptions::agcParamChanged()
{
    if (vfo->agcOn()) {
        ui->gainSlider->setDisabled(true);
        ui->gainLabel->setDisabled(true);
        ui->gainNameLabel->setDisabled(true);

        ui->thresholdSlider->setEnabled(true);
        ui->thresholdLabel->setEnabled(true);
        ui->thresholdNameLabel->setEnabled(true);

        ui->slopeSlider->setEnabled(true);
        ui->slopeLabel->setEnabled(true);
        ui->slopeNameLabel->setEnabled(true);

        ui->decaySlider->setEnabled(true);
        ui->decayLabel->setEnabled(true);
        ui->decayNameLabel->setEnabled(true);
    } else {
        ui->gainSlider->setEnabled(true);
        ui->gainLabel->setEnabled(true);
        ui->gainNameLabel->setEnabled(true);

        ui->thresholdSlider->setEnabled(true);
        ui->thresholdLabel->setEnabled(true);
        ui->thresholdNameLabel->setEnabled(true);

        ui->slopeSlider->setDisabled(true);
        ui->slopeLabel->setDisabled(true);
        ui->slopeNameLabel->setDisabled(true);

        ui->decaySlider->setDisabled(true);
        ui->decayLabel->setDisabled(true);
        ui->decayNameLabel->setDisabled(true);
    }

    ui->gainSlider->blockSignals(true);
    ui->gainSlider->setValue(vfo->agcManualGain());
    ui->gainSlider->blockSignals(false);

    ui->thresholdSlider->blockSignals(true);
    ui->thresholdSlider->setValue(vfo->agcThreshold());
    ui->thresholdSlider->blockSignals(false);

    ui->decaySlider->blockSignals(true);
    ui->decaySlider->setValue(vfo->agcDecay());
    ui->decaySlider->blockSignals(false);

    ui->slopeSlider->blockSignals(true);
    ui->slopeSlider->setValue(vfo->agcSlope());
    ui->slopeSlider->blockSignals(true);

    ui->gainLabel->setText(QString("%1 dB").arg(ui->gainSlider->value()));
    ui->thresholdLabel->setText(
        QString("%1 dB").arg(ui->thresholdSlider->value()));
    ui->decayLabel->setText(QString("%1 ms").arg(ui->decaySlider->value()));
    ui->slopeLabel->setText(QString("%1 dB").arg(ui->slopeSlider->value()));

    ui->hangButton->setText(vfo->agcHang() ? tr("Enabled") : tr("Disabled"));
    ui->hangButton->blockSignals(true);
    ui->hangButton->setChecked(vfo->agcHang());
    ui->hangButton->blockSignals(false);
}

/*! \brief Catch window close events.
 *
 * This method is called when the user closes the dialog window using the
 * window close icon. We catch the event and hide the dialog but keep it
 * around for later use.
 */
void CAgcOptions::closeEvent(QCloseEvent* event)
{
    hide();
    event->ignore();
}

/*! \brief AGC gain slider value has changed. */
void CAgcOptions::on_gainSlider_valueChanged(int gain)
{
    // reset old value
    ui->gainSlider->blockSignals(true);
    ui->gainSlider->setValue(vfo->agcManualGain());
    ui->gainSlider->blockSignals(false);

    vfo->setAgcManualGain(gain);
}

/*! \brief AGC threshold slider value has changed. */
void CAgcOptions::on_thresholdSlider_valueChanged(int threshold)
{
    // reset old value
    ui->thresholdSlider->blockSignals(true);
    ui->thresholdSlider->setValue(vfo->agcThreshold());
    ui->thresholdSlider->blockSignals(false);

    vfo->setAgcThreshold(threshold);
}

/*! \brief AGC slope slider value has changed. */
void CAgcOptions::on_slopeSlider_valueChanged(int slope)
{
    // reset old value
    ui->slopeSlider->blockSignals(true);
    ui->slopeSlider->setValue(vfo->agcSlope());
    ui->slopeSlider->blockSignals(false);

    vfo->setAgcSlope(slope);
}

/*! \brief AGC decay slider value has changed. */
void CAgcOptions::on_decaySlider_valueChanged(int decay)
{
    // reset old value
    ui->decaySlider->blockSignals(true);
    ui->decaySlider->setValue(vfo->agcDecay());
    ui->decaySlider->blockSignals(false);

    vfo->setAgcDecay(decay);
}

/*! \brief AGC hang button has been toggled. */
void CAgcOptions::on_hangButton_toggled(bool checked)
{
    ui->hangButton->blockSignals(true);
    ui->hangButton->setChecked(!checked);
    ui->hangButton->blockSignals(false);

    vfo->setAgcHang(checked);
}
