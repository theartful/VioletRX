/* -*- c++ -*- */
/*
 * Gqrx SDR: Software defined radio receiver powered by GNU Radio and Qt
 *           https://gqrx.dk/
 *
 * Copyright 2011-2013 Alexandru Csete OZ9AEC.
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
#include "demod_options.h"
#include "receiver_model.h"
#include "ui_demod_options.h"
#include <QDebug>

/* convert between deemphasis time constant and combo-index */
const double tau_tbl[] = {0.0,      25.0e-6,  50.0e-6,  75.0e-6,
                          100.0e-6, 250.0e-6, 530.0e-6, 1.0e-3};
const int tau_tbl_maxidx = 7;

double tau_from_index(int index)
{
    if (index < 0 || index > tau_tbl_maxidx)
        return 0.0;

    return tau_tbl[index];
}

int tau_to_index(double tau)
{
    int i;
    for (i = 0; i < tau_tbl_maxidx; i++) {
        if (tau < (tau_tbl[i] + tau_tbl[i + 1]) / 2)
            return i;
    }
    return tau_tbl_maxidx;
}

/* convert betweenFM max dev and combo index */
float maxdev_from_index(int index)
{
    switch (index) {
    case 0:
        /* Voice 2.5k */
        return 2500.0;
    case 1:
        /* Voice 5k */
        return 5000.0;
    case 2:
        /* APT 17k */
        return 17000.0;
    case 3:
        /* APT 25k (17k but need some margin for Doppler and freq error) */
        return 25000.0;
    default:
        /* Voice 5k */
        qDebug() << "Invalid max_dev index: " << index;
        return 5000.0;
    }
}

int maxdev_to_index(float max_dev)
{
    if (max_dev < 3000.0f)
        /* Voice 2.5k */
        return 0;
    else if (max_dev < 10000.0f)
        /* Voice 5k */
        return 1;
    else if (max_dev < 20000.0f)
        /* APT 17k */
        return 2;
    else
        /* APT 25k */
        return 3;
}

/* convert between synchronous AM PLL bandwidth and combo index */
static float pll_bw_from_index(int index)
{
    switch (index) {
    case 0:
        /* Fast */
        return 0.01;
    case 1:
        /* Medium */
        return 0.001;
    case 2:
        /* Slow */
        return 0.0001;
    default:
        qDebug() << "Invalid AM-Sync PLL BW index: " << index;
        return 0.001;
    }
}

static int pll_bw_to_index(float pll_bw)
{
    if (pll_bw < 0.00015f)
        /* Slow */
        return 2;
    else if (pll_bw < 0.0015f)
        /* Medium */
        return 1;
    else if (pll_bw < 0.015f)
        /* Fast */
        return 0;
    else
        /* Medium */
        return 1;
}

CDemodOptions::CDemodOptions(VFOChannelModel* vfo_, QWidget* parent) :
    QDialog(parent), ui(new Ui::CDemodOptions), vfo(vfo_)
{
    ui->setupUi(this);

    connect(vfo, &VFOChannelModel::demodChanged, this,
            &CDemodOptions::onDemodChanged);
    connect(vfo, &VFOChannelModel::cwOffsetChanged, this,
            &CDemodOptions::onCwOffsetChanged);
    connect(vfo, &VFOChannelModel::fmDeemphChanged, this,
            &CDemodOptions::onFmDeemphChanged);
    connect(vfo, &VFOChannelModel::fmMaxDevChanged, this,
            &CDemodOptions::onFmMaxDevChanged);
    connect(vfo, &VFOChannelModel::amDcrChanged, this,
            &CDemodOptions::onAmDcrChanged);
    connect(vfo, &VFOChannelModel::amSyncPllBwChanged, this,
            &CDemodOptions::onPllBwChanged);
    connect(vfo, &VFOChannelModel::amDcrChanged, this,
            &CDemodOptions::onAmDcrChanged);
    connect(vfo, &VFOChannelModel::amSyncDcrChanged, this,
            &CDemodOptions::onAmSyncDcrChanged);

    refreshMaxdevSelector();
    refreshEmphSelector();
    refreshDcrCheckBox();
    refreshCwOffsetSpin();
    refreshSyncDcrCheckBox();
    refreshPllBwSelector();
}

CDemodOptions::~CDemodOptions() { delete ui; }

/*! \brief Catch window close events.
 *
 * This method is called when the user closes the demod options dialog
 * window using the window close icon. We catch the event and hide the
 * dialog but keep it around for later use.
 */
void CDemodOptions::closeEvent(QCloseEvent* event)
{
    hide();
    event->ignore();
}

void CDemodOptions::setCurrentPage(int index)
{
    if (index < PAGE_NUM)
        ui->demodOptions->setCurrentIndex(index);
}

int CDemodOptions::currentPage() const
{
    return ui->demodOptions->currentIndex();
}

void CDemodOptions::refreshMaxdevSelector()
{
    ui->maxdevSelector->blockSignals(true);
    ui->maxdevSelector->setCurrentIndex(maxdev_to_index(vfo->fmMaxDev()));
    ui->maxdevSelector->blockSignals(false);
}

void CDemodOptions::refreshEmphSelector()
{
    ui->emphSelector->blockSignals(true);
    ui->emphSelector->setCurrentIndex(tau_to_index(vfo->fmDeemph()));
    ui->emphSelector->blockSignals(false);
}

void CDemodOptions::refreshDcrCheckBox()
{
    ui->dcrCheckBox->blockSignals(true);
    ui->dcrCheckBox->setChecked(vfo->amDcr());
    ui->dcrCheckBox->blockSignals(false);
}

void CDemodOptions::refreshCwOffsetSpin()
{
    ui->cwOffsetSpin->blockSignals(true);
    ui->cwOffsetSpin->setValue(vfo->cwOffset());
    ui->cwOffsetSpin->blockSignals(false);
}

void CDemodOptions::refreshSyncDcrCheckBox()
{
    ui->syncdcrCheckBox->blockSignals(true);
    ui->syncdcrCheckBox->setChecked(vfo->amSyncDcr());
    ui->syncdcrCheckBox->blockSignals(false);
}

void CDemodOptions::refreshPllBwSelector()
{
    ui->pllBwSelector->blockSignals(true);
    ui->pllBwSelector->setCurrentIndex(pll_bw_to_index(vfo->amSyncPllBw()));
    ui->pllBwSelector->blockSignals(false);
}

void CDemodOptions::on_maxdevSelector_currentIndexChanged(int index)
{
    refreshMaxdevSelector();
    vfo->setFmMaxDev(maxdev_from_index(index));
}

void CDemodOptions::on_emphSelector_currentIndexChanged(int index)
{
    refreshEmphSelector();
    vfo->setFmDeemph(tau_from_index(index));
}

void CDemodOptions::on_dcrCheckBox_clicked(bool checked)
{
    refreshDcrCheckBox();
    vfo->setAmDcr(checked);
}

void CDemodOptions::on_cwOffsetSpin_valueChanged(int value)
{
    refreshCwOffsetSpin();
    vfo->setCwOffset(value);
}

void CDemodOptions::on_syncdcrCheckBox_clicked(bool checked)
{
    refreshSyncDcrCheckBox();
    vfo->setAmSyncDcr(checked);
}

void CDemodOptions::on_pllBwSelector_currentIndexChanged(int index)
{
    refreshPllBwSelector();
    vfo->setAmSyncPllBw(pll_bw_from_index(index));
}

void CDemodOptions::onDemodChanged()
{
    switch (vfo->demod()) {
    case Demod::NFM:
        setCurrentPage(CDemodOptions::PAGE_FM_OPT);
        break;
    case Demod::AM:
        setCurrentPage(CDemodOptions::PAGE_AM_OPT);
        break;
    case Demod::CWL:
    case Demod::CWU:
        setCurrentPage(CDemodOptions::PAGE_CW_OPT);
        break;
    case Demod::AM_SYNC:
        setCurrentPage(CDemodOptions::PAGE_AMSYNC_OPT);
        break;
    default:
        setCurrentPage(CDemodOptions::PAGE_NO_OPT);
    }
}

void CDemodOptions::onCwOffsetChanged() { refreshCwOffsetSpin(); }
void CDemodOptions::onFmDeemphChanged() { refreshEmphSelector(); }
void CDemodOptions::onFmMaxDevChanged() { refreshMaxdevSelector(); }
void CDemodOptions::onAmDcrChanged() { refreshDcrCheckBox(); }
void CDemodOptions::onPllBwChanged() { refreshPllBwSelector(); }
void CDemodOptions::onAmSyncDcrChanged() { refreshSyncDcrCheckBox(); }
