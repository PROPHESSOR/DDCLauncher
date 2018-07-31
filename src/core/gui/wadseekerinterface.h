//------------------------------------------------------------------------------
// wadseekerinterface.h
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
// Copyright (C) 2009 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef __WADSEEKERINTERFACE_H_
#define __WADSEEKERINTERFACE_H_

#include "serverapi/serverptr.h"
#include "wadseeker/wadseeker.h"
#include "dptr.h"
#include <QDialog>
#include <QStringList>
#include <QTimer>

class QModelIndex;

/**
 * @brief Wadseeker dialog box, only one instance is allowed.
 *
 * This is not a singleton, but create() methods will return NULL if instance
 * is already running. There's also isInstantiated() static method available.
 */
class WadseekerInterface : public QDialog
{
	Q_OBJECT

	public:
		static bool isInstantiated();

		static WadseekerInterface *create(QWidget* parent = NULL);
		static WadseekerInterface *create(ServerPtr server, QWidget* parent = NULL);
		static WadseekerInterface *createAutoNoGame(QWidget* parent = NULL);
		~WadseekerInterface();

		bool isAutomatic() { return bAutomatic; }

		void setCustomSite(const QString& site)
		{
			this->customSite = site;
		}

		/**
		 * @brief Sets WADs to seek.
		 *
		 * If window is automatic seek will start immediatelly. Otherwise
		 * WADs are inserted into the line edit.
		 *
		 * @param wads - wads to seek.
		 */
		void setWads(const QStringList& wads);

		Wadseeker& wadseekerRef() { return wadseeker; }

	private:
		enum States
		{
			Downloading = 0,
			Waiting = 1
		};

		DPtr<WadseekerInterface> d;

		static const int UPDATE_INTERVAL_MS;
		static WadseekerInterface *currentInstance;

		bool bAutomatic;
		bool bFirstShown;

		// Setup for customization in the future.
		QString colorHtmlMessageFatalError;
		QString colorHtmlMessageError;
		QString colorHtmlMessageNotice;

		QString customSite;

		/**
		 * Interface uses this instead of line edit if bAutomatic is true.
		 */
		QStringList seekedWads;

		States state;

		/**
		 * @brief A subset of seekedWads list. Contains all WADs that were
		 *        successfully installed.
		 */
		QStringList successfulWads;

		QTimer updateTimer;
		Wadseeker wadseeker;

		WadseekerInterface(QWidget* parent = NULL);
		WadseekerInterface(ServerPtr server, QWidget* parent = NULL);

		void connectWadseekerObject();
		void construct();
		void displayMessage(const QString& message, WadseekerLib::MessageType type, bool bPrependErrorsWithMessageType);
		void initMessageColors();

		/**
		 * @brief Sets default window title to default.
		 */
		void resetTitleToDefault();

		void setStateDownloading();
		void setStateWaiting();
		void setupAutomatic();
		void setupIdgames();
		void showEvent(QShowEvent* event);
		void startSeeking(const QStringList& seekedFilesList);
		void updateProgressBar();
		void updateTitle();

		/**
		 * @brief Subtracts successulWads list from seekedWads and returns the
		 *        difference.
		 */
		QStringList unsuccessfulWads() const;

	private slots:
		void abortService(const QString &service);
		void abortSite(const QUrl &url);
		void accept();
		void allDone(bool bSuccess);
		void fileDownloadSuccessful(const QString& filename);
		void reject();
		void message(const QString& message, WadseekerLib::MessageType type);
		void registerUpdateRequest();
		void seekStarted(const QStringList& filenames);
		void serviceStarted(const QString &service);
		void serviceFinished(const QString &service);
		void siteFinished(const QUrl& site);
		void siteProgress(const QUrl& site, qint64 bytes, qint64 total);
		void siteRedirect(const QUrl& oldUrl, const QUrl& newUrl);
		void siteStarted(const QUrl& site);
		void wadsTableRightClicked(const QModelIndex& index, const QPoint& cursorPosition);
};

#endif