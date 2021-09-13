/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  K-Indexer : index documents in SQL database                            *
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

#ifndef ITEMSDOCK_HPP
#define ITEMSDOCK_HPP

#include <QtWidgets>
#include <QtCore>
#include <QtSql>

QT_BEGIN_NAMESPACE
namespace Ui { class ItemsDock; }
QT_END_NAMESPACE

class ItemsDock : public QDockWidget
{

		Q_OBJECT

	private:

		Ui::ItemsDock *ui;

		QSqlTableModel* model = nullptr;
		QSqlDatabase& database;

		QString limiter;

		int uidIndex = -1;
		int imgIndex = -1;

		int userID = 0;
		int sheetID = 0;

	public:

		explicit ItemsDock(QSqlDatabase& db, QWidget *parent = nullptr);
		virtual ~ItemsDock(void) override;

		void setFilter(const QString& filter);
		QString getFilter(void) const;

		void clearFilter(void);

	private slots:

		void selectionChanged(const QModelIndex& item);

	public slots:

		void setupDatabase(int user);
		void clearDatabase(void);

		void refreshList(void);

		void selectItem(int id);

		void selectNext(void);
		void selectPrevious(void);

	signals:

		void onFilterClicked(void);

		void onItemSelected(int);
		void onImageSelected(const QString&);

};

#endif // ITEMSDOCK_HPP
