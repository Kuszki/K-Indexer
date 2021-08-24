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

#include "itemsdock.hpp"
#include "ui_itemsdock.h"

ItemsDock::ItemsDock(QSqlDatabase& db, QWidget *parent) :
	QDockWidget(parent),
	ui(new Ui::ItemsDock),
	database(db)
{
	ui->setupUi(this);

	connect(ui->typesCombo, qOverload<int>(&QComboBox::currentIndexChanged),
		   this, &ItemsDock::refreshList);

	connect(ui->searchEdit, &QLineEdit::editingFinished,
		   this, &ItemsDock::refreshList);

	connect(ui->refreshButton, &QToolButton::clicked,
		   this, &ItemsDock::refreshList);

}

ItemsDock::~ItemsDock(void)
{
	delete ui;
}

void ItemsDock::setupDatabase(int user)
{
	if (!user) { model->clear(); return; }

	ui->typesCombo->setCurrentIndex(0);
	ui->searchEdit->clear();

	auto oldModel = ui->listView->model();
	auto oldSel = ui->listView->selectionModel();

	model = new QSqlTableModel(this, database);
	model->setTable("main");
	model->select();

	ui->listView->setModel(model);
	ui->listView->setModelColumn(1);

	oldModel->deleteLater();
	oldSel->deleteLater();

	userID = user;
}

void ItemsDock::clearDatabase(void)
{
	model->clear();
}

void ItemsDock::refreshList(void)
{
	const QString test = ui->searchEdit->text().trimmed().replace('\'', "\'\'");
	const int index = ui->typesCombo->currentIndex();

	QStringList filter;

	if (!test.isEmpty())
		filter.append(QString("path LIKE '%%1%'").arg(test));

	if (index == 1)
		filter.append(QString("id IN (SELECT sheet FROM locks WHERE user = %1)").arg(userID));
	else if (index == 2)
		filter.append(QString("user = %1").arg(userID));

	model->setFilter(filter.join(" AND "));
	model->select();
}
