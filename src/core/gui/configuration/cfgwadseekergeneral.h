//------------------------------------------------------------------------------
// cfgwadseekergeneral.h
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
#ifndef __WADSEEKERCONFIG_GENERAL_H_
#define __WADSEEKERCONFIG_GENERAL_H_

#include "ui_cfgwadseekergeneral.h"
#include "gui/configuration/configurationbasebox.h"

class CFGWadseekerGeneral : public ConfigurationBaseBox, private Ui::CFGWadseekerGeneral
{
	Q_OBJECT

	public:
		CFGWadseekerGeneral(QWidget* parent = NULL);

		QIcon icon() const { return QIcon(":/icons/preferences-system-4.png"); }
		QString name() const { return tr("General"); }
		void readSettings();
		QString title() const { return tr("Wadseeker - General"); }

	protected:
		void fillTargetDirectoryComboBox();
		void saveSettings();
};

#endif
