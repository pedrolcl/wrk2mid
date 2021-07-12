/*
    Cakewalk to Standard MIDI Files Command Line Utility Translator
    Copyright (C) 2021, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include <QDebug>
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
    const QString PGM_DESCRIPTION = QStringLiteral("Command line utility for translating WRK (Cakewalk) files into MID (standard MIDI files)");

    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(PGM_NAME);
    QCoreApplication::setApplicationVersion(QStringLiteral(QT_STRINGIFY(VERSION)));

    QCommandLineParser parser;
    parser.setApplicationDescription(PGM_DESCRIPTION);
    auto helpOption = parser.addHelpOption();
    auto versionOption = parser.addVersionOption();
    QCommandLineOption formatOption({"f", "format"}, "SMF Format (0/1)", "format", "1");
    parser.addOption(formatOption);
    QCommandLineOption outputOption({"o", "output"}, "Output file name", "output");
    parser.addOption(outputOption);
    parser.addPositionalArgument("file", "Input WRK File Name", "file");
    parser.process(app);

    if (parser.isSet(versionOption) || parser.isSet(helpOption)) {
        return EXIT_SUCCESS;
    }

    Sequence seq;
    if (parser.isSet(formatOption)) {
        QString format = parser.value(formatOption);
        int f = format.toInt();
        if (f >= 0 && f <= 1) {
            seq.setOutputFormat(f);
            //qDebug() << "format:" << f;
        } else {
            qWarning() << "wrong format:" << f;
        }
    }

    QStringList fileNames, positionalArgs = parser.positionalArguments();
    foreach(const QVariant& a, positionalArgs) {
        QFileInfo f(a.toString());
        if (f.exists()) {
            fileNames += f.canonicalFilePath();
            //qDebug() << "input:" << fileNames;
            break;
        } else {
            qWarning() << "file not found:" << f.fileName();
        }
    }

    if (!fileNames.isEmpty()) {
        QString outfile, infile = fileNames.first();
        if (parser.isSet(outputOption)) {
            outfile = parser.value(outputOption);
        } else {
            QFileInfo finfo(infile);
            outfile = finfo.path() + QDir::separator() + finfo.baseName() + ".mid";
        }
        //qDebug() << "output:" << outfile;
        seq.loadFile(infile);
        seq.saveFile(outfile);
        return seq.returnCode();
    }
    return EXIT_FAILURE;
}
