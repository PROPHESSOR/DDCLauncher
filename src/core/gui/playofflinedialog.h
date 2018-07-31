//------------------------------------------------------------------------------
// playofflinedialog.h
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
#ifndef DOOMSEEKER_GUI_PLAYOFFLINEDIALOG_H
#define DOOMSEEKER_GUI_PLAYOFFLINEDIALOG_H

#include "serverapi/serverstructs.h"
#include "dptr.h"
#include <QDialog>

class CreateServerDialogPage;
class EnginePlugin;
class GameCreateParams;
class Message;
class Server;

/**
 * @brief Dialog window allowing user to host a game.
 */
class PlayOfflineDialog : public QDialog {
    Q_OBJECT

  public:
    PlayOfflineDialog(QWidget* parent = NULL);
    virtual ~PlayOfflineDialog();

    bool commandLineArguments(QString &executable, QStringList &args);
    void setIwadByName(const QString &iwad);

  private slots:
    void btnCommandLineClicked();
    void btnLoadClicked();
    void btnPlayOfflineClicked();
    void btnSaveClicked();
    void firstLoadConfigTimer();

  private:
    static const QString TEMP_SERVER_CONFIG_FILENAME;

    DPtr<PlayOfflineDialog> d;

    /**
     * Adds IWAD path to the IWAD ComboBox.
     * If the path already exists no new items will be added but this path
     * will be selected.
     */
    void addIwad(const QString& path);

    /**
     * Sets host information for both server and hi objects. Both
     * parameters obtain new information after this method is called.
     * @return false if fail.
     */
    bool createHostInfo(GameCreateParams& params, bool offline);

    bool loadConfig(const QString& filename, bool loadingPrevious);
    void runGame(bool offline);
    bool saveConfig(const QString& filename);
};

#endif
