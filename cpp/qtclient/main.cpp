/* -*- c++ -*- */
/*
 * Gqrx SDR: Software defined radio receiver powered by GNU Radio and Qt
 *           https://VioletRX.dk/
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
#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QFile>
#include <QLoggingCategory>
#include <QMessageBox>
#include <QString>
#include <QStringList>
#include <QtGlobal>

#include <spdlog/spdlog.h>

#include "mainwindow.h"

int main(int argc, char* argv[])
{
    spdlog::set_level(spdlog::level::debug);

    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("VioletRX");
    QCoreApplication::setApplicationName("VioletRx");
    QCoreApplication::setApplicationVersion("0.1");
    QLoggingCategory::setFilterRules("*.debug=false");

    QString plugin_path = QDir::cleanPath(
        QCoreApplication::applicationDirPath() + "/../soapy-modules");
    QFileInfo plugin_path_info(plugin_path);
    if (plugin_path_info.isDir()) {
        qputenv("SOAPY_SDR_PLUGIN_PATH", plugin_path.toUtf8());
        qputenv("SOAPY_SDR_ROOT", "/invalid");
    }

    // setup controlport via environment variables
    // see
    // http://lists.gnu.org/archive/html/discuss-gnuradio/2013-05/msg00270.html
    // Note: tried using gr::prefs().save() but that doesn't have effect until
    // the next time
    if (qputenv("GR_CONF_CONTROLPORT_ON", "False")) {
        qDebug() << "Controlport disabled";
    } else {
        qDebug() << "Failed to disable controlport";
    }

    QCommandLineParser parser;
    parser.setApplicationDescription(
        "VioletRX software defined radio receiver");
    parser.addHelpOption();
    parser.addOptions({
        {{"s", "style"}, "Use the given style (fusion, windows)", "style"},
    });
    parser.process(app);

    if (parser.isSet("style"))
        QApplication::setStyle(parser.value("style"));

    MainWindow w{};
    w.show();
    return QApplication::exec();
}
