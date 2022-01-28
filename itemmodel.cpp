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

#include "itemmodel.hpp"

ItemModel::ItemModel(QObject* parent, QSqlDatabase db)
: QSqlTableModel(parent, db)
{
	defBrush = QApplication::palette("QStandardItem").windowText();
}

ItemModel::~ItemModel(void) {}

void ItemModel::setTable(const QString& tableName)
{
	QSqlTableModel::setTable(tableName);
	idIndex = record().indexOf("id");
}

QVariant ItemModel::data(const QModelIndex& idx, int role) const
{
	if (role == Qt::ForegroundRole) return getColor(idx);
	else return QSqlTableModel::data(idx, role);
}

void ItemModel::setColor(Qt::GlobalColor color, const QSet<int> items)
{
	const QSet<int> updates = colors.value(color, QSet<int>()) + items;

	if (items.isEmpty())
	{
		colors.remove(color);
		brushes.remove(color);
	}
	else
	{
		colors[color] = items;
		brushes[color] = QBrush(color);
	}

	updateItemsColors(updates);
}

void ItemModel::setTable(const QString& tableName, const QString& idField)
{
	QSqlTableModel::setTable(tableName);
	idIndex = record().indexOf(idField);
}

void ItemModel::clearColors(void)
{
	QSet<int> updates;

	for (const auto& i : colors) updates += i;

	colors.clear();
	brushes.clear();

	updateItemsColors(updates);
}

QBrush ItemModel::getColor(const QModelIndex& item) const
{
	if (!item.isValid()) return defBrush;

	const auto i = index(item.row(), idIndex, item.parent());

	return getColor(data(i).toInt());
}

QBrush ItemModel::getColor(int id) const
{
	for (auto i = colors.cbegin(); i != colors.cend(); ++i)
	{
		if (i.value().contains(id)) return brushes[i.key()];
	}

	return defBrush;
}

void ItemModel::updateItemsColors(const QSet<int>& updates)
{
	const QVector<int> changes = { Qt::ForegroundRole };
	const int cc = columnCount();

	for (int i = 0; i < rowCount(); ++i)
	{
		const auto in = index(i, idIndex);

		if (updates.contains(data(in).toInt()))
		{
			const auto start = index(i, 0);
			const auto stop = index(i, cc);

			emit dataChanged(start, stop, changes);
		}
	}
}
