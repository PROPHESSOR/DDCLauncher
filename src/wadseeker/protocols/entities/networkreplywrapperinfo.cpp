//------------------------------------------------------------------------------
// networkreplywrapperinfo.cpp
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
// Copyright (C) 2011 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "networkreplywrapperinfo.h"

#include "protocols/networkreplysignalwrapper.h"
#include "protocols/networkreplytimeouter.h"

NetworkReplyWrapperInfo::NetworkReplyWrapperInfo(QNetworkReply* pReply)
{
	this->pReply = pReply;

	if (pReply != NULL)
	{
		pSignalWrapper = new NetworkReplySignalWrapper(pReply);
		pTimeouter = new NetworkReplyTimeouter(pReply);
	}
	else
	{
		pSignalWrapper = NULL;
		pTimeouter = NULL;
	}
}

NetworkReplyWrapperInfo::~NetworkReplyWrapperInfo()
{
	if (pReply != NULL)
	{
		delete pSignalWrapper;
		delete pTimeouter;
		delete pReply;
	}
}

void NetworkReplyWrapperInfo::deleteMembersLater()
{
	if (pReply != NULL)
	{
		delete pSignalWrapper;
		delete pTimeouter;
		pReply->abort();
		pReply->deleteLater();

		pReply = NULL;
	}
}

void NetworkReplyWrapperInfo::setProgressTimeout(unsigned timeoutMsecs)
{
	if (pTimeouter != NULL)
	{
		pTimeouter->setProgressTimeout(timeoutMsecs);
	}
}

void NetworkReplyWrapperInfo::startConnectionTimeoutTimer(unsigned timeoutMsecs)
{
	if (pTimeouter != NULL)
	{
		pTimeouter->startConnectionTimeoutTimer(timeoutMsecs);
	}
}

bool NetworkReplyWrapperInfo::operator==(const NetworkReplyWrapperInfo& other) const
{
	return *this == other.pReply;
}

bool NetworkReplyWrapperInfo::operator==(const QNetworkReply* pReply) const
{
	return this->pReply == pReply;
}


