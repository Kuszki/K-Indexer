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

#include "connectdialog.hpp"
#include "imagewidget.hpp"
#include "itemsdock.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{

		Q_OBJECT

	private:

		Ui::MainWindow *ui;

		ItemsDock* items;

		QSqlDatabase database;
		QString imgPath;

		int userID = 0;

	public:

		MainWindow(QWidget *parent = nullptr);
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

		void dockOptionsChanged(void);

	signals:

		void onDatabaseLogin(int);
		void onDatabaseError(const QString&);

};

#endif // MAINWINDOW_HPP
