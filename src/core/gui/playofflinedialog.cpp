//------------------------------------------------------------------------------
// playofflinedialog.cpp
//------------------------------------------------------------------------------
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301  USA
//
//------------------------------------------------------------------------------
// Copyright (C) 2009-2012 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "playofflinedialog.h"
#include "ui_playofflinedialog.h"

#include "configuration/doomseekerconfig.h"
#include "copytextdlg.h"
// #include "gui/widgets/createserverdialogpage.h"
#include "datapaths.h"
#include "ini/ini.h"
#include "ini/settingsproviderqt.h"
#include "plugins/engineplugin.h"
#include "serverapi/gamecreateparams.h"
#include "serverapi/gamehost.h"
#include "serverapi/message.h"
#include "apprunner.h"
#include "commandline.h"
#include "gamedemo.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>

DClass<PlayOfflineDialog> :
public Ui::PlayOfflineDialog {
public:
    bool remoteGameSetup;
    QList<CreateServerDialogPage*> currentCustomPages;
    EnginePlugin *currentEngine;
};

DPointered(PlayOfflineDialog)

const QString PlayOfflineDialog::TEMP_SERVER_CONFIG_FILENAME = "/tmpserver.ini";

PlayOfflineDialog::PlayOfflineDialog(QWidget* parent)
    : QDialog(parent) {
    // Have the console delete itself
    setAttribute(Qt::WA_DeleteOnClose);

    d->remoteGameSetup = false;
    d->currentEngine = NULL;

    d->setupUi(this);

    qDebug("[PRO][Not Implemented]PlayOfflineDialog::PlayOfflineDialog");

    // d->generalSetupPanel->setCreateServerDialog(this);
    // d->rulesPanel->setCreateServerDialog(this);

    // This is a crude solution to the problem where message boxes appear
    // before the actual Create Server dialog. We need to give some time
    // for the Dialog to appear. Unfortunately reimplementing
    // QDialog::showEvent() didn't work very well.
    QTimer::singleShot(1, this, SLOT(firstLoadConfigTimer()) );
}

PlayOfflineDialog::~PlayOfflineDialog() {
}

void PlayOfflineDialog::btnCommandLineClicked() {
    QString executable;
    QStringList args;
    if(commandLineArguments(executable, args)) {
        // Lines below directly modify the passed values.
        CommandLine::escapeExecutable(executable);
        CommandLine::escapeArgs(args);

        CopyTextDlg ctd(executable + " " + args.join(" "), "Host server command line:", this);
        ctd.exec();
    }
}

void PlayOfflineDialog::btnLoadClicked() {
    QString dialogDir = gConfig.doomseeker.previousCreateServerConfigDir;
    QString strFile = QFileDialog::getOpenFileName(this, tr("Doomseeker - load server config"), dialogDir, tr("Config files (*.ini)"));

    if (!strFile.isEmpty()) {
        QFileInfo fi(strFile);
        gConfig.doomseeker.previousCreateServerConfigDir = fi.absolutePath();

        loadConfig(strFile, false);
    }
}

void PlayOfflineDialog::btnPlayOfflineClicked() {
    runGame(true);
}

void PlayOfflineDialog::btnSaveClicked() {
    QString dialogDir = gConfig.doomseeker.previousCreateServerConfigDir;
    QString strFile = QFileDialog::getSaveFileName(this, tr("Doomseeker - save server config"), dialogDir, tr("Config files (*.ini)"));
    if (!strFile.isEmpty()) {
        QFileInfo fi(strFile);
        gConfig.doomseeker.previousCreateServerConfigDir = fi.absolutePath();

        if (fi.suffix().isEmpty()) {
            strFile += ".ini";
        }

        if (!saveConfig(strFile)) {
            QMessageBox::critical(NULL, tr("Doomseeker - save server config"), tr("Unable to save server configuration!"));
        }
    }

}

bool PlayOfflineDialog::commandLineArguments(QString &executable, QStringList &args) {
    const QString errorCapt = tr("Doomseeker - create game");
    if (d->currentEngine == NULL) {
        QMessageBox::critical(NULL, errorCapt, tr("No engine selected"));
        return false;
    }

    GameCreateParams gameParams;
    if (createHostInfo(gameParams, false)) {
        CommandLineInfo cli;
        QString error;

        GameHost* gameRunner = d->currentEngine->gameHost();
        Message message = gameRunner->createHostCommandLine(gameParams, cli);

        delete gameRunner;

        if (message.isError()) {
            QMessageBox::critical(NULL, tr("Doomseeker - error"), message.contents());
            return false;
        } else {
            executable = cli.executable.absoluteFilePath();
            args = cli.args;
            return true;
        }
    }
    return false;
}

bool PlayOfflineDialog::createHostInfo(GameCreateParams& params, bool offline) {
    if (d->remoteGameSetup) {
        params.setHostMode(GameCreateParams::Remote);
    } else {
        params.setHostMode(offline ? GameCreateParams::Offline : GameCreateParams::Host);
    }
    d->generalSetupPanel->fillInParams(params);

    d->customParamsPanel->fillInParams(params);

    return true;
}

void PlayOfflineDialog::firstLoadConfigTimer() {
    QString tmpServerCfgPath = gDefaultDataPaths->programsDataDirectoryPath() + TEMP_SERVER_CONFIG_FILENAME;

    QFileInfo fi(tmpServerCfgPath);
    if (fi.exists()) {
        loadConfig(tmpServerCfgPath, true);
    }
}

bool PlayOfflineDialog::loadConfig(const QString& filename, bool loadingPrevious) {
    QSettings settingsFile(filename, QSettings::IniFormat);
    SettingsProviderQt settingsProvider(&settingsFile);
    Ini ini(&settingsProvider);

    d->generalSetupPanel->loadConfig(ini, loadingPrevious);

    d->customParamsPanel->loadConfig(ini);
    return true;
}

void PlayOfflineDialog::runGame(bool offline) {
    const QString errorCapt = tr("Doomseeker - create game");
    if (d->currentEngine == NULL) {
        QMessageBox::critical(NULL, errorCapt, tr("No engine selected"));
        return;
    }

    GameCreateParams gameParams;
    if (createHostInfo(gameParams, offline)) {
        QString error;

        GameHost* gameRunner = d->currentEngine->gameHost();
        Message message = gameRunner->host(gameParams);

        delete gameRunner;

        if (message.isError()) {
            QMessageBox::critical(NULL, tr("Doomseeker - error"), message.contents());
        } else {
            QString tmpServerConfigPath = gDefaultDataPaths->programsDataDirectoryPath() + TEMP_SERVER_CONFIG_FILENAME;
            saveConfig(tmpServerConfigPath);
        }
    }
}

bool PlayOfflineDialog::saveConfig(const QString& filename) {
    QSettings settingsFile(filename, QSettings::IniFormat);
    SettingsProviderQt settingsProvider(&settingsFile);
    Ini ini(&settingsProvider);
    IniSection general = ini.section("General");

    d->generalSetupPanel->saveConfig(ini);

    d->customParamsPanel->saveConfig(ini);

    if (settingsFile.isWritable()) {
        settingsFile.sync();
        return true;
    }
    return false;
}

void PlayOfflineDialog::setIwadByName(const QString &iwad) {
    d->generalSetupPanel->setIwadByName(iwad);
}
