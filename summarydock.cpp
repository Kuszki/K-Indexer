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

#include "summarydock.hpp"
#include "ui_summarydock.h"

SummaryDock::SummaryDock(QSqlDatabase& db, QWidget *parent)
	: QDockWidget(parent)
	, ui(new Ui::SummaryDock)
	, database(db)
{
	ui->setupUi(this);
}

SummaryDock::~SummaryDock(void)
{
	delete ui;
}

void SummaryDock::setupDatabase(int user)
{
	clearDatabase();
	userID = user;
	refreshList();
}

void SummaryDock::clearDatabase(void)
{
	ui->lockedText->setText(tr("Unknown"));
	ui->doneText->setText(tr("Unknown"));
	ui->leftText->setText(tr("Unknown"));
	ui->validText->setText(tr("Unknown"));
	ui->invalidText->setText(tr("Unknown"));
	ui->todayText->setText(tr("Unknown"));
}

void SummaryDock::refreshList(void)
{
	QSqlQuery query(database);
	query.setForwardOnly(true);

	query.prepare("SELECT COUNT(sheet) FROM locks WHERE user = ?");
	query.addBindValue(userID);

	if (query.exec() && query.next())
		ui->lockedText->setText(QString::number(query.value(0).toInt()));
	else
		ui->lockedText->setText(tr("Unknown"));

	query.prepare("SELECT COUNT(id) FROM main WHERE user = ? AND time IS NOT NULL "
			    "AND id NOT IN (SELECT sheet FROM locks)");
	query.addBindValue(userID);

	if (query.exec() && query.next())
		ui->doneText->setText(QString::number(query.value(0).toInt()));
	else
		ui->doneText->setText(tr("Unknown"));

	query.prepare("SELECT COUNT(id) FROM main WHERE user IS NULL "
			    "AND id NOT IN (SELECT sheet FROM locks)");

	if (query.exec() && query.next())
		ui->leftText->setText(QString::number(query.value(0).toInt()));
	else
		ui->leftText->setText(tr("Unknown"));

	query.prepare("SELECT COUNT(id) FROM main WHERE user = ? AND time IS NOT NULL "
			    "AND id NOT IN (SELECT sheet FROM locks) "
			    "AND id NOT IN (SELECT id FROM invalid)");
	query.addBindValue(userID);

	if (query.exec() && query.next())
		ui->validText->setText(QString::number(query.value(0).toInt()));
	else
		ui->validText->setText(tr("Unknown"));

	query.prepare("SELECT COUNT(id) FROM main WHERE user = ? AND time IS NOT NULL "
			    "AND id NOT IN (SELECT sheet FROM locks) "
			    "AND id IN (SELECT id FROM invalid)");
	query.addBindValue(userID);

	if (query.exec() && query.next())
		ui->invalidText->setText(QString::number(query.value(0).toInt()));
	else
		ui->invalidText->setText(tr("Unknown"));

	query.prepare("SELECT COUNT(id) FROM main WHERE user = ? AND time IS NOT NULL "
			    "AND time BETWEEN ? AND ? "
			    "AND id NOT IN (SELECT sheet FROM locks)");
	query.addBindValue(userID);
	query.addBindValue(QDate::currentDate());
	query.addBindValue(QDate::currentDate().addDays(1));

	if (query.exec() && query.next())
		ui->todayText->setText(QString::number(query.value(0).toInt()));
	else
		ui->todayText->setText(tr("Unknown"));
}
