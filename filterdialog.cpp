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

#include "filterdialog.hpp"
#include "ui_filterdialog.h"

FilterDialog::FilterDialog(QSqlDatabase& db, QWidget* Parent)
	: QDialog(Parent)
	, ui(new Ui::FilterDialog)
	, database(db)
{
	ui->setupUi(this); new SqlHighlighter(ui->advancedEdit->document());

	ui->advancedEdit->setFont(QFont("monospace"));

	ui->simpleLayout->setAlignment(Qt::AlignTop);
	ui->queryLayout->setAlignment(Qt::AlignTop);

	connect(ui->buttonBox->button(QDialogButtonBox::Reset), &QPushButton::clicked,
		   this, &FilterDialog::resetButtonClicked);

	connect(ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked,
		   this, &FilterDialog::accept);
}

FilterDialog::~FilterDialog(void)
{
	delete ui;
}

QString FilterDialog::getFilterRules(void) const
{
	const QString join = ui->operatorBox->currentText();
	QStringList Rules;

	for (int i = 0; i < ui->simpleLayout->count(); ++i)
	{
		if (auto W = qobject_cast<FilterWidget*>(ui->simpleLayout->itemAt(i)->widget()))
		{
			if (W->isChecked()) Rules.append(W->getCondition());
		}
	}

	if (Rules.isEmpty()) return QString();
	else return Rules.join(QString(" %1 ").arg(join));
}

QString FilterDialog::getAdvancedRules(void) const
{
	return ui->advancedEdit->document()->toPlainText().trimmed();
}

QString FilterDialog::getQueryRules(void) const
{
	const QString format = "id IN (SELECT id FROM %1)";
	const QString join = ui->operatorBox->currentText();
	QStringList Rules;

	for (int i = 0; i < ui->queryLayout->count(); ++i)
	{
		if (auto W = qobject_cast<QCheckBox*>(ui->queryLayout->itemAt(i)->widget()))
		{
			if (W->isChecked()) Rules.append(format.arg(W->toolTip()));
		}
	}

	return Rules.join(QString(" %1 ").arg(join));
}

QVariantMap FilterDialog::getFieldsRules(void) const
{
	QVariantMap Rules;

	for (int i = 0; i < ui->simpleLayout->count(); ++i)
	{
		if (auto W = qobject_cast<FilterWidget*>(ui->simpleLayout->itemAt(i)->widget()))
		{
			if (W->isChecked()) Rules.insert(W->objectName(), W->getValue());
		}
	}

	return Rules;
}

void FilterDialog::resetButtonClicked(void)
{
	for (int i = 0; i < ui->queryLayout->count(); ++i)
		if (auto W = qobject_cast<QCheckBox*>(ui->queryLayout->itemAt(i)->widget()))
		{
			W->setChecked(false);
		}

	for (int i = 0; i < ui->simpleLayout->count(); ++i)
		if (auto W = qobject_cast<FilterWidget*>(ui->simpleLayout->itemAt(i)->widget()))
		{
			W->reset();
		}

	ui->advancedEdit->clear();
}

void FilterDialog::helperIndexChanged(int Index)
{
	auto Model = ui->helperCombo->model();

	if (Model && Index != -1)
	{
		auto Root = Model->index(Index, 0);

		ui->variablesList->setRootIndex(Root);
	}
}

void FilterDialog::tooltipShowRequest(const QModelIndex& Index)
{
	ui->helpLabel->setText(ui->variablesList->model()->data(Index, Qt::ToolTipRole).toString());
}


void FilterDialog::variablePasteRequest(const QModelIndex& Index)
{
	const QString Value = ui->variablesList->model()->data(Index).toString();

	if (!Value.isEmpty()) ui->advancedEdit->insertPlainText(Value);
}

void FilterDialog::accept(void)
{
	const QString join = ui->operatorBox->currentText();
	QStringList rules;

	rules.append(getFilterRules());
	rules.append(getAdvancedRules());
	rules.append(getQueryRules());

	rules.removeAll(QString());

	QDialog::accept();

	emit onFiltersUpdate(rules.join(QString(" %1 ").arg(join)));
}

void FilterDialog::setFields(const QVariantList& list, const QVariantList& queries)
{
	while (auto I = ui->queryLayout->takeAt(0)) if (auto W = I->widget()) W->deleteLater();
	while (auto I = ui->simpleLayout->takeAt(0)) if (auto W = I->widget()) W->deleteLater();

	for (const auto& field : list)
	{
		ui->simpleLayout->addWidget(new FilterWidget(field.toMap(), this));
	}

	for (const auto& field : queries)
	{
		QVariantMap map = field.toMap();
		QCheckBox* box = new QCheckBox(this);

		box->setText(map.value("label").toString());
		box->setToolTip(map.value("name").toString());

		ui->queryLayout->addWidget(box);
	}

	auto newModel = SqlHighlighter::getSqlHelperModel(this, list);

	ui->variablesList->model()->deleteLater();
	ui->variablesList->selectionModel()->deleteLater();

	ui->helperCombo->setModel(newModel);
	ui->variablesList->setModel(newModel);
	ui->variablesList->setRootIndex(newModel->index(0, 0));

	connect(ui->variablesList->selectionModel(),
		   &QItemSelectionModel::currentChanged,
		   this, &FilterDialog::tooltipShowRequest);
}

void FilterDialog::setupDialog(int user)
{
	const QSqlRecord record = database.record("main");

	const int start = user ? 3 : 2;

	QMap<int, QVariantMap> fields;
	QHash<QString, QString> names;
	QVariantList list, queries;

	for (int i = start; i < record.count(); ++i)
	{
		QVariantMap map;

		map["name"] = QString(record.fieldName(i));
		map["type"] = int(record.field(i).type());

		fields.insert(i, map);
	}

	QSqlQuery query(database), select(database);

	query.setForwardOnly(true);
	select.setForwardOnly(true);

	query.prepare("SELECT colindex, srctable, srckey, srcval FROM joins");

	if (query.exec()) while (query.next()) if (fields.contains(query.value(0).toInt()))
	{
		QVariantMap map;

		select.prepare(QString("SELECT %1, %2 FROM %3")
					.arg(query.value(2).toString())
					.arg(query.value(3).toString())
					.arg(query.value(1).toString()));

		if (select.exec()) while (select.next())
		{
			map.insert(select.value(1).toString(),
					 select.value(0));
		}

		fields[query.value(0).toInt()]["dict"] = map;
	}

	query.prepare("SELECT colindex, name FROM meta");

	if (query.exec()) while (query.next()) if (fields.contains(query.value(0).toInt()) &&
									   !query.value(1).toString().isEmpty())
	{
		fields[query.value(0).toInt()]["label"] = query.value(1).toString();
	}

	query.prepare("SELECT name, label FROM quers");

	if (query.exec()) while (query.next())
	{
		names.insert(query.value(0).toString(),
				   query.value(1).toString());
	}

	for (const auto& q : database.tables(QSql::Views))
	{
		QVariantMap map;

		map["name"] = q;
		map["label"] = names.value(q, q);

		queries.append(map);
	}

	for (const auto& i : fields) list.append(i);

	setFields(list, queries);
}
