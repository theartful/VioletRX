/* -*- c++ -*- */
/*
 * Gqrx SDR: Software defined radio receiver powered by GNU Radio and Qt
 *           https://gqrx.dk/
 *
 * Copyright 2013-2016 Alexandru Csete OZ9AEC.
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
#ifndef AUDIO_OPTIONS_H
#define AUDIO_OPTIONS_H

#include <QDialog>

namespace Ui {
    class CAudioOptions;
}

class QCloseEvent;
class QPalette;
class QDir;

/*! \brief GUI widget for configuring audio options. */
class CAudioOptions : public QDialog
{
    Q_OBJECT

public:
    explicit CAudioOptions(QWidget *parent = 0);
    ~CAudioOptions();

    void closeEvent(QCloseEvent *event);

    void setRecDir(const QString &dir);
    void setUdpHost(const QString &host);
    void setUdpPort(int port);
    void setUdpStereo(bool stereo);
    void setUdpOptionsEnabled(bool enabled);

    void setFftSplit(int pct_2d);

    QString udpHost();
    int udpPort();
    bool udpStereo();

signals:
    /*! \brief Signal emitted when a new valid directory has been selected. */
    void newRecDirSelected(const QString &dir);

    void newUdpHost(const QString text);
    void newUdpPort(int port);
    void newUdpStereo(bool enabled);

private slots:
    void on_recDirEdit_textChanged(const QString &text);
    void on_recDirButton_clicked();
    void on_udpHost_textChanged(const QString &text);
    void on_udpPort_valueChanged(int port);
    void on_udpStereo_stateChanged(int state);

private:
    Ui::CAudioOptions *ui;                   /*!< The user interface widget. */
    QDir              *work_dir;             /*!< Used for validating chosen directory. */
    QPalette          *error_palette;        /*!< Palette used to indicate an error. */
    bool               m_pand_last_modified; /*!< Flag to indicate which slider was changed last */
};

#endif // AUDIO_OPTIONS_H
