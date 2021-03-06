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

#ifndef SUMMARYDOCK_HPP
#define SUMMARYDOCK_HPP

#include <QtWidgets>
#include <QtCore>
#include <QtSql>

QT_BEGIN_NAMESPACE
namespace Ui { class SummaryDock; }
QT_END_NAMESPACE

class SummaryDock : public QDockWidget
{

		Q_OBJECT

	private:

		Ui::SummaryDock *ui;

		QSqlDatabase& database;

		int userID = 0;

	public:

		explicit SummaryDock(QSqlDatabase& db, QWidget *parent = nullptr);
		virtual ~SummaryDock(void) override;

	public slots:

		void setupDatabase(int user);
		void clearDatabase(void);

		void refreshList(void);

};

#endif // SUMMARYDOCK_HPP
