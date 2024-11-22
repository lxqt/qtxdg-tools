/*
 * libqtxdg - An Qt implementation of freedesktop.org xdg specs
 * Copyright (C) 2018  Luís Pereira <luis.artur.pereira@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#include "openmatcommand.h"
#include "matglobals.h"

#include "xdgdesktopfile.h"
#include "xdgmimeapps.h"

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>
#include <QStringList>
#include <QtGlobal>
#include <QUrl>

#include <iostream>

using namespace Qt::Literals::StringLiterals;

OpenMatCommand::OpenMatCommand(QCommandLineParser *parser)
    : MatCommandInterface(u"open"_s,
                          u"Open files with the default application"_s,
                          parser)
{
}

OpenMatCommand::~OpenMatCommand() = default;

static CommandLineParseResult parseCommandLine(QCommandLineParser *parser, QStringList *files, QString *errorMessage)
{
    parser->clearPositionalArguments();
    parser->setApplicationDescription(u"Open files with the default application"_s);

    parser->addPositionalArgument(u"open"_s, u"files | URLs"_s,
                                  QCoreApplication::tr("[files | URLs]"));

    const QCommandLineOption helpOption = parser->addHelpOption();
    const QCommandLineOption versionOption = parser->addVersionOption();

    if (!parser->parse(QCoreApplication::arguments())) {
        *errorMessage = parser->errorText();
        return CommandLineError;
    }

    if (parser->isSet(versionOption)) {
        return CommandLineVersionRequested;
    }

    if (parser->isSet(helpOption) || parser->isSet(u"help-all"_s)) {
        return CommandLineHelpRequested;
    }

    QStringList fs = parser->positionalArguments();
    if (fs.size() < 2) {
        *errorMessage = u"No file or URL given"_s;
        return CommandLineError;
    }

    fs.removeAt(0);

    *files = fs;

    return CommandLineOk;
}

int OpenMatCommand::run(const QStringList &arguments)
{
    Q_UNUSED(arguments);

    bool success = true;
    QString errorMessage;
    QStringList files;

    switch(parseCommandLine(parser(), &files, &errorMessage)) {
    case CommandLineOk:
        break;
    case CommandLineError:
        std::cerr << qPrintable(errorMessage);
        std::cerr << "\n\n";
        std::cerr << qPrintable(parser()->helpText());
        return EXIT_FAILURE;
    case CommandLineVersionRequested:
        parser()->showVersion();
        Q_UNREACHABLE();
    case CommandLineHelpRequested:
        parser()->showHelp();
        Q_UNREACHABLE();
    }

    XdgMimeApps appsDb;
    QMimeDatabase mimeDb;
    XdgDesktopFile *df = nullptr;
    bool isLocalFile = false;
    QString localFilename;
    for (const QString &urlString : std::as_const(files)) {
        const QUrl url(urlString);
        const QString scheme = url.scheme();
        if (scheme.isEmpty()) {
            isLocalFile = true;
            localFilename = urlString;
        } else if (scheme == "file"_L1) {
            isLocalFile = true;
            localFilename = QUrl(urlString).toLocalFile();
        }

        if (isLocalFile) {
            const QFileInfo f (localFilename);
            if (!f.exists()) {
                std::cerr << qPrintable(u"Cannot access %1: No such file or directory\n"_s.arg(urlString));
                break;
            } else {
                const QMimeType mimeType = mimeDb.mimeTypeForFile(f);
                df = appsDb.defaultApp(mimeType.name());
            }
        } else { // not a local file
			const QString contentType = u"x-scheme-handler/%1"_s.arg(scheme);
			df = appsDb.defaultApp(contentType);
        }

        if (df) { // default app found
            if (!df->startDetached(isLocalFile ? localFilename : urlString)) {
                std::cerr << qPrintable(
                        u"Error while running the default application (%1) for %2\n"_s.arg(df->name(), urlString));
                success = false;
            }
            delete df;
            df = nullptr;
        } else { // no default app found
            std::cout << qPrintable(u"No default application for '%1'\n"_s.arg(urlString));
        }
    }
    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
