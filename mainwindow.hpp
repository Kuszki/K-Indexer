/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  {description}                                                          *
 *  Copyright (C) 2020  Łukasz "Kuszki" Dróżdż  lukasz.kuszki@gmail.com    *
 *                                                                         *
 *  This program is free software: you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the  Free Software Foundation, either  version 3 of the  License, or   *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This  program  is  distributed  in the hope  that it will be useful,   *
 *  but WITHOUT ANY  WARRANTY;  without  even  the  implied  warranty of   *
 *  MERCHANTABILITY  or  FITNESS  FOR  A  PARTICULAR  PURPOSE.  See  the   *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have  received a copy  of the  GNU General Public License   *
 *  along with this program. If not, see http://www.gnu.org/licenses/.     *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QtWidgets>
#include <QtCore>
#include <QtSql>

#include "sqleditordialog.hpp"
#include "connectdialog.hpp"
#include "exportdialog.hpp"
#include "importdialog.hpp"

#include "imagedock.hpp"
#include "itemsdock.hpp"
#include "metadock.hpp"

#include "threadworker.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{

		Q_OBJECT

	private:

		Ui::MainWindow *ui;

		QThread* wthread;

		ImageDock* image;
		ItemsDock* items;
		MetaDock* meta;

		QSqlDatabase database;
		QString imgPath;

		int sheetID = -1;
		int userID = -1;

	public:

		explicit MainWindow(QWidget *parent = nullptr);
		virtual ~MainWindow(void) override;

	private slots:

		void openDatabase(const QString& driver,
					   const QString& server,
					   const QString& name,
					   const QString& user,
					   const QString& pass,
					   const QString& path);

		void connectActionClicked(void);
		void disconnectActionClicked(void);

		void lockActionClicked(void);
		void unlockActionClicked(void);
		void nextjobActionClicked(void);
		void commitActionClicked(void);

		void exportActionClicked(void);
		void importActionClicked(void);

		void queryActionClicked(void);

		void dockOptionsChanged(void);

		void recordIndexSelected(int index);

		void metaDataSaved(int ok, const QString& msg);

		void showInfoMessage(const QString& msg, const QString& title = QString());
		void showErrorMessage(const QString& msg, const QString& title = QString());
		void showWarningMessage(const QString& msg, const QString& title = QString());

		void performExport(const QString& path,
					    const QVariantList& users,
					    int status,
					    int validation,
					    int lock,
					    const QDateTime& from,
					    const QDateTime& to);

		void performImport(const QString& path,
					    const QString& logs,
					    const QVariantMap& map,
					    bool header);

		void finishExport(const QString& msg, int code);

		void finishImport(const QString& msg, int code);

	signals:

		void onDatabaseLogin(int);
		void onDatabaseLogout(void);
		void onDatabaseError(const QString&);

		void onImagepathUpdate(const QString&);

		void onDocumentLock(int);
		void onDocumentUnlock(int);

		void onExportRequest(const QString&,
						 const QVariantList&,
						 int, int, int,
						 const QDateTime&,
						 const QDateTime&);

		void onImportRequest(const QString&,
						 const QString&,
						 const QVariantMap&,
						 bool);

};

#endif // MAINWINDOW_HPP
