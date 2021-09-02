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

#include "importdialog.hpp"
#include "ui_importdialog.h"

ImportDialog::ImportDialog(QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::ImportDialog)
{
	ui->setupUi(this);

	ui->buttonBox->button(QDialogButtonBox::Open)->setEnabled(false);

	connect(ui->pathButton, &QToolButton::clicked,
		   this, &ImportDialog::pathClicked);

	connect(ui->logButton, &QToolButton::clicked,
		   this, &ImportDialog::logsClicked);
}

ImportDialog::~ImportDialog(void)
{
	delete ui;
}

void ImportDialog::setFields(const QStringList& rows)
{
	while (ui->formLayout->rowCount())
		ui->formLayout->removeRow(0);

	spins.clear();
	labels.clear();

	int current = 0;

	for (const auto& r : rows)
	{
		QLabel* label = new QLabel(r);
		QSpinBox* spin = new QSpinBox();

		spin->setRange(current ? 0 : 1, 50);
		spin->setSpecialValueText(current ? tr("Skip") : QString());
		spin->setValue(++current);

		ui->formLayout->addRow(label, spin);

		spins.append(spin);
		labels.append(label);
	}
}

QVariantMap ImportDialog::getFields(void) const
{
	QVariantMap map;

	for (int i = 0; i < spins.size(); ++i) if (spins[i]->value())
	{
		map.insert(labels[i]->text(), spins[i]->value() - 1);
	}

	return map;
}

void ImportDialog::accept(void)
{
	QDialog::accept();

	emit onAccepted(ui->pathEdit->text(),
				 ui->logEdit->text(),
				 getFields(),
				 ui->headerCombo->currentIndex() == 0);
}

void ImportDialog::pathClicked(void)
{
	const QString path = QFileDialog::getOpenFileName(this,
			tr("Select import file"), ui->pathEdit->text(),
			tr("Text files (*.txt);;CSV files (*.csv);;All files (*.*)"));

	if (!path.isEmpty()) ui->pathEdit->setText(path); else return;

	QFile file(path);

	if (!file.open(QFile::ReadOnly | QFile::Text)) return;

	const QChar sep = path.endsWith(".csv") ? ';' : '\t';
	const QString line = QString::fromLocal8Bit(file.readLine());
	const QStringList fields = line.trimmed().split(sep);

	for (int i = 0; i < fields.size(); ++i)
		for (int j = 0; j < spins.size(); ++j)
			if (labels[j]->text() == fields[i])
				spins[j]->setValue(i + 1);

	for (int i = 0; i < spins.size(); ++i)
		if (!fields.contains(labels[i]->text()))
			spins[i]->setValue(0);

	ui->buttonBox->button(QDialogButtonBox::Open)->setEnabled(true);
}

void ImportDialog::logsClicked(void)
{
	const QString path = QFileDialog::getSaveFileName(this,
			tr("Select log file"), ui->logEdit->text(),
			tr("Text files (*.txt);;Log files (*.log);;All files (*.*)"));

	if (!path.isEmpty()) ui->logEdit->setText(path);
}
