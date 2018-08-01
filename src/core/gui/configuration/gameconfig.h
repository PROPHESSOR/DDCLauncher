//------------------------------------------------------------------------------
// gameconfig.h
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
#pragma once

#include "ui_gameconfig.h"
#include <QDialog>

class Message;
class EnginePlugin;

class GameConfig : public QDialog, private Ui::GameConfig {
    Q_OBJECT

  public:
    GameConfig(QWidget* parent = NULL);
    ~GameConfig();
    void runGame();
  private slots:
    void on_btnClose_clicked();

    void on_btnApply_clicked();

    void on_btnOpenExec_clicked();

    void on_cbNickEnable_stateChanged(int);

    void on_inputExecutable_textChanged(const QString&);

  private:
    class PrivData;
    PrivData *d;
    QString generateArguments();
};
