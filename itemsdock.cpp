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

#include "itemsdock.hpp"
#include "ui_itemsdock.h"

ItemsDock::ItemsDock(QSqlDatabase& db, QWidget *parent)
     : QDockWidget(parent)
     , ui(new Ui::ItemsDock)
     , database(db)
{
	ui->setupUi(this);

	ui->listView->setEnabled(false);
	ui->typesCombo->setEnabled(false);
	ui->searchEdit->setEnabled(false);
	ui->refreshButton->setEnabled(false);

	ui->listView->model()->deleteLater();
	ui->listView->selectionModel()->deleteLater();

	connect(ui->typesCombo, qOverload<int>(&QComboBox::currentIndexChanged),
	        this, &ItemsDock::refreshList);

	connect(ui->searchEdit, &QLineEdit::editingFinished,
	        this, &ItemsDock::refreshList);

	connect(ui->refreshButton, &QToolButton::clicked,
	        this, &ItemsDock::refreshList);

	connect(ui->clearButton, &QToolButton::clicked,
	        this, &ItemsDock::clearFilter);

	connect(ui->searchButton, &QToolButton::clicked,
	        this, &ItemsDock::onFilterClicked);
}

ItemsDock::~ItemsDock(void)
{
	delete ui;
}

void ItemsDock::setFilter(const QString& filter)
{
	const QString old = limiter;

	if (filter.isEmpty()) limiter.clear();
	else limiter = filter.simplified();

	if (old != limiter) refreshList();
}

QString ItemsDock::getFilter(void) const
{
	return limiter;
}

QSet<int> ItemsDock::getLocked(void)
{
	QSqlQuery query(database);
	QSet<int> values;

	query.setForwardOnly(true);
	query.prepare("SELECT sheet FROM locks WHERE user = :usr OR :usr = 0");
	query.bindValue(":usr", userID);

	if (query.exec()) while (query.next())
	{
		values.insert(query.value(0).toInt());
	}

	return values;
}

QSet<int> ItemsDock::getDone(void)
{
	QSqlQuery query(database);
	QSet<int> values;

	query.setForwardOnly(true);
	query.prepare("SELECT id FROM main WHERE user IS NOT NULL AND (user = :usr OR :usr = 0)");
	query.bindValue(":usr", userID);

	if (query.exec()) while (query.next())
	{
		values.insert(query.value(0).toInt());
	}

	return values;
}

QSet<int> ItemsDock::getInvalid(void)
{
	QSqlQuery query(database);
	QSet<int> values;

	query.setForwardOnly(true);
	query.prepare("SELECT id FROM invalid WHERE user IS NOT NULL AND (user = :usr OR :usr = 0)");
	query.bindValue(":usr", userID);

	if (query.exec()) while (query.next())
	{
		values.insert(query.value(0).toInt());
	}

	return values;
}

void ItemsDock::clearFilter(void)
{
	setFilter(QString());
}

bool ItemsDock::isColors(void) const
{
	return doColor;
}

void ItemsDock::selectionChanged(const QModelIndex& item)
{
	if (!model || !item.isValid()) return;

	const auto indexI = model->index(item.row(), uidIndex, item.parent());
	const auto indexP = model->index(item.row(), imgIndex, item.parent());

	ui->listView->selectionModel()->select(indexI,
	                    QItemSelectionModel::ClearAndSelect |
	                    QItemSelectionModel::Rows);

	sheetID = model->data(indexI).toInt();

	emit onItemSelected(model->data(indexI).toInt());
	emit onImageSelected(model->data(indexP).toString());
}

void ItemsDock::setupDatabase(int user)
{
	clearDatabase(); userID = user;

	ui->typesCombo->setCurrentIndex(0);
	ui->searchEdit->clear();

	model = new ItemModel(this, database);
	model->setTable("main");
	model->select();

	const QSqlRecord rec = model->record();
	uidIndex = rec.indexOf("id");
	imgIndex = rec.indexOf("path");

	ui->listView->setModel(model);
	ui->listView->setModelColumn(imgIndex);

	ui->listView->setEnabled(true);
	ui->typesCombo->setEnabled(true);
	ui->searchEdit->setEnabled(true);
	ui->refreshButton->setEnabled(true);
	ui->clearButton->setEnabled(true);
	ui->searchButton->setEnabled(true);

	connect(ui->listView->selectionModel(),
	        &QItemSelectionModel::currentRowChanged,
	        this, &ItemsDock::selectionChanged);

	applyColors();
}

void ItemsDock::clearDatabase(void)
{
	ui->listView->setEnabled(false);
	ui->typesCombo->setEnabled(false);
	ui->searchEdit->setEnabled(false);
	ui->refreshButton->setEnabled(false);
	ui->clearButton->setEnabled(false);
	ui->searchButton->setEnabled(false);

	if (model)
	{
		ui->listView->clearSelection();
		ui->listView->selectionModel()->deleteLater();
		ui->listView->model()->deleteLater();

		model = nullptr;
	}

	userID = sheetID = -1;
	limiter.clear();
}

void ItemsDock::refreshList(void)
{
	if (!model) return;

	const QString test = ui->searchEdit->text().trimmed().replace('\'', "\'\'");
	const int index = ui->typesCombo->currentIndex();

	QStringList filter;

	if (!test.isEmpty())
		filter.append(QString("path LIKE '%%1%'").arg(test));

	if (!limiter.isEmpty())
		filter.append(QString("(%1)").arg(limiter));

	if (index == 1)
		filter.append(QString("id NOT IN (SELECT sheet FROM locks) AND (user = %1 OR %1 = 0) AND user IS NOT NULL").arg(userID));
	else if (index == 2)
		filter.append(QString("id IN (SELECT sheet FROM locks WHERE user = %1 OR %1 = 0)").arg(userID));
	else if (index == 3)
		filter.append(QString("id IN (SELECT id FROM invalid) AND (user = %1 OR %1 = 0) AND user IS NOT NULL").arg(userID));
	else if (index == 4)
		filter.append(QString("id IN (SELECT sheet FROM locks WHERE user = %1 OR %1 = 0) OR ((user = %1 OR %1 = 0) AND user IS NOT NULL)").arg(userID));
	else if (index == 5)
		filter.append(QString("id NOT IN (SELECT sheet FROM locks) AND user IS NULL"));

	setUpdatesEnabled(false);

	model->setFilter(filter.join(" AND "));
	model->select();

	if (sheetID) selectItem(sheetID);
	if (doColor) applyColors();

	setUpdatesEnabled(true);
}

void ItemsDock::applyColors(void)
{
	if (!model) return;

	const auto locked = getLocked();
	const auto invalid = getInvalid() - locked;
	const auto done = getDone() - locked - invalid;

	model->setColor(Qt::yellow, locked);
	model->setColor(Qt::red, invalid);
	model->setColor(Qt::green, done);
}

void ItemsDock::removeColors(void)
{
	if (model) model->clearColors();
}

void ItemsDock::selectItem(int id)
{
	if (model) ui->listView->selectionModel()->clearSelection();
	else return;

	if (id > 0) for (int i = 0; i < model->rowCount(); ++i)
	{
		const auto in = model->index(i, uidIndex);

		if (model->data(in).toInt() == id)
		{
			ui->listView->selectionModel()->select(in,
			     QItemSelectionModel::ClearAndSelect |
			     QItemSelectionModel::Rows);

			break;
		}
		else if (i == model->rowCount() - 1)
		{
			if (model->canFetchMore())
			{
				model->fetchMore();
			}
		}
	}

	sheetID = id;
}

void ItemsDock::selectNext(void)
{
	if (!model || !model->rowCount()) return;

	const auto index = ui->listView->selectionModel()->selectedRows().value(0);
	int row = 0;

	if (index.isValid()) row = (index.row() + 1) < model->rowCount() ?
	                                index.row() + 1 : 0;

	const auto indexI = model->index(row, uidIndex);
	const auto indexP = model->index(row, imgIndex);

	ui->listView->selectionModel()->select(indexI,
	                    QItemSelectionModel::ClearAndSelect |
	                    QItemSelectionModel::Rows);

	sheetID = model->data(indexI).toInt();

	emit onItemSelected(model->data(indexI).toInt());
	emit onImageSelected(model->data(indexP).toString());
}

void ItemsDock::selectPrevious(void)
{
	if (!model || !model->rowCount()) return;

	const auto index = ui->listView->selectionModel()->selectedRows().value(0);
	int row = model->rowCount() - 1;

	if (index.isValid()) row = (index.row() - 1) < 0 ?
	                                model->rowCount() - 1 :
	                                index.row() - 1;

	const auto indexI = model->index(row, uidIndex);
	const auto indexP = model->index(row, imgIndex);

	ui->listView->selectionModel()->select(indexI,
	                    QItemSelectionModel::ClearAndSelect |
	                    QItemSelectionModel::Rows);

	sheetID = model->data(indexI).toInt();

	emit onItemSelected(model->data(indexI).toInt());
	emit onImageSelected(model->data(indexP).toString());
}

void ItemsDock::setColors(bool enable)
{
	if (doColor == enable) return;
	else doColor = enable;

	if (doColor) applyColors();
	else removeColors();
}
