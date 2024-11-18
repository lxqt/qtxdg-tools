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

#include "defappmatcommand.h"
#include "matglobals.h"

#include "xdgdesktopfile.h"
#include "xdgmacros.h"
#include "xdgmimeapps.h"

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>

#include <iostream>

using namespace Qt::Literals::StringLiterals;

enum DefAppCommandMode {
    CommandModeGetDefApp,
    CommandModeSetDefApp
};

struct DefAppData {
    DefAppData() : mode(CommandModeGetDefApp) {}

    DefAppCommandMode mode;
    QString defAppName;
    QStringList mimeTypes;
};

static CommandLineParseResult parseCommandLine(QCommandLineParser *parser, DefAppData *data, QString *errorMessage)
{
    parser->clearPositionalArguments();
    parser->setApplicationDescription(u"Get/Set the default application for a mimetype"_s);

    parser->addPositionalArgument(u"defapp"_s, u"mimetype(s)"_s,
                                  QCoreApplication::tr("[mimetype(s)...]"));

    const QCommandLineOption defAppNameOption(QStringList() << u"s"_s << u"set"_s,
                u"Application to be set as default"_s, u"app name"_s);

    parser->addOption(defAppNameOption);
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

    const bool isDefAppNameSet = parser->isSet(defAppNameOption);
    QString defAppName;

    if (isDefAppNameSet)
        defAppName = parser->value(defAppNameOption);

    if (isDefAppNameSet && defAppName.isEmpty()) {
        *errorMessage = u"No application name"_s;
        return CommandLineError;
    }

    QStringList mimeTypes = parser->positionalArguments();

    if (mimeTypes.size() < 2) {
        *errorMessage = u"MimeType missing"_s;
        return CommandLineError;
    }

    mimeTypes.removeAt(0);

    if (!isDefAppNameSet && mimeTypes.size() > 1) {
        *errorMessage = u"Only one mimeType, please"_s;
        return CommandLineError;
    }

    data->mode = isDefAppNameSet ? CommandModeSetDefApp : CommandModeGetDefApp;
    data->defAppName = defAppName;
    data->mimeTypes = mimeTypes;

    return CommandLineOk;
}

DefAppMatCommand::DefAppMatCommand(QCommandLineParser *parser)
    : MatCommandInterface(u"defapp"_s,
                          u"Get/Set the default application for a mimetype"_s,
                          parser)
{
   Q_CHECK_PTR(parser);
}

DefAppMatCommand::~DefAppMatCommand() = default;

int DefAppMatCommand::run(const QStringList & /*arguments*/)
{
    bool success = true;
    DefAppData data;
    QString errorMessage;
    if (!MatCommandInterface::parser()) {
        qFatal("DefAppMatCommand::run: MatCommandInterface::parser() returned a null pointer");
    }

    switch(parseCommandLine(parser(), &data, &errorMessage)) {
    case CommandLineOk:
        break;
    case CommandLineError:
        std::cerr << qPrintable(errorMessage);
        std::cerr << "\n\n";
        std::cerr << qPrintable(parser()->helpText());
        return EXIT_FAILURE;
    case CommandLineVersionRequested:
        showVersion();
        Q_UNREACHABLE();
    case CommandLineHelpRequested:
        showHelp();
        Q_UNREACHABLE();
    }

    if (data.mode == CommandModeGetDefApp) { // Get default App
        XdgMimeApps apps;
        const QString mimeType = data.mimeTypes.constFirst();
        XdgDesktopFile *defApp = apps.defaultApp(mimeType);
        if (defApp != nullptr) {
            std::cout << qPrintable(XdgDesktopFile::id(defApp->fileName())) << "\n";
            delete defApp;
        } else {
//            std::cout << qPrintable(u"No default application for '%1'\n"_s.arg(mimeType));
        }
    } else { // Set default App
        XdgDesktopFile app;
        if (!app.load(data.defAppName)) {
            std::cerr << qPrintable(u"Could not find find '%1'\n"_s.arg(data.defAppName));
            return EXIT_FAILURE;
        }

        XdgMimeApps apps;
        for (const QString &mimeType : std::as_const(data.mimeTypes)) {
            if (!apps.setDefaultApp(mimeType, app)) {
                std::cerr << qPrintable(u"Could not set '%1' as default for '%2'\n"_s.arg(app.fileName(), mimeType));
                success = false;
            } else {
                std::cout << qPrintable(u"Set '%1' as default for '%2'\n"_s.arg(app.fileName(), mimeType));
            }
        }
    }
    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
