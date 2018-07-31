//------------------------------------------------------------------------------
// playofflinedialog.cpp
//------------------------------------------------------------------------------
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301, USA.
//
//------------------------------------------------------------------------------
// Copyright (C) 2018 PROPHESSOR
//------------------------------------------------------------------------------
#include "playofflinedialog.h"

#include "configuration/doomseekerconfig.h"
#include "copytextdlg.h"
#include "gui/widgets/createserverdialogpage.h"
#include "datapaths.h"
#include "commongui.h"
#include "ini/ini.h"
#include "ini/settingsproviderqt.h"
#include "pathfinder/pathfinder.h"
#include "plugins/engineplugin.h"
#include "plugins/pluginloader.h"
#include "scanner.h"
#include "serverapi/exefile.h"
#include "serverapi/gameexeretriever.h"
#include "serverapi/gamecreateparams.h"
#include "serverapi/gamehost.h"
#include "serverapi/message.h"
#include "serverapi/server.h"
#include "apprunner.h"
#include "commandline.h"
#include "strings.h"

#include <QCheckBox>
#include <QDebug>
#include <QFileDialog>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QObject>
#include <QScopedPointer>
#include <QStandardItemModel>
#include <QTimer>

class PlayOfflineDialog::PrivData {
  public:
    bool bSuppressMissingExeErrors;
    bool bIsServerSetup;
    bool iwadSetExplicitly;
    QList<CreateServerDialogPage*> currentCustomPages;
    EnginePlugin *currentEngine;
    QList<GameCVar> gameModifiers;
};

PlayOfflineDialog::PlayOfflineDialog(QWidget* parent)
    : QDialog(parent) {
    d = new PrivData();
    // Have the console delete itself
    setAttribute(Qt::WA_DeleteOnClose);

    d->bSuppressMissingExeErrors = true;
    d->bIsServerSetup = false;
    d->currentEngine = NULL;
    d->iwadSetExplicitly = false;

    setupUi(this);
    connect(btnAddPwad, SIGNAL( clicked() ), this, SLOT ( btnAddPwadClicked() ) );
    connect(btnBrowseExecutable, SIGNAL( clicked() ), this, SLOT ( btnBrowseExecutableClicked() ) );
    connect(btnCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect(btnCommandLine, SIGNAL( clicked() ), this, SLOT( btnCommandLineClicked() ) );
    connect(btnClearPwadList, SIGNAL( clicked() ), this, SLOT( btnClearPwadListClicked() ) );
    connect(btnDefaultExecutable, SIGNAL( clicked() ), this, SLOT( btnDefaultExecutableClicked() ) );
    connect(btnIwadBrowse, SIGNAL( clicked() ), this, SLOT ( btnIwadBrowseClicked() ) );
    connect(btnLoad, SIGNAL( clicked() ), this, SLOT ( btnLoadClicked() ) );
    connect(btnPlayOffline, SIGNAL( clicked() ), this, SLOT ( btnPlayOfflineClicked() ) );
    connect(btnRemovePwad, SIGNAL( clicked() ), this, SLOT ( btnRemovePwadClicked() ) );
    connect(btnSave, SIGNAL( clicked() ), this, SLOT ( btnSaveClicked() ) );

    connect(cboEngine, SIGNAL( currentIndexChanged(int) ), this, SLOT( cboEngineSelected(int) ) );

    connect(lstAdditionalFiles, SIGNAL( fileSystemPathDropped(const QString& ) ),
            this, SLOT( lstAdditionalFilesPathDnd(const QString&) ) );

    connect(QApplication::instance(), SIGNAL( focusChanged(QWidget*, QWidget*) ), this, SLOT( focusChanged(QWidget*, QWidget*) ));

    cboIwad->setEditable(true);
    lstAdditionalFiles->setModel(new QStandardItemModel(this));

    initPrimary();
    d->bSuppressMissingExeErrors = false;

    // This is a crude solution to the problem where message boxes appear
    // before the actual Create Server dialog. We need to give some time
    // for the Dialog to appear. Unfortunately reimplementing
    // QDialog::showEvent() didn't work very well.
    QTimer::singleShot(1, this, SLOT(firstLoadConfigTimer()) );
}

PlayOfflineDialog::~PlayOfflineDialog() {
    delete d;
}

void PlayOfflineDialog::addIwad(const QString& path) {
    if (path.isEmpty()) {
        return;
    }

    for (int i = 0; i < cboIwad->count(); ++i) {
        if (cboIwad->itemText(i).compare(path) == 0) {
            cboIwad->setCurrentIndex(i);
            return;
        }
    }

    QString cleanPath = Strings::normalizePath(path);

    cboIwad->addItem(cleanPath);
    cboIwad->setCurrentIndex(cboIwad->count() - 1);
}

void PlayOfflineDialog::addWadPath(const QString& strPath) {
    if (strPath.isEmpty()) {
        return;
    }

    QStandardItemModel* model = static_cast<QStandardItemModel*>(lstAdditionalFiles->model());

    // Check if this path exists already, if so - do nothing.
    for(int i = 0; i < model->rowCount(); ++i) {
        QStandardItem* item = model->item(i);
        QString dir = item->text();
        Qt::CaseSensitivity cs;

#ifdef Q_OS_WIN32
        cs = Qt::CaseInsensitive;
#else
        cs = Qt::CaseSensitive;
#endif

        if (dir.compare(strPath, cs) == 0) {
            return;
        }
    }

    QStandardItem* it = new QStandardItem(strPath);

    it->setDragEnabled(true);
    it->setDropEnabled(false);
    it->setToolTip(strPath);

    model->appendRow(it);
}

void PlayOfflineDialog::btnAddPwadClicked() {
    QString dialogDir = gConfig.doomseeker.previousCreateServerWadDir;
    QStringList filesNames = QFileDialog::getOpenFileNames(this, tr("Doomseeker - Add file(s)"), dialogDir);

    if (!filesNames.isEmpty()) {
        // Remember the directory of the first file. This directory will be
        // restored the next time this dialog is opened.
        QFileInfo fi(filesNames[0]);
        gConfig.doomseeker.previousCreateServerWadDir = fi.absolutePath();

        foreach (const QString& strFile, filesNames) {
            addWadPath(strFile);
        }
    }
}

void PlayOfflineDialog::btnBrowseExecutableClicked() {
    QString dialogDir = gConfig.doomseeker.previousCreateServerExecDir;
    QString strFile = QFileDialog::getOpenFileName(this, tr("Doomseeker - Add file"), dialogDir);

    if (!strFile.isEmpty()) {
        QFileInfo fi(strFile);
        gConfig.doomseeker.previousCreateServerExecDir = fi.absolutePath();

        leExecutable->setText(fi.absoluteFilePath());
    }
}

void PlayOfflineDialog::btnClearPwadListClicked() {
    QStandardItemModel* pModel = (QStandardItemModel*)lstAdditionalFiles->model();
    pModel->clear();
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

void PlayOfflineDialog::btnDefaultExecutableClicked() {
    Message message;
    leExecutable->setText(pathToOfflineExe(message));
    qDebug("[PRO][Not Implemented][TODO]: PlayOfflineDialog::btnDefaultExecutableClicked => pathToServerExe -> offline");

    if (!message.isIgnore()) {
        QMessageBox::critical(NULL, tr("Obtaining default server binary path."),
                              message.contents(),QMessageBox::Ok, QMessageBox::Ok);
    }
}

void PlayOfflineDialog::btnIwadBrowseClicked() {
    QString dialogDir = gConfig.doomseeker.previousCreateServerWadDir;
    QString strFile = QFileDialog::getOpenFileName(this, tr("Doomseeker - select IWAD"), dialogDir);

    if (!strFile.isEmpty()) {
        QFileInfo fi(strFile);
        gConfig.doomseeker.previousCreateServerWadDir = fi.absolutePath();

        addIwad(strFile);
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

void PlayOfflineDialog::btnRemovePwadClicked() {
    const bool bSelectNextLowest = true;
    CommonGUI::removeSelectedRowsFromStandardItemView(lstAdditionalFiles, bSelectNextLowest);
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

void PlayOfflineDialog::cboEngineSelected(int index) {
    if (index >= 0) {
        unsigned enginePluginIndex = cboEngine->itemData(index).toUInt();
        if (enginePluginIndex < gPlugins->numPlugins()) {
            initEngineSpecific(gPlugins->info(enginePluginIndex));
        }
    }
}

bool PlayOfflineDialog::commandLineArguments(QString &executable, QStringList &args) {
    const QString errorCapt = tr("Doomseeker - create server");
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
    // Since some operating systems have different offline and server binaries
    // We will see if they are playing offline and switch to the client
    // binary if the specified executable is the same as what is provided
    // as the server.
    Message message;
    QString offlineExePath = pathToOfflineExe(message);
    QString serverExePath = pathToOfflineExe(message); // TODO:
    bool bIsLineEditPotiningToServerBinary = (leExecutable->text() == serverExePath);
    bool bShouldUseClientBinary = (offline || d->bIsServerSetup) && message.isIgnore() && bIsLineEditPotiningToServerBinary;

    params.setHostMode(offline ? GameCreateParams::Offline : GameCreateParams::Host);
    if (bShouldUseClientBinary) {
        params.setExecutablePath(offlineExePath);
    } else {
        params.setExecutablePath(leExecutable->text());
    }

    params.setIwadPath(cboIwad->currentText());
    params.setPwadsPaths(CommonGUI::listViewStandardItemsToStringList(lstAdditionalFiles));

    // Custom pages.
    QStringList customPagesParams;
    foreach (CreateServerDialogPage* page, d->currentCustomPages) {
        if (page->validate()) {
            customPagesParams << page->generateGameRunParameters();
        } else {
            // Pages must take care of displaying their own error messages.
            tabWidget->setCurrentIndex(tabWidget->indexOf(page));
            return false;
        }
    }
    params.setCustomParameters(customPagesParams);

    // Custom parameters
    {
        QString cp = pteCustomParameters->toPlainText();
        Scanner sc(cp.toAscii().constData(), cp.length());
        while (sc.nextString()) {
            QString param = sc->str();
            params.customParameters() << param;
        }
    }

    return true;
}

void PlayOfflineDialog::firstLoadConfigTimer() {
    QString tmpServerCfgPath = gDefaultDataPaths->programsDataDirectoryPath() + PlayOfflineDialog::TEMP_SERVER_CONFIG_FILENAME;

    QFileInfo fi(tmpServerCfgPath);
    if (fi.exists()) {
        loadConfig(tmpServerCfgPath, true);
    }
}


void PlayOfflineDialog::focusChanged(QWidget* oldW, QWidget* newW) {
    // if (newW == leMapname) {
    //     btnAddMapToMaplist->setDefault(true);
    // } else if (oldW == leMapname) {
    //     btnAddMapToMaplist->setDefault(false);
    // }
    qDebug("[PRO][Not Implemented]: PlayOfflineDialog::focusChanged");
}

void PlayOfflineDialog::initEngineSpecific(EnginePlugin* engineInfo) {
    if (engineInfo == d->currentEngine || engineInfo == NULL) {
        return;
    }

    d->currentEngine = engineInfo;

    // Executable path
    Message message;

    if (d->bIsServerSetup) {
        // TODO: something weird, perhaps just refer to offline executable here?
        ServerPtr server = d->currentEngine->server(QHostAddress("127.0.0.1"), 1);
        leExecutable->setText(pathToClientExe(server.data(), message));
    } else {
        leExecutable->setText(pathToOfflineExe(message));
    }

    if (message.isError() && !d->bSuppressMissingExeErrors) {
        QString caption = tr("Doomseeker - error obtaining server binary");
        QString error = tr("Server binary for engine \"%1\" cannot be obtained.\nFollowing error has occured:\n%2").arg(engineInfo->data()->name, message.contents());

        QMessageBox::warning(NULL, caption, error);
    }

    initEngineSpecificPages(engineInfo);
}

void PlayOfflineDialog::initEngineSpecificPages(EnginePlugin* engineInfo) {
    // First, get rid of the original pages.
    foreach (CreateServerDialogPage* page, d->currentCustomPages) {
        delete page;
    }
    d->currentCustomPages.clear();

    // Add new custom pages to the dialog.
    // d->currentCustomPages = engineInfo->createServerDialogPages(this);
    // foreach (CreateServerDialogPage* page, d->currentCustomPages) {
    //     int idxInsertAt = tabWidget->indexOf(tabCustomParameters);
    //     tabWidget->insertTab(idxInsertAt, page, page->name());
    // }
}

void PlayOfflineDialog::initPrimary() {
    cboEngine->clear();

    for (unsigned i = 0; i < gPlugins->numPlugins(); ++i) {
        const EnginePlugin* plugin = gPlugins->info(i);
        cboEngine->addItem(plugin->icon(), plugin->data()->name, i);
    }

    if (cboEngine->count() > 0) {
        cboEngine->setCurrentIndex(0);
    }

    const QString iwads[] = { "doom.wad", "doom1.wad", "doom2.wad", "tnt.wad", "plutonia.wad", "heretic.wad", "hexen.wad", "hexdd.wad", "freedoom.wad", "strife1.wad", "" };

    cboIwad->clear();

    for (int i = 0; !iwads[i].isEmpty(); ++i) {
        PathFinder pathFinder;
        QString path = pathFinder.findFile(iwads[i]);
        if (!path.isEmpty()) {
            cboIwad->addItem(path);
        }
    }
}

bool PlayOfflineDialog::loadConfig(const QString& filename, bool loadingPrevious) {
    QAbstractItemModel* model;
    QStringList stringList;
    QSettings settingsFile(filename, QSettings::IniFormat);
    SettingsProviderQt settingsProvider(&settingsFile);
    Ini ini(&settingsProvider);
    IniSection general = ini.section("General");
    IniSection rules = ini.section("Rules");
    IniSection misc = ini.section("Misc");
    IniSection dmflags = ini.section("DMFlags");

    // General
    if (!d->bIsServerSetup) {
        QString engineName = general["engine"];
        const EnginePlugin* prevEngine = d->currentEngine;
        if(!setEngine(engineName))
            return false;

        bool bChangeExecutable = (prevEngine != d->currentEngine);

        // First let's check if we can use executable stored in the server's config.
        // We will save the path to this executable in a local variable.
        QString executablePath = "";
        if (bChangeExecutable) {
            executablePath = *general["executable"];
            QFileInfo fileInfo(executablePath);
            if (!fileInfo.exists()) {
                // Executable cannot be found, display error message and reset
                // the local variable.
                QMessageBox::warning(NULL, tr("Doomseeker - load server config"), tr("Game executable saved in config cannot be found.\nDefault executable will be used."));
                executablePath = "";
            }
        }

        // If we successfuly retrieved path from the config we shall
        // set this path in the line edit control.
        if (!executablePath.isEmpty()) {
            leExecutable->setText(executablePath);
        }
    }

    if (!(loadingPrevious && d->iwadSetExplicitly)) {
        addIwad(general["iwad"]);
    }

    stringList = general["pwads"].valueString().split(";");
    model = lstAdditionalFiles->model();
    model->removeRows(0, model->rowCount());
    foreach (QString s, stringList) {
        addWadPath(s);
    }

    // Custom pages.
    foreach (CreateServerDialogPage* page, d->currentCustomPages) {
        page->loadConfig(ini);
    }

    // Custom parameters
    pteCustomParameters->document()->setPlainText(misc["CustomParams"]);
    return true;
}

void PlayOfflineDialog::lstAdditionalFilesPathDnd(const QString& path) {
    QFileInfo fileInfo(path);
    if (fileInfo.isFile()) {
        addWadPath(path);
    }
}

void PlayOfflineDialog::makeSetupServerDialog(const EnginePlugin *plugin) {
    d->bSuppressMissingExeErrors = true;
    d->bIsServerSetup = true;
    setEngine(plugin->data()->name);

    btnCommandLine->hide();

    // Disable some stuff
    QWidget *disableControls[] = {
        cboEngine, leExecutable, btnBrowseExecutable, btnDefaultExecutable,
        btnPlayOffline,

        NULL
    };
    for(int i = 0; disableControls[i] != NULL; ++i)
        disableControls[i]->setDisabled(true);
}

QString PlayOfflineDialog::pathToClientExe(Server* server, Message& message) {
    QScopedPointer<ExeFile> f(server->clientExe());
    return f->pathToExe(message);
}

QString PlayOfflineDialog::pathToOfflineExe(Message& message) {
    return GameExeRetriever(*d->currentEngine->gameExe()).pathToOfflineExe(message);
}

void PlayOfflineDialog::runGame(bool offline) {
    const QString errorCapt = tr("Doomseeker - create server");
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
            QString tmpServerConfigPath = gDefaultDataPaths->programsDataDirectoryPath() + PlayOfflineDialog::TEMP_SERVER_CONFIG_FILENAME;
            saveConfig(tmpServerConfigPath);
        }
    }
}

bool PlayOfflineDialog::saveConfig(const QString& filename) {
    QStringList stringList;
    QSettings settingsFile(filename, QSettings::IniFormat);
    SettingsProviderQt settingsProvider(&settingsFile);
    Ini ini(&settingsProvider);
    IniSection general = ini.createSection("General");
    IniSection rules = ini.createSection("Rules");
    IniSection misc = ini.createSection("Misc");
    IniSection dmflags = ini.createSection("DMFlags");

    // General
    general["engine"] = cboEngine->currentText();
    general["executable"] = leExecutable->text();
    general["iwad"] = cboIwad->currentText();

    stringList = CommonGUI::listViewStandardItemsToStringList(lstAdditionalFiles);
    general["pwads"] = stringList.join(";");

    // Custom pages.
    foreach (CreateServerDialogPage* page, d->currentCustomPages) {
        page->saveConfig(ini);
    }

    // Custom parameters
    misc["CustomParams"] = pteCustomParameters->toPlainText();

    if (settingsFile.isWritable()) {
        settingsFile.sync();
        return true;
    }
    return false;
}

bool PlayOfflineDialog::setEngine(const QString &engineName) {
    int engIndex = gPlugins->pluginIndexFromName(engineName);
    if (engIndex < 0) {
        QMessageBox::critical(NULL, tr("Doomseeker - load server config"), tr("Plugin for engine \"%1\" is not present!").arg(engineName));
        return false;
    }

    // Select engine. This will also select the default executable path.
    cboEngine->setCurrentIndex(engIndex);
    return true;
}

void PlayOfflineDialog::setIwadByName(const QString &iwad) {
    d->iwadSetExplicitly = true;
    for (int i = 0; i < cboIwad->count(); ++i) {
        QFileInfo fi(cboIwad->itemText(i));
        if (fi.fileName().compare(iwad, Qt::CaseInsensitive) == 0) {
            cboIwad->setCurrentIndex(i);
            return;
        }
    }
    // If IWAD with given name isn't present on the list try to find it anyway.
    PathFinder pathFinder;
    QString path = pathFinder.findFile(iwad);
    if (!path.isEmpty()) {
        cboIwad->addItem(path);
        cboIwad->setCurrentIndex(cboIwad->count() - 1);
    }
}
