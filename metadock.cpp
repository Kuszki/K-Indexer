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

#include "metadock.hpp"
#include "ui_metadock.h"

const QString MetaDock::wrongstyle = "text-decoration: underline wavy red;";

MetaDock::MetaDock(QSqlDatabase& db, QWidget *parent)
	: QDockWidget(parent)
	, ui(new Ui::MetaDock)
	, database(db)
{
	ui->setupUi(this);
}

MetaDock::~MetaDock(void)
{
	delete ui;
}

bool MetaDock::isChanged(void) const
{
	if (!model && model->rowCount() != 1) return false;
	else for (int i = 0; i < model->columnCount(); ++i)
	{
		auto index = model->index(0, i);
		auto data = model->data(index);

		if (values[i] != data) return true;
	}

	return false;
}

void MetaDock::dataChanged(void)
{
	if (!model || model->rowCount() != 1)
	{
		for (auto l : labels) l->setStyleSheet(QString()); return;
	}

	const auto row = model->record(0);

	for (int i = 0; i < indexes.size(); ++i)
	{
		const int id = indexes[i];

		const bool r = !row.field(id).value().toString().isEmpty() ||
					!(model->headerData(id, Qt::Horizontal, Qt::UserRole)
					  .toInt() & 0b001);

		if (r) labels[i]->setStyleSheet(QString());
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

	query.prepare("SELECT colindex, name, fill, hide, block FROM meta");

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
	mapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);

	const auto record = model->record();

	for (int i = 4; i < record.count(); ++i)
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
			case QVariant::Double:
			{
				auto spin = new QDoubleSpinBox(this);

				spin->setMinimum(-Q_INFINITY);
				spin->setMaximum(Q_INFINITY);

				widget = spin;
			}
			break;
			case QVariant::Int:
			{
				auto spin = new QSpinBox(this);

				spin->setMinimum(INT32_MIN);
				spin->setMaximum(INT32_MAX);

				widget = spin;
			}
			break;
			case QVariant::DateTime:
			{
				auto edit = new QDateTimeEdit(this);

				edit->setCalendarPopup(true);

				widget = edit;
			}
			break;
			case QVariant::String:
			{
				const int len = record.field(i).length() / 4;

				if (len > 64)
				{
					auto edit = new QPlainTextEdit(this);

					edit->setMaximumBlockCount(len > 0 ? len : -1);

					widget = edit;
				}
				else
				{
					auto edit = new QLineEdit(this);

					edit->setMaxLength(len > 0 ? len : -1);

					widget = edit;
				}
			}
			break;
			case QVariant::Bool:
				widget = new QCheckBox(this);
			break;
			case QVariant::Date:
				widget = new QDateEdit(this);
			break;
			case QVariant::Time:
				widget = new QTimeEdit(this);
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
	if (!model || !mapper || id == sheetID) return;
	else if (mapper->submit() && isChanged())
	{
		saveRecord();
	}

	QSqlQuery query(database);

	query.prepare("SELECT sheet FROM locks WHERE user = ? AND sheet = ?");
	query.addBindValue(userID);
	query.addBindValue(id);

	model->setFilter(QString("main.id = %1").arg(id));
	model->select();

	const bool ok = model->rowCount() == 1;

	locked = query.exec() && query.next();
	sheetID = ok ? id : 0;
	values.clear();

	if (ok)
	{
		for (int i = 0; i < model->columnCount(); ++i)
		{
			auto index = model->index(0, i);

			values.append(model->data(index));
		}

		mapper->toFirst();
		dataChanged();
	}

	lockWidgets(!(ok && locked));
}

bool MetaDock::saveRecord(void)
{
	if (!model || !sheetID || !locked) return false;
	else mapper->submit();

	if (!isChanged())
	{
		emit onRecordSave(0); return true;
	}

	if (userID > 0 || model->data(model->index(0, 2)).isNull())
	{
		model->setData(model->index(0, 2), userID);
		model->setData(model->index(0, 3), QDateTime::currentDateTimeUtc());
	}

	bool ok = model->submitAll();

	if (ok)
	{
		ok = model->select() && model->rowCount() == 1;

		if (!ok) lockWidgets(true);
		else
		{
			mapper->toFirst();
			values.clear();

			for (int i = 0; i < model->columnCount(); ++i)
			{
				auto index = model->index(0, i);

				values.append(model->data(index));
			}

			dataChanged();
		}
	}

	emit onRecordSave(ok ? 1 : -1, model->lastError().text());

	return ok;
}

void MetaDock::rollbackRecord(void)
{
	if (model) model->revertRow(0);
}

void MetaDock::lockRecord(int id)
{
	if (id == sheetID) lockWidgets(!(locked = true));
}

void MetaDock::unlockRecord(int id)
{
	if (id == sheetID) lockWidgets(!(locked = false));
}
