//------------------------------------------------------------------------------
// cfgwadseekersites.cpp
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
// Copyright (C) 2010 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "cfgwadseekersites.h"
#include "configuration/doomseekerconfig.h"
#include "wadseeker/wadseeker.h"
#include <QCompleter>
#include <QDebug>
#include <QDirModel>
#include <QFileDialog>
#include <QMessageBox>
#include <QUrl>

CFGWadseekerSites::CFGWadseekerSites(QWidget* parent) 
: ConfigurationBaseBox(parent)
{
	setupUi(this);

	lstUrls->setModel(new QStandardItemModel());

	connect(btnUrlAdd, SIGNAL( clicked() ), this, SLOT( btnUrlAddClicked() ) );
	connect(btnUrlDefault, SIGNAL( clicked() ), this, SLOT( btnUrlDefaultClicked() ) );
	connect(btnUrlRemove, SIGNAL( clicked() ), this, SLOT( btnUrlRemoveClicked() ) );
	connect(leUrl, SIGNAL(returnPressed()), this, SLOT(btnUrlAddClicked()));
}

void CFGWadseekerSites::btnUrlAddClicked()
{
	insertUrl(leUrl->text());
}

void CFGWadseekerSites::btnUrlDefaultClicked()
{
	for (int i = 0; !Wadseeker::defaultSites[i].isEmpty(); ++i)
	{
		this->insertUrl(Wadseeker::defaultSites[i]);
	}
}

void CFGWadseekerSites::btnUrlRemoveClicked()
{
	QItemSelectionModel* selModel = lstUrls->selectionModel();
	QModelIndexList indexList = selModel->selectedIndexes();
	selModel->clear();

	QStandardItemModel* model = static_cast<QStandardItemModel*>(lstUrls->model());
	QList<QStandardItem*> itemList;
	for (int i = 0; i < indexList.count(); ++i)
	{
		itemList << model->itemFromIndex(indexList[i]);
	}

	for (int i = 0; i < itemList.count(); ++i)
	{
		QModelIndex index = model->indexFromItem(itemList[i]);
		model->removeRow(index.row());
	}
}

void CFGWadseekerSites::insertUrl(const QUrl& url)
{
	if (url.isEmpty())
	{
		return;
	}

	// first we check whether the URL is already in the box.
	QStandardItemModel* model = static_cast<QStandardItemModel*>(lstUrls->model());
	for (int i = 0; i < model->rowCount(); ++i)
	{
		QUrl existingUrl( model->item(i)->text() );
		if (existingUrl == url)
		{
			return;
		}
	}

	QStandardItem* it = new QStandardItem(url.toString());

	it->setDragEnabled(true);
	it->setDropEnabled(false);
	it->setToolTip(url.toString());

	model->appendRow(it);
}

void CFGWadseekerSites::readSettings()
{
	const QStringList& urlList = gConfig.wadseeker.searchURLs;
	foreach (const QString& url, urlList)
	{
		this->insertUrl(url);
	}
}

void CFGWadseekerSites::saveSettings()
{
	QStringList urlList;
	QStandardItemModel* model = static_cast<QStandardItemModel*>(lstUrls->model());
	for (int i = 0; i < model->rowCount(); ++i)
	{
		QUrl existingUrl( model->item(i)->text() );
		urlList << existingUrl.toString();
	}

	gConfig.wadseeker.searchURLs = urlList;
}
