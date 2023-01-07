/*
    Cakewalk to Standard MIDI Files Command Line Utility Translator
    Copyright (C) 2021-2023, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QDir>
#include <QVariant>
#include <QStringList>
#include "sequence.h"

int main(int argc, char *argv[])
{
    const QString PGM_NAME = QStringLiteral("wrk2mid");
    const QString PGM_VER = QStringLiteral(QT_STRINGIFY(VERSION));
    const QString DRUMSTICK_VER = QStringLiteral(QT_STRINGIFY(Drumstick_VERSION));
    const QString PGM_DESCRIPTION = QStringLiteral("Command line utility for translating WRK (Cakewalk) files into MID (standard MIDI files)")
        + "\n" + PGM_NAME + " v" + PGM_VER
        + "\nDrumstick v" + DRUMSTICK_VER
        + "\nQt v" + QT_VERSION_STR + " (running with v" + qVersion() + ")";

    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(PGM_NAME);
    QCoreApplication::setApplicationVersion(PGM_VER);

    QCommandLineParser parser;
    parser.setApplicationDescription(PGM_DESCRIPTION);
    auto helpOption = parser.addHelpOption();
    auto versionOption = parser.addVersionOption();
    QCommandLineOption formatOption({"f", "format"}, "SMF Format (0/1)", "format", "1");
    parser.addOption(formatOption);
    QCommandLineOption outputOption({"o", "output"}, "Output file name", "output");
    parser.addOption(outputOption);
    QCommandLineOption testOption({"t", "test"}, "Test only (no output)");
    parser.addOption(testOption);
    parser.addPositionalArgument("file", "Input WRK File Name", "file");
    parser.process(app);

    if (parser.isSet(versionOption) || parser.isSet(helpOption)) {
        return EXIT_SUCCESS;
    }

    Sequence seq;
    if (parser.isSet(formatOption)) {
        bool ok;
        QString format = parser.value(formatOption);
        int f = format.toInt(&ok);
        if (ok && f >= 0 && f <= 1) {
            seq.setOutputFormat(f);
        } else {
            std::cerr << "wrong format: " << format.toStdString() << std::endl;
            std::cerr << parser.helpText().toStdString() << std::endl;
        }
    }

    QStringList fileNames, positionalArgs = parser.positionalArguments();
    foreach(const QVariant& a, positionalArgs) {
        QFileInfo f(a.toString());
        if (f.exists()) {
            fileNames += f.canonicalFilePath();
            break;
        } else {
            std::cerr << "file not found:" << f.fileName().toStdString() << std::endl;
        }
    }

    if (fileNames.isEmpty()) {
        std::cerr << "invalid arguments" << std::endl;
        std::cerr << parser.helpText().toStdString() << std::endl;
    } else {
        QString outfile, infile = fileNames.first();
        if (parser.isSet(outputOption)) {
            outfile = parser.value(outputOption);
        } else {
            QFileInfo finfo(infile);
            outfile = QDir::current().absoluteFilePath(finfo.baseName() + ".mid");
        }
        seq.loadFile(infile);
        if (!parser.isSet(testOption) && seq.returnCode() == 0) {
            seq.saveFile(outfile);
        }
        return seq.returnCode();
    }
    return EXIT_FAILURE;
}
