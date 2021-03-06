//------------------------------------------------------------------------------
// urlopener.cpp
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
// Copyright (C) 2014 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "urlopener.h"

#include <QDesktopServices>
#include <QQueue>
#include <QTimer>

class UrlOpener::PrivData
{
public:
	QQueue<QUrl> queue;
};

UrlOpener *UrlOpener::inst = NULL;

UrlOpener::UrlOpener()
{
	d = new PrivData();
}

void UrlOpener::doOpen()
{
	if (!d->queue.isEmpty())
	{
		QDesktopServices::openUrl(d->queue.dequeue());
	}
}

UrlOpener *UrlOpener::instance()
{
	if (inst == NULL)
	{
		inst = new UrlOpener();
	}
	return inst;
}

void UrlOpener::open(const QUrl &url)
{
	d->queue.enqueue(url);
	QTimer::singleShot(0, this, SLOT(doOpen()));
}
