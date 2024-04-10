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
#ifndef DOCKINPUTCTL_H
#define DOCKINPUTCTL_H

#include <string>
#include <vector>

#include <QDockWidget>
#include <QGridLayout>
#include <QLabel>
#include <QList>
#include <QMap>
#include <QSlider>
#include <QString>
#include <QVariant>

#include "qtclient/receiver_model.h"

/*! \brief Structure describing a gain parameter with its range. */
typedef struct {
    std::string name; /*!< The name of this gain stage. */
    double value;     /*!< Initial value. */
    double start;     /*!< The lower limit. */
    double stop;      /*!< The uppewr limit. */
    double step;      /*!< The resolution/step. */
} gain_t;

/*! \brief A vector with gain parameters.
 *
 * This data structure is used for transferring
 * information about available gain stages.
 */
typedef std::vector<gain_t> gain_list_t;

namespace Ui
{
class DockInputCtl;
}

class DockInputCtl : public QDockWidget
{
    Q_OBJECT

public:
    explicit DockInputCtl(ReceiverModel* rxModel, QWidget* parent = 0);
    ~DockInputCtl();

    double gain(QString& name);

    void setAgc(bool enabled);
    bool agc();

    void setFreqCorr(double corr);
    double freqCorr();

    void setIqSwap(bool reversed);
    bool iqSwap(void);

    void setDcCancel(bool enabled);
    bool dcCancel(void);

    void setIqBalance(bool enabled);
    bool iqBalance(void);

    void setIgnoreLimits(bool reversed);
    bool ignoreLimits(void);

    void setAntenna(const QString& antenna);

    void setGainStages(const QList<GainStage>& gain_list);

private slots:
    void onGainChanged(QString name, double value);
    void onAutoGainChanged(bool enabled);
    void onFreqCorrChanged(double value);
    void onIqSwapChanged(bool reverse);
    void onDcCancelChanged(bool enabled);
    void onIqBalanceChanged(bool enabled);
    void onAntennaChanged(const QString& antenna);
    void onLnbLoChanged(qint64);
    void onInputDeviceChanged();
    void setLnbLo(double freq_mhz);

signals:
    void ignoreLimitsChanged(bool ignore);

private slots:
    void on_lnbSpinBox_valueChanged(double value);
    void on_agcButton_toggled(bool checked);
    void on_freqCorrSpinBox_valueChanged(double value);
    void on_iqSwapButton_toggled(bool checked);
    void on_dcCancelButton_toggled(bool checked);
    void on_iqBalanceButton_toggled(bool checked);
    void on_ignoreButton_toggled(bool checked);
    void on_antSelector_currentIndexChanged(int index);

    void sliderValueChanged(int value);

private:
    void clearWidgets();
    void updateLabel(int idx, double value);
    void setGains(QMap<QString, QVariant>* gains);

private:
    ReceiverModel* rxModel;

    QList<QSlider*> gain_sliders; /*!< A list containing the gain sliders. */
    QList<QLabel*> gain_labels;   /*!< A list containing the gain labels. */
    QList<QLabel*> value_labels;  /*!< A list containing labels showing the
                                     current gain value. */

    Ui::DockInputCtl* ui; /*!< User interface. */
    QGridLayout*
        gainLayout; /*!< Grid layout containing gain controls and labels. */
};

#endif // DOCKINPUTCTL_H
