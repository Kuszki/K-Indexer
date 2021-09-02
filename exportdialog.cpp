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

#include "exportdialog.hpp"
#include "ui_exportdialog.h"

ExportDialog::ExportDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::ExportDialog)
{
	ui->setupUi(this);

	ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);

	ui->fromDate->setDate(QDate::currentDate());
	ui->fromDate->setTime(QTime(0, 0, 0));

	ui->toDate->setDate(QDate::currentDate());
	ui->toDate->setTime(QTime(23, 59, 59));

	connect(ui->pathButton, &QToolButton::clicked,
		   this, &ExportDialog::openClicked);

	connect(ui->fromCheck, &QCheckBox::toggled,
		   this, &ExportDialog::dataChanged);

	connect(ui->toCheck, &QCheckBox::toggled,
		   this, &ExportDialog::dataChanged);

	connect(ui->fromDate, &QDateTimeEdit::dateTimeChanged,
		   this, &ExportDialog::dataChanged);

	connect(ui->toDate, &QDateTimeEdit::dateTimeChanged,
		   this, &ExportDialog::dataChanged);

	dataChanged();
}

ExportDialog::~ExportDialog(void)
{
	delete ui;
}

void ExportDialog::setUsers(const QVariantMap& map)
{
	if (map.size() <= 1) ui->userCombo->clear();

	if (map.size() == 0)
		ui->userCombo->addItem(
					tr("Current user"),
					0);

	else if (map.size() == 1)
		ui->userCombo->addItem(
					map.firstKey(),
					map.first());

	else
	{
		auto amodel = new QStandardItemModel(0, 1, this);
		auto aitem = new QStandardItem(tr("Select users"));

		for (auto i = map.cbegin(); i != map.cend(); ++i)
		{
			auto format = new QStandardItem(i.key());

			format->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
			format->setCheckState(Qt::Unchecked);
			format->setData(i.value());

			amodel->appendRow(format);
		}

		amodel->sort(0);
		aitem->setFlags(Qt::ItemIsEnabled);
		amodel->insertRow(0, aitem);

		ui->userCombo->model()->deleteLater();
		ui->userCombo->setModel(amodel);
	}
}

QVariantList ExportDialog::getUsers(void) const
{
	auto M = dynamic_cast<QStandardItemModel*>(ui->userCombo->model());

	QVariantList checked;

	if (ui->userCombo->count() == 1) return { ui->userCombo->itemData(0) };
	else for (int i = 1; i < M->rowCount(); ++i)
		if (M->item(i)->checkState() == Qt::Checked)
		{
			checked.append(M->item(i)->data());
		}

	return checked;
}

void ExportDialog::accept(void)
{
	QDialog::accept();

	emit onAccepted(ui->pathEdit->text(), getUsers(),
				 ui->statusCombo->currentIndex(),
				 ui->validationCombo->currentIndex(),
				 ui->lockedCombo->currentIndex(),
				 ui->fromCheck->isChecked() ?
					 ui->fromDate->dateTime() :
					 QDateTime(),
				 ui->toCheck->isChecked() ?
					 ui->toDate->dateTime() :
					 QDateTime());
}

void ExportDialog::openClicked(void)
{
	const QString path = QFileDialog::getSaveFileName(this,
			tr("Select export file"), QString(),
			tr("Text files (*.txt);;CSV files (*.csv);;All files (*.*)"));

	if (!path.isEmpty()) ui->pathEdit->setText(path);

	ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(!path.isEmpty());
}

void ExportDialog::dataChanged(void)
{
	const QDateTime a = ui->fromDate->dateTime();
	const QDateTime b = ui->toDate->dateTime();

	if (ui->fromCheck->isChecked())
		ui->toDate->setMinimumDateTime(a);
	else
		ui->toDate->clearMinimumDateTime();

	if (ui->toCheck->isChecked())
		ui->fromDate->setMaximumDateTime(b);
	else
		ui->fromDate->clearMaximumDateTime();
}
