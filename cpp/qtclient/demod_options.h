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
#ifndef DEMOD_OPTIONS_H
#define DEMOD_OPTIONS_H

#include <QCloseEvent>
#include <QDialog>

namespace Ui
{
class CDemodOptions;
}

class VFOChannelModel;

/*! \brief Widget for adjusting demodulator options.
 *
 */
class CDemodOptions : public QDialog
{
    Q_OBJECT

public:
    /*! \brief Index in the QStackedWidget. */
    enum page {
        PAGE_NO_OPT = 0,
        PAGE_FM_OPT = 1,
        PAGE_AM_OPT = 2,
        PAGE_CW_OPT = 3,
        PAGE_AMSYNC_OPT = 4,
        PAGE_NUM = 5
    };

    explicit CDemodOptions(VFOChannelModel* vfo, QWidget* parent = 0);
    ~CDemodOptions();

    void closeEvent(QCloseEvent* event);

    void setCurrentPage(int index);
    int currentPage() const;

private slots:
    void on_maxdevSelector_currentIndexChanged(int index);
    void on_emphSelector_currentIndexChanged(int index);
    void on_dcrCheckBox_clicked(bool checked);
    void on_cwOffsetSpin_valueChanged(int value);
    void on_syncdcrCheckBox_clicked(bool checked);
    void on_pllBwSelector_currentIndexChanged(int index);

    void onDemodChanged();
    void onCwOffsetChanged();
    void onFmDeemphChanged();
    void onFmMaxDevChanged();
    void onAmDcrChanged();
    void onPllBwChanged();
    void onAmSyncDcrChanged();

    void refreshMaxdevSelector();
    void refreshEmphSelector();
    void refreshDcrCheckBox();
    void refreshCwOffsetSpin();
    void refreshSyncDcrCheckBox();
    void refreshPllBwSelector();

private:
    Ui::CDemodOptions* ui;
    VFOChannelModel* vfo;
};

#endif // DEMOD_OPTIONS_H
