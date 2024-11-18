/*
 * libqtxdg - An Qt implementation of freedesktop.org xdg specs
 * Copyright (C) 2020  Luís Pereira <luis.artur.pereira@gmail.com>
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

#include "defemailclientmatcommand.h"

#include "matglobals.h"
#include "xdgmacros.h"
#include "xdgdefaultapps.h"
#include "xdgdesktopfile.h"

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QString>
#include <QStringList>

#include <iostream>

using namespace Qt::Literals::StringLiterals;

enum DefEmailClientCommandMode {
    CommandModeGetDefEmailClient,
    CommandModeSetDefEmailClient,
    CommandModeListAvailableEmailClients
};

struct DefEmailClientData {
    DefEmailClientData() : mode(CommandModeGetDefEmailClient) {}

    DefEmailClientCommandMode mode;
    QString defEmailClientName;
};

static CommandLineParseResult parseCommandLine(QCommandLineParser *parser, DefEmailClientData *data, QString *errorMessage)
{
    parser->clearPositionalArguments();
    parser->setApplicationDescription("Get/Set the default email client"_L1);

    parser->addPositionalArgument("def-email-client"_L1, ""_L1);

    const QCommandLineOption defEmailClientNameOption(QStringList() << u"s"_s << u"set"_s,
                u"Email Client to be set as default"_s, u"email client"_s);

    const QCommandLineOption listAvailableOption(QStringList() << u"l"_s << u"list-available"_s,
                u"List available email clients"_s);

    parser->addOption(defEmailClientNameOption);
    parser->addOption(listAvailableOption);
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

    const bool isListAvailableSet = parser->isSet(listAvailableOption);
    const bool isDefEmailClientNameSet = parser->isSet(defEmailClientNameOption);
    QString defEmailClientName;

    if (isDefEmailClientNameSet)
        defEmailClientName = parser->value(defEmailClientNameOption);

    QStringList posArgs = parser->positionalArguments();
    posArgs.removeAt(0);

    if (isDefEmailClientNameSet && !posArgs.empty()) {
        *errorMessage = u"Extra arguments given: "_s;
        errorMessage->append(posArgs.join(u','));
        return CommandLineError;
    }

    if (!isDefEmailClientNameSet && !posArgs.empty()) {
        *errorMessage = u"To set the default email client use the -s/--set option"_s;
        return CommandLineError;
    }

    if (isListAvailableSet && (isDefEmailClientNameSet || !posArgs.empty())) {
        *errorMessage = u"list-available can't be used with other options and doesn't take arguments"_s;
        return CommandLineError;
    }

    if (isListAvailableSet) {
        data->mode = CommandModeListAvailableEmailClients;
    } else {
        data->mode = isDefEmailClientNameSet ? CommandModeSetDefEmailClient: CommandModeGetDefEmailClient;
        data->defEmailClientName = defEmailClientName;
    }

    return CommandLineOk;
}

DefEmailClientMatCommand::DefEmailClientMatCommand(QCommandLineParser *parser)
    : MatCommandInterface("def-email-client"_L1,
                          u"Get/Set the default email client"_s,
                          parser)
{
   Q_CHECK_PTR(parser);
}

DefEmailClientMatCommand::~DefEmailClientMatCommand() = default;

int DefEmailClientMatCommand::run(const QStringList & /*arguments*/)
{
    bool success = true;
    DefEmailClientData data;
    QString errorMessage;
    if (!MatCommandInterface::parser()) {
        qFatal("DefEmailClientMatCommand::run: MatCommandInterface::parser() returned a null pointer");
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

    if (data.mode == CommandModeListAvailableEmailClients) {
        const auto emailClients = XdgDefaultApps::emailClients();
        for (const auto *app : emailClients)
            std::cout << qPrintable(XdgDesktopFile::id(app->fileName())) << "\n";

        qDeleteAll(emailClients);
        return EXIT_SUCCESS;
    }

    if (data.mode == CommandModeGetDefEmailClient) { // Get default email client
        XdgDesktopFile *defEmailClient = XdgDefaultApps::emailClient();
        if (defEmailClient != nullptr && defEmailClient->isValid()) {
            std::cout << qPrintable(XdgDesktopFile::id(defEmailClient->fileName())) << "\n";
            delete defEmailClient;
        }
    } else { // Set default email client
        XdgDesktopFile toSetDefEmailClient;
        if (toSetDefEmailClient.load(data.defEmailClientName)) {
            if (XdgDefaultApps::setEmailClient(toSetDefEmailClient)) {
                std::cout << qPrintable(u"Set '%1' as the default email client\n"_s.arg(toSetDefEmailClient.fileName()));
            } else {
                std::cerr << qPrintable(u"Could not set '%1' as the default email client\n"_s.arg(toSetDefEmailClient.fileName()));
                success = false;
            }
        } else { // could not load application file
            std::cerr << qPrintable(u"Could not find find '%1'\n"_s.arg(data.defEmailClientName));
            success = false;
        }
    }

    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
