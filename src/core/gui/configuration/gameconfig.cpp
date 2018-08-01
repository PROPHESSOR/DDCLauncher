//------------------------------------------------------------------------------
// gameconfig.cpp
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
#include "gameconfig.h"
#include "qtmetapointer.h"
#include <Qt>
#include <QDebug>
#include <QKeyEvent>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QTreeView>
#include <QAbstractButton>
#include <QMessageBox>
#include "apprunner.h"
#include "plugins/engineplugin.h"
#include <QProcess>
#include <QFileDialog>

class GameConfig::PrivData {
  public:
    QString gamePath;
};

GameConfig::GameConfig(QWidget* parent)
    : QDialog(parent) {
    d = new PrivData();
    setupUi(this);
}

GameConfig::~GameConfig() {
    delete d;
}

QString GameConfig::generateArguments() {
    QString arguments = "";
    if(cbNickEnable->isEnabled() && cbNickEnable->isChecked()) arguments.append(" +Name \""+inputNick->text()+"\"");

    return arguments;
}

void GameConfig::runGame() {
    bool result = QProcess::startDetached(inputExecutable->text() + generateArguments());
    if(result) qDebug("Game started");
    else qDebug("Game start error!");
}

void GameConfig::on_btnClose_clicked() {
    reject();
}

void GameConfig::on_btnApply_clicked() {
    runGame();
    reject();
}

void GameConfig::on_btnOpenExec_clicked() {
    QString exec = QFileDialog::getOpenFileName(this, tr("Game executable"), "~", tr("Any file (*)"));

    inputExecutable->setText(exec);
}

void GameConfig::on_cbNickEnable_stateChanged(int state) {
    cbNickColorful->setEnabled(state);
    inputNick->setEnabled(state);
}

void GameConfig::on_inputExecutable_textChanged(const QString &text) {
    if(!text.isEmpty()) {
        cbNickEnable->setEnabled(true);
    } else {
        cbNickEnable->setEnabled(false);
    }
}
