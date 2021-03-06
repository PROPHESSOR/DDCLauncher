//------------------------------------------------------------------------------
// datastreamoperatorwrapper.h
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
// Copyright (C) 2012 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef DOOMSEEKER_DATASTREAMOPERATORWRAPPER_H
#define DOOMSEEKER_DATASTREAMOPERATORWRAPPER_H

#include <QDataStream>

#include "global.h"

/**
 * @ingroup group_pluginapi
 * @brief Wraps around QDataStream stream operators to provide cleaner
 *        reading interface.
 */
class MAIN_EXPORT DataStreamOperatorWrapper
{
	public:
		DataStreamOperatorWrapper(QDataStream* stream);
		virtual ~DataStreamOperatorWrapper();

		/**
		 * @brief Gets underlying QDataStream.
		 */
		QDataStream* dataStream();

		/**
		 * @brief Gets underlying QDataStream.
		 */
		const QDataStream* dataStream() const;

		/**
		 * @brief Returns true if there is any data remaining in the stream.
		 *
		 * The check is performed through 'remaining() > 0' instruction.
		 */
		bool hasRemaining() const
		{
			return remaining() > 0;
		}

		qint8 readQInt8();
		bool readBool();
		quint8 readQUInt8();
		quint16 readQUInt16();
		qint16 readQInt16();
		quint32 readQUInt32();
		qint32 readQInt32();
		quint64 readQUInt64();
		qint64 readQInt64();
		float readFloat();
		double readDouble();

		/**
		 * @brief Reads specified amount of raw bytes.
		 *
		 * @param length
		 *     Amount of bytes to read.
		 * @return QByteArray of read bytes.
		 */
		QByteArray readRaw(qint64 length);

		/**
		 * @brief Reads all remaining raw bytes.
		 *
		 * This calls QIODevice::readAll() directly.
		 *
		 * @return QByteArray of read bytes.
		 */
		QByteArray readRawAll();

		/**
		 * @brief This method calls Strings::readUntilByte() .
		 */
		QByteArray readRawUntilByte(char stopByte);

		/**
		 * @brief Returns a remaining amount of bytes from the underlying
		 * QIODevice.
		 *
		 * This is done by performing `QIODevice::size() - QIODevice::pos()`.
		 * It is assumed that the QIODevice can return both values, ie. is not
		 * sequential.
		 */
		qint64 remaining() const;

		/**
		 * @brief Reads and discards specified amount of data.
		 *
		 * Calls QIODevice::skipRawData() directly.
		 *
		 * @return Number of bytes actually skipped or -1 on error.
		 */
		int skipRawData(int len);

	private:
		class PrivData;
		PrivData *d;
};

#endif

