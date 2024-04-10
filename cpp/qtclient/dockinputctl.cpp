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
#include "dockinputctl.h"
#include "ui_dockinputctl.h"
#include <QDebug>
#include <qnamespace.h>

DockInputCtl::DockInputCtl(ReceiverModel* rxModel_, QWidget* parent) :
    QDockWidget(parent), rxModel(rxModel_), ui(new Ui::DockInputCtl)
{
    ui->setupUi(this);

    // Grid layout with gain controls (device dependent)
    gainLayout = new QGridLayout(this);
    gainLayout->setObjectName(QString::fromUtf8("gainLayout"));
    ui->verticalLayout->insertLayout(2, gainLayout);

    connect(rxModel, &ReceiverModel::gainChanged, this,
            &DockInputCtl::onGainChanged);
    connect(rxModel, &ReceiverModel::autoGainChanged, this,
            &DockInputCtl::onAutoGainChanged);
    connect(rxModel, &ReceiverModel::freqCorrChanged, this,
            &DockInputCtl::onFreqCorrChanged);
    connect(rxModel, &ReceiverModel::iqSwapChanged, this,
            &DockInputCtl::onIqSwapChanged);
    connect(rxModel, &ReceiverModel::dcCancelChanged, this,
            &DockInputCtl::onDcCancelChanged);
    connect(rxModel, &ReceiverModel::iqBalanceChanged, this,
            &DockInputCtl::onIqBalanceChanged);
    connect(rxModel, &ReceiverModel::antennaChanged, this,
            &DockInputCtl::onAntennaChanged);
    connect(rxModel, &ReceiverModel::lnbLoChanged, this,
            &DockInputCtl::onLnbLoChanged);
    connect(rxModel, &ReceiverModel::inputDeviceChanged, this,
            &DockInputCtl::onInputDeviceChanged);
    connect(rxModel, &ReceiverModel::gainStagesChanged, this,
            &DockInputCtl::setGainStages);

    onInputDeviceChanged();
}

DockInputCtl::~DockInputCtl() { delete ui; }

void DockInputCtl::setLnbLo(double freq_mhz)
{
    ui->lnbSpinBox->setValue(freq_mhz);
}

void DockInputCtl::onGainChanged(QString name, double value)
{
    for (int idx = 0; idx < gain_labels.length(); idx++) {
        if (gain_labels.at(idx)->text().contains(name)) {
            int gain = (int)(10 * value);
            gain_sliders.at(idx)->setValue(gain);
            updateLabel(idx, gain);
            break;
        }
    }
}

/**
 * @brief Get current gain.
 * @returns The relative gain between 0.0 and 1.0 or -1 if HW AGC is enabled.
 */
double DockInputCtl::gain(QString& name)
{
    double gain = 0.0;

    for (int idx = 0; idx < gain_labels.length(); ++idx) {
        if (gain_labels.at(idx)->text() == name) {
            gain = 0.1 * (double)gain_sliders.at(idx)->value();
            break;
        }
    }

    return gain;
}

/**
 * Set status of hardware AGC button.
 * @param enabled Whether hardware AGC is enabled or not.
 */
void DockInputCtl::setAgc(bool enabled) { rxModel->setAutoGain(enabled); }

void DockInputCtl::onAutoGainChanged(bool enabled)
{
    ui->agcButton->blockSignals(true);
    ui->agcButton->setChecked(enabled);
    ui->agcButton->blockSignals(false);

    for (int i = 0; i < gain_sliders.length(); ++i) {
        gain_sliders.at(i)->setEnabled(!enabled);
    }
}

/**
 * @brief Get status of hardware AGC button.
 * @return Whether hardware AGC is enabled or not.
 */
bool DockInputCtl::agc() { return ui->agcButton->isChecked(); }

void DockInputCtl::onFreqCorrChanged(double corr)
{
    ui->freqCorrSpinBox->setValue(corr);
}

void DockInputCtl::setFreqCorr(double corr) { rxModel->setFreqCorr(corr); }

/** Get current frequency correction. */
double DockInputCtl::freqCorr() { return ui->freqCorrSpinBox->value(); }

/** Enasble/disable I/Q swapping. */
void DockInputCtl::setIqSwap(bool reversed) { rxModel->setIqSwap(reversed); }

void DockInputCtl::onIqSwapChanged(bool reversed)
{
    ui->iqSwapButton->blockSignals(true);
    ui->iqSwapButton->setChecked(reversed);
    ui->iqSwapButton->blockSignals(false);
}

/** Get current I/Q swapping. */
bool DockInputCtl::iqSwap(void) { return ui->iqSwapButton->isChecked(); }

/** Enable automatic DC removal. */
void DockInputCtl::setDcCancel(bool enabled) { rxModel->setDcCancel(enabled); }

void DockInputCtl::onDcCancelChanged(bool enabled)
{
    ui->dcCancelButton->blockSignals(true);
    ui->dcCancelButton->setChecked(enabled);
    ui->dcCancelButton->blockSignals(false);
}

/** Get current DC remove status. */
bool DockInputCtl::dcCancel(void) { return ui->dcCancelButton->isChecked(); }

/** Enable automatic IQ balance. */
void DockInputCtl::setIqBalance(bool enabled)
{
    rxModel->setIqBalance(enabled);
}

void DockInputCtl::onIqBalanceChanged(bool enabled)
{
    ui->iqBalanceButton->blockSignals(true);
    ui->iqBalanceButton->setChecked(enabled);
    ui->iqBalanceButton->blockSignals(false);
}

/** Get current IQ balance status. */
bool DockInputCtl::iqBalance(void) { return ui->iqBalanceButton->isChecked(); }

/** Enasble/disable ignoring hardware limits. */
void DockInputCtl::setIgnoreLimits(bool reversed)
{
    ui->ignoreButton->setChecked(reversed);
}

/** Get current status of whether limits should be ignored or not. */
bool DockInputCtl::ignoreLimits(void) { return ui->ignoreButton->isChecked(); }

/** Select antenna. */
void DockInputCtl::setAntenna(const QString& antenna)
{
    rxModel->setAntenna(antenna);
}

/** Select antenna. */
void DockInputCtl::onAntennaChanged(const QString& antenna)
{
    int index = ui->antSelector->findText(antenna, Qt::MatchExactly);
    if (index != -1)
        ui->antSelector->setCurrentIndex(index);
}

/**
 * Set gain stages.
 * @param gainStages A list containing the gain stages for this device.
 */
void DockInputCtl::setGainStages(const QList<GainStage>& gainStages)
{
    QLabel* label;
    QSlider* slider;
    QLabel* value;
    int start, stop, step, gain;

    // ensure that gain lists are empty
    clearWidgets();

    for (unsigned int i = 0; i < gainStages.size(); i++) {
        start = (int)(10.0 * gainStages[i].start);
        stop = (int)(10.0 * gainStages[i].stop);
        step = (int)(10.0 * gainStages[i].step);
        gain = (int)(10.0 * gainStages[i].value);

        label = new QLabel(QString("%1 ").arg(gainStages[i].name), this);
        label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);

        value = new QLabel(
            QString(" %1 dB").arg(gainStages[i].value, 0, 'f', 1), this);
        value->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);

        slider = new QSlider(Qt::Horizontal, this);
        slider->setProperty("idx", i);
        slider->setProperty("name", gainStages[i].name);
        slider->setSizePolicy(QSizePolicy::MinimumExpanding,
                              QSizePolicy::Minimum);
        slider->setRange(start, stop);
        slider->setSingleStep(step);
        slider->setValue(gain);
        if (abs(stop - start) > 10 * step)
            slider->setPageStep(10 * step);

        gainLayout->addWidget(label, i, 0, Qt::AlignLeft);
        gainLayout->addWidget(slider, i,
                              1); // setting alignment would force minimum size
        gainLayout->addWidget(value, i, 2, Qt::AlignLeft);

        gain_labels.push_back(label);
        gain_sliders.push_back(slider);
        value_labels.push_back(value);

        connect(slider, SIGNAL(valueChanged(int)), this,
                SLOT(sliderValueChanged(int)));
    }

    qDebug() << "********************";
    for (auto& gainStage : gainStages) {
        qDebug() << "Gain name:" << QString(gainStage.name);
        qDebug() << "      min:" << gainStage.start;
        qDebug() << "      max:" << gainStage.stop;
        qDebug() << "     step:" << gainStage.step;
    }
    qDebug() << "********************";
}

/** LNB LO value has changed. */
void DockInputCtl::on_lnbSpinBox_valueChanged(double value)
{
    ui->lnbSpinBox->blockSignals(true);
    ui->lnbSpinBox->setValue(rxModel->lnbLo());
    ui->lnbSpinBox->blockSignals(false);

    rxModel->setLnbLo(value);
}

/** Automatic gain control button has been toggled. */
void DockInputCtl::on_agcButton_toggled(bool checked)
{
    ui->agcButton->blockSignals(true);
    ui->agcButton->setChecked(!checked);
    ui->agcButton->blockSignals(false);

    setAgc(checked);
}

/**
 * Frequency correction changed.
 * @param value The new frequency correction in ppm.
 */
void DockInputCtl::on_freqCorrSpinBox_valueChanged(double value)
{
    setFreqCorr(value);
}

/**
 * I/Q swapping checkbox changed.
 * @param checked True if I/Q swapping is enabled, false otherwise
 */
void DockInputCtl::on_iqSwapButton_toggled(bool checked)
{
    ui->iqSwapButton->blockSignals(true);
    ui->iqSwapButton->setChecked(!checked);
    ui->iqSwapButton->blockSignals(false);

    setIqSwap(checked);
}

/**
 * DC removal checkbox changed.
 * @param checked True if DC removal is enabled, false otherwise
 */
void DockInputCtl::on_dcCancelButton_toggled(bool checked)
{
    ui->dcCancelButton->blockSignals(true);
    ui->dcCancelButton->setChecked(!checked);
    ui->dcCancelButton->blockSignals(false);

    setDcCancel(checked);
}

/**
 * IQ balance checkbox changed.
 * @param checked True if automatic IQ balance is enabled, false otherwise
 */
void DockInputCtl::on_iqBalanceButton_toggled(bool checked)
{
    ui->iqBalanceButton->blockSignals(true);
    ui->iqBalanceButton->setChecked(!checked);
    ui->iqBalanceButton->blockSignals(false);

    setIqBalance(checked);
}

/*! \brief Ignore hardware limits checkbox changed.
 *  \param checked True if hardware limits should be ignored, false otherwise
 *
 * This option exists to allow experimenting with out-of-spec settings.
 */
void DockInputCtl::on_ignoreButton_toggled(bool checked)
{
    Q_EMIT ignoreLimitsChanged(checked);
}

/** Antenna selection has changed. */
void DockInputCtl::on_antSelector_currentIndexChanged(int index)
{
    setAntenna(ui->antSelector->itemText(index));
}

/** Remove all widgets from the lists. */
void DockInputCtl::clearWidgets()
{
    QWidget* widget;

    // sliders
    while (!gain_sliders.isEmpty()) {
        widget = gain_sliders.takeFirst();
        gainLayout->removeWidget(widget);
        delete widget;
    }

    // labels
    while (!gain_labels.isEmpty()) {
        widget = gain_labels.takeFirst();
        gainLayout->removeWidget(widget);
        delete widget;
    }

    // value labels
    while (!value_labels.isEmpty()) {
        widget = value_labels.takeFirst();
        gainLayout->removeWidget(widget);
        delete widget;
    }
}

/**
 * Slot for managing slider value changed signals.
 * @param value The value of the slider.
 *
 * Note. We use the sender() function to find out which slider has Q_EMITted the
 * signal.
 */
void DockInputCtl::sliderValueChanged(int value)
{
    QSlider* slider = (QSlider*)sender();

    // convert to discrete value according to step
    if (slider->singleStep()) {
        value = slider->singleStep() * (value / slider->singleStep());
    }

    // convert to double and send signal
    double gain = (double)value / 10.0;

    rxModel->setGain(slider->property("name").toString(), gain);
}

/**
 * Update value label
 * @param idx The index of the gain
 * @param value The new value
 */
void DockInputCtl::updateLabel(int idx, double value)
{
    QLabel* label = value_labels.at(idx);

    label->setText(QString("%1 dB").arg(value, 0, 'f', 1));
}

void DockInputCtl::onInputDeviceChanged()
{
    // Add available antenna connectors to the UI
    ui->antSelector->clear();
    ui->antSelector->addItems(rxModel->getAntennas());

    // set gains
    setGainStages(rxModel->getGainStages());
}

void DockInputCtl::onLnbLoChanged(qint64 value)
{
    ui->lnbSpinBox->blockSignals(true);
    ui->lnbSpinBox->setValue(value);
    ui->lnbSpinBox->blockSignals(false);
}
