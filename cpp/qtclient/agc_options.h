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
#ifndef AGC_OPTIONS_H
#define AGC_OPTIONS_H

#include <QCloseEvent>
#include <QDialog>

namespace Ui
{
class CAgcOptions;
}

class VFOChannelModel;

/*! \brief Dialog windows with advanced AGC controls.
 *  \ingroup UI
 *
 * By default the user is presented with a combo box and a small tool button.
 * The combo box contains the most common AGC presets (fast, medium, slow, user)
 * plus an option to switch AGC OFF. The controls for the individual AGC
 * parameters are inside this dialog and can be shown using the small tool
 * button next to the combo box containing the presets.
 *
 * \todo A graph that shows the current AGC profile updated in real time.
 */

class CAgcOptions : public QDialog
{
    Q_OBJECT

public:
    explicit CAgcOptions(VFOChannelModel* vfo, QWidget* parent = 0);
    ~CAgcOptions();

    void closeEvent(QCloseEvent* event);
    void agcParamChanged();

private Q_SLOTS:
    void on_gainSlider_valueChanged(int gain);
    void on_thresholdSlider_valueChanged(int threshold);
    void on_slopeSlider_valueChanged(int slope);
    void on_decaySlider_valueChanged(int decay);
    void on_hangButton_toggled(bool checked);

private:
    Ui::CAgcOptions* ui;
    VFOChannelModel* vfo;
};

#endif // AGC_OPTIONS_H
