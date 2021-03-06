//------------------------------------------------------------------------------
// gameexefactory.cpp
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
// Copyright (C) 2013 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "gameexefactory.h"

#include "plugins/engineplugin.h"
#include "serverapi/exefile.h"

class GameExeFactory::PrivData
{
public:
	EnginePlugin* plugin;

	ExeFile* (GameExeFactory::*offline)();
	ExeFile* (GameExeFactory::*server)();
};

GameExeFactory::GameExeFactory(EnginePlugin* plugin)
{
	d = new PrivData();
	d->plugin = plugin;

	set_offline(&GameExeFactory::offline_default);
	set_server(&GameExeFactory::server_default);
}

GameExeFactory::~GameExeFactory()
{
	delete d;
}

POLYMORPHIC_DEFINE(ExeFile*, GameExeFactory, offline, (), ());
POLYMORPHIC_DEFINE(ExeFile*, GameExeFactory, server, (), ());

EnginePlugin* GameExeFactory::plugin()
{
	return d->plugin;
}

ExeFile* GameExeFactory::offline_default()
{
	ExeFile *f = new ExeFile();
	f->setProgramName(d->plugin->data()->name);
	f->setExeTypeName(tr("offline"));
	f->setConfigKey("BinaryPath");
	return f;
}

ExeFile* GameExeFactory::server_default()
{
	ExeFile *f = new ExeFile();
	f->setProgramName(d->plugin->data()->name);
	f->setExeTypeName(tr("server"));
	if (d->plugin->data()->clientOnly)
	{
		f->setConfigKey("BinaryPath");
	}
	else
	{
		f->setConfigKey("ServerBinaryPath");
	}
	return f;
}
