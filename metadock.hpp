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

#ifndef METADOCK_HPP
#define METADOCK_HPP

#include <QtWidgets>
#include <QtCore>
#include <QtSql>

QT_BEGIN_NAMESPACE
namespace Ui { class MetaDock; }
QT_END_NAMESPACE

class MetaDock : public QDockWidget
{

		Q_OBJECT

	private:

		static const QString wrongstyle;

		Ui::MetaDock *ui;

		QSqlRelationalTableModel* model = nullptr;
		QDataWidgetMapper* mapper = nullptr;
		QSqlDatabase& database;

		QVariantList values;

		QList<QLabel*> labels;
		QList<QWidget*> widgets;
		QList<int> indexes;

		int sheetID = 0;
		int userID = 0;

		bool locked = true;

	public:

		explicit MetaDock(QSqlDatabase& db, QWidget *parent = nullptr);
		virtual ~MetaDock(void) override;

		bool isChanged(void) const;

	private slots:

		void dataChanged(void);
		void lockWidgets(bool lock);

	public slots:

		void setupDatabase(int user);
		void clearDatabase(void);

		void setupRecord(int id);

		bool saveRecord(void);
		void rollbackRecord(void);

		void lockRecord(int id);
		void unlockRecord(int id);

	signals:

		void onRecordSave(int, const QString& = QString());

};

#endif // METADOCK_HPP
