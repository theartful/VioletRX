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

#include "mainwindow.h"

#include <iostream>

static void reset_conf(const QString& file_name);
static void list_conf();

int main(int argc, char* argv[])
{
    QString cfg_file;
    bool edit_conf = false;
    int return_code = 0;

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
    if (qputenv("GR_CONF_CONTROLPORT_ON", "False"))
        qDebug() << "Controlport disabled";
    else
        qDebug() << "Failed to disable controlport";

    QCommandLineParser parser;
    parser.setApplicationDescription(
        "VioletRX software defined radio receiver");
    parser.addHelpOption();
    parser.addOptions({
        {{"s", "style"}, "Use the given style (fusion, windows)", "style"},
        {{"l", "list"}, "List existing configurations"},
        {{"c", "conf"}, "Start with this config file", "file"},
        {{"e", "edit"}, "Edit the config file before using it"},
        {{"r", "reset"}, "Reset configuration file"},
    });
    parser.process(app);

    if (parser.isSet("style"))
        QApplication::setStyle(parser.value("style"));

    if (parser.isSet("list")) {
        list_conf();
        return 0;
    }

    if (parser.isSet("conf")) {
        cfg_file = parser.value("conf");
        qDebug() << "User specified config file:" << cfg_file;
    } else {
        cfg_file = "default.conf";
        qDebug() << "No user supplied config file. Using" << cfg_file;
    }

    if (parser.isSet("reset"))
        reset_conf(cfg_file);
    else if (parser.isSet("edit"))
        edit_conf = true;

    try {
        // Mainwindow will check whether we have a configuration
        // and open the config dialog if there is none or the specified
        // file does not exist.
        MainWindow w(cfg_file, edit_conf);

        if (w.configOk) {
            w.show();
            return_code = QApplication::exec();
        } else {
            return_code = 1;
        }
    } catch (std::exception& x) {
        QMessageBox::warning(
            nullptr, QObject::tr("Crash detected"),
            QObject::tr("<p>VioletRX exited with an exception:</p>"
                        "<p><b>%1</b></p>"
                        "<p>More information can be found by "
                        "launching it from the console.</p>")
                .arg(x.what()),
            QMessageBox::Close);
    }

#ifdef WITH_PORTAUDIO
    Pa_Terminate();
#endif

    return return_code;
}

/** Reset configuration file specified by file_name. */
static void reset_conf(const QString& file_name)
{
    QString cfg_file;
    QByteArray xdg_dir = qgetenv("XDG_CONFIG_HOME");

    if (xdg_dir.isEmpty())
        cfg_file = QString("%1/.config/VioletRX/%2")
                       .arg(QDir::homePath())
                       .arg(file_name);
    else
        cfg_file = QString("%1/VioletRX/%2").arg(xdg_dir.data()).arg(file_name);

    if (QFile::exists(cfg_file)) {
        if (QFile::remove(cfg_file))
            qDebug() << cfg_file << "deleted";
        else
            qDebug() << "Failed to remove" << cfg_file;
    } else {
        qDebug() << "Can not delete" << cfg_file << "- file does not exist!";
    }
}

/** List available configurations. */
static void list_conf()
{
    QString conf_path;
    QByteArray xdg_dir = qgetenv("XDG_CONFIG_HOME");

    if (xdg_dir.isEmpty())
        conf_path = QString("%1/.config/VioletRX/").arg(QDir::homePath());
    else
        conf_path = QString("%1/VioletRX/").arg(xdg_dir.data());

    QDir conf_dir = QDir(conf_path, "*.conf", QDir::Name, QDir::Files);
    QStringList conf_files = conf_dir.entryList(QStringList("*.conf"));

    std::cout << std::endl
              << " Existing configuration files in " << conf_path.toStdString()
              << std::endl;

    if (conf_files.isEmpty()) {
        std::cout << "     *** NONE ***" << std::endl;
    } else {
        for (int i = 0; i < conf_files.count(); i++) {
            std::cout << "     ";
            std::cout << conf_files.at(i).toLocal8Bit().constData()
                      << std::endl;
        }
    }
}
