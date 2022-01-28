/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  {description}                                                          *
 *  Copyright (C) 2022  Łukasz "Kuszki" Dróżdż  lukasz.kuszki@gmail.com    *
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

#ifndef ITEMMODEL_HPP
#define ITEMMODEL_HPP

#include <QtWidgets>
#include <QtCore>
#include <QtSql>
#include <QtGui>

class ItemModel : public QSqlTableModel
{

		Q_OBJECT

	protected:

		QHash<Qt::GlobalColor, QSet<int>> colors;
		QHash<Qt::GlobalColor, QBrush> brushes;

		QBrush defBrush;
		int idIndex = 0;

	public:

		explicit ItemModel(QObject *parent = nullptr,
		                   QSqlDatabase db = QSqlDatabase());
		virtual ~ItemModel(void) override;

		virtual void setTable(const QString& tableName) override;

		virtual QVariant data(const QModelIndex &idx,
		                      int role = Qt::DisplayRole) const override;

		void setColor(Qt::GlobalColor color, const QSet<int> items);

		void setTable(const QString& tableName,
		              const QString& idField);

		void clearColors(void);

		QBrush getColor(const QModelIndex& item) const;
		QBrush getColor(int id) const;

	protected:

		void updateItemsColors(const QSet<int>& updates);

};

#endif // ITEMMODEL_HPP
