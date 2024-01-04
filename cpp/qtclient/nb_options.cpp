/* -*- c++ -*- */
/*
 * Gqrx SDR: Software defined radio receiver powered by GNU Radio and Qt
 *           https://gqrx.dk/
 *
 * Copyright 2013 Alexandru Csete OZ9AEC.
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
#include "nb_options.h"
#include "ui_nb_options.h"

#include "receiver_model.h"

CNbOptions::CNbOptions(VFOChannelModel* vfo_, QWidget* parent) :
    QDialog(parent), ui(new Ui::CNbOptions), vfo(vfo_)
{
    ui->setupUi(this);

    connect(vfo, &VFOChannelModel::noiseBlankerThresholdChanged, this,
            &CNbOptions::onThresholdChanged);

    onThresholdChanged();
}

CNbOptions::~CNbOptions() { delete ui; }

/*! \brief Catch window close events.
 *
 * This method is called when the user closes the dialog window using the
 * window close icon. We catch the event and hide the dialog but keep it
 * around for later use.
 */
void CNbOptions::closeEvent(QCloseEvent* event)
{
    hide();
    event->ignore();
}

void CNbOptions::on_nb1Threshold_valueChanged(double val)
{
    ui->nb1Threshold->blockSignals(true);
    ui->nb1Threshold->setValue(vfo->noiseBlanker1Threshold());
    ui->nb1Threshold->blockSignals(false);

    vfo->setNoiseBlankerThreshold(1, val);
}

void CNbOptions::on_nb2Threshold_valueChanged(double val)
{
    ui->nb2Threshold->blockSignals(true);
    ui->nb2Threshold->setValue(vfo->noiseBlanker2Threshold());
    ui->nb2Threshold->blockSignals(false);

    vfo->setNoiseBlankerThreshold(2, val);
}

void CNbOptions::onThresholdChanged()
{
    ui->nb1Threshold->blockSignals(true);
    ui->nb1Threshold->setValue(vfo->noiseBlanker1Threshold());
    ui->nb1Threshold->blockSignals(false);

    ui->nb2Threshold->blockSignals(true);
    ui->nb2Threshold->setValue(vfo->noiseBlanker2Threshold());
    ui->nb2Threshold->blockSignals(false);
}
