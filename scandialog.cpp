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

#include "scandialog.hpp"
#include "ui_scandialog.h"

ScanDialog::ScanDialog(QWidget *parent)
     : QDialog(parent)
     , ui(new Ui::ScanDialog)
{
	ui->setupUi(this);

	ui->buttonBox->button(QDialogButtonBox::Open)->setEnabled(false);

	connect(ui->pathButton, &QToolButton::clicked,
	        this, &ScanDialog::openClicked);
}

ScanDialog::~ScanDialog(void)
{
	delete ui;
}

QStringList ScanDialog::getFilter(void) const
{
	return ui->filterEdit->toPlainText().split(
	               QRegExp("[,;\\s]+"),
	               Qt::SkipEmptyParts);
}

void ScanDialog::accept(void)
{
	QDialog::accept();

	emit onAccepted(ui->pathEdit->text(),
	                getFilter());
}

void ScanDialog::setFilter(const QStringList& filter)
{
	ui->filterEdit->setPlainText(filter.join('\n'));
}

void ScanDialog::openClicked(void)
{
	const QString path = QFileDialog::getExistingDirectory(this,
	          tr("Select export file"), QString());

	if (!path.isEmpty()) ui->pathEdit->setText(path);

	ui->buttonBox->button(QDialogButtonBox::Open)->setEnabled(!path.isEmpty());
}
