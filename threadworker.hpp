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

#ifndef THREADWORKER_HPP
#define THREADWORKER_HPP

#include <QtCore>
#include <QtSql>

class ThreadWorker : public QObject
{

		Q_OBJECT

	private:

		QSqlDatabase& database;

	public:

		explicit ThreadWorker(QSqlDatabase& db, QObject *parent = nullptr);
		virtual ~ThreadWorker(void) override;

	public slots:

		void exportData(const QString& path,
					 const QVariantList& users,
					 int status,
					 int validation,
					 int lock,
					 const QDateTime& from,
					 const QDateTime& to);

		void importData(const QString& path,
					 const QString& logs,
					 QVariantMap map,
					 bool header);

	signals:

		void onSetup(int, int);
		void onProgress(int);

		void onFinish(const QString&, int = 0);

};

#endif // THREADWORKER_HPP
