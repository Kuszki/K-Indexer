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

#include "metadock.hpp"
#include "ui_metadock.h"

const QString MetaDock::wrongstyle = "text-decoration: underline wavy red;";

MetaDock::MetaDock(QSqlDatabase& db, QWidget *parent) :
	QDockWidget(parent),
	ui(new Ui::MetaDock),
	database(db)
{
	ui->setupUi(this);
}

MetaDock::~MetaDock(void)
{
	delete ui;
}

void MetaDock::dataChanged(void)
{
	const auto row = model->record(0);

	for (int i = 0; i < indexes.size(); ++i)
	{
		const int id = indexes[i];

		const bool v = row.field(id).isValid();
		const bool r = !row.field(id).value().toString().isEmpty() ||
					!(model->headerData(id, Qt::Horizontal, Qt::UserRole)
					  .toInt() & 0b001);

		if (v && r) labels[i]->setStyleSheet(QString());
		else labels[i]->setStyleSheet(wrongstyle);
	}
}

void MetaDock::lockWidgets(bool lock)
{
	for (int i = 0; i < indexes.size(); ++i)
	{
		const int id = indexes[i];

		const bool r = model->headerData(id, Qt::Horizontal, Qt::UserRole).toInt() & 0b100;

		widgets[i]->setEnabled(!lock && !r);
	}
}

void MetaDock::setupDatabase(int user)
{
	clearDatabase(); userID = user;

	model = new QSqlRelationalTableModel(this, database);
	model->setJoinMode(QSqlRelationalTableModel::LeftJoin);
	model->setEditStrategy(QSqlTableModel::OnManualSubmit);
	model->setTable("main");
	model->setFilter("main.id = 0");

	QSqlQuery query(database);

	query.prepare("SELECT colindex, srctable, srckey, srcval FROM joins");

	if (query.exec()) while (query.next())
	{
		model->setRelation(query.value(0).toInt(),
					    QSqlRelation(query.value(1).toString(),
								  query.value(2).toString(),
								  query.value(3).toString()));
	}

	query.prepare("SELECT colindex, text, fill, hide, read FROM meta");

	if (query.exec()) while (query.next())
	{
		const QString text = query.value(1).toString();
		const int flags = (query.value(2).toBool() << 0) |
					   (query.value(3).toBool() << 1) |
					   (query.value(4).toBool() << 2);

		if (!text.isEmpty()) model->setHeaderData(query.value(0).toInt(),
										  Qt::Horizontal, text);

		model->setHeaderData(query.value(0).toInt(),
						 Qt::Horizontal, flags, Qt::UserRole);
	}

	mapper = new QDataWidgetMapper(this);
	mapper->setModel(model);
	mapper->setItemDelegate(new QSqlRelationalDelegate(mapper));

	const auto record = model->record();

	for (int i = 3; i < record.count(); ++i)
	{
		if (model->headerData(i, Qt::Horizontal, Qt::UserRole).toInt() & 0b010) continue;

		QLabel* label = new QLabel(model->headerData(i, Qt::Horizontal).toString(), this);
		QWidget* widget = nullptr;

		QSqlTableModel* rel = model->relationModel(i);

		if (rel)
		{
			const int col = rel->fieldIndex(model->relation(i).displayColumn());
			QComboBox* combo = new QComboBox(this);

			combo->setModel(rel);
			combo->setModelColumn(col);

			widget = combo;
		}
		else switch (record.field(i).type())
		{
			case QVariant::Bool:
				widget = new QCheckBox(this);
			break;
			case QVariant::Double:
				widget = new QDoubleSpinBox(this);
			break;
			case QVariant::Int:
				widget = new QSpinBox(this);
			break;
			case QVariant::Date:
				widget = new QDateEdit(this);
			break;
			case QVariant::DateTime:
				widget = new QDateTimeEdit(this);
			break;
			default:
				widget = new QLineEdit(this);
		}

		mapper->addMapping(widget, i);

		ui->formLayout->addRow(label, widget);
		widget->setEnabled(false);

		labels.append(label);
		widgets.append(widget);
		indexes.append(i);
	}

	connect(model, &QSqlRelationalTableModel::dataChanged,
		   this, &MetaDock::dataChanged);
}

void MetaDock::clearDatabase(void)
{
	if (model) saveRecord();

	for (auto w : labels) w->deleteLater();
	for (auto w : widgets) w->deleteLater();

	if (model) model->deleteLater();
	if (mapper) mapper->deleteLater();

	model = nullptr;
	mapper = nullptr;

	labels.clear();
	widgets.clear();
	indexes.clear();
}

void MetaDock::setupRecord(int id)
{
	if (!model) return;
	else if (model->isDirty())
	{
		saveRecord();
	}

	QSqlQuery query(database);

	query.prepare("SELECT sheet FROM locks WHERE user = ? AND sheet = ?");
	query.addBindValue(userID);
	query.addBindValue(id);

	const bool locked = userID == 0 || (query.exec() && query.next());

	model->setFilter(QString("main.id = %1").arg(id));
	model->select();

	const bool ok = model->rowCount() == 1;

	if (ok) mapper->toFirst();

	sheetID = ok ? id : 0;

	lockWidgets(!(ok && locked));
	dataChanged();
}

void MetaDock::saveRecord(void)
{
	if (!model || !sheetID) return;

	if (model->isDirty()) model->setData(model->index(0, 2), userID);

	if (model->submitAll()) emit onRecordSave(sheetID, true, QString());
	else emit onRecordSave(sheetID, false, model->lastError().text());
}

void MetaDock::rollbackRecord(void)
{
	if (model) model->revertRow(0);
}
