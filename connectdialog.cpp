/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Firebird database editor                                               *
 *  Copyright (C) 2016  Łukasz "Kuszki" Dróżdż  lukasz.kuszki@gmail.com    *
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

#include "connectdialog.hpp"
#include "ui_connectdialog.h"

ConnectDialog::ConnectDialog(QWidget *Parent)
: QDialog(Parent), ui(new Ui::ConnectDialog)
{
	ui->setupUi(this);

	ui->buttonBox->button(QDialogButtonBox::Open)->setEnabled(false);
	ui->Driver->addItems(QSqlDatabase::drivers());

	QSettings Settings("K-OSP", "Indexer");

	Settings.beginGroup("History");
	ui->Server->addItems(Settings.value("server").toStringList());
	ui->Database->addItems(Settings.value("path").toStringList());
	Settings.endGroup();

	Settings.beginGroup("Database");
	ui->Server->setCurrentText(Settings.value("server").toString());
	ui->Database->setCurrentText(Settings.value("path").toString());
	ui->Driver->setCurrentText(Settings.value("driver").toString());
	ui->User->setText(Settings.value("user").toString());
	ui->Path->setText(Settings.value("root").toString());
	Settings.endGroup();

	if (!ui->Server->currentText().isEmpty() &&
	    !ui->Database->currentText().isEmpty() &&
	    !ui->User->text().isEmpty())
	{
		ui->Password->setFocus();
	}

	ui->Server->model()->sort(0);
	ui->Database->model()->sort(0);
}

ConnectDialog::~ConnectDialog(void)
{
	delete ui;
}

void ConnectDialog::edited(void)
{
	ui->buttonBox->button(QDialogButtonBox::Open)->setEnabled(
				!ui->Database->currentText().isEmpty() &&
				!ui->User->text().isEmpty() &&
				!ui->Password->text().isEmpty() &&
				!ui->Path->text().isEmpty());
}

void ConnectDialog::openpath(void)
{
	const QString Path = QFileDialog::getExistingDirectory(this,
						tr("Select scans directory"),
						ui->Path->text());

	if (!Path.isEmpty()) ui->Path->setText(Path);
}

void ConnectDialog::accept(void)
{
	setEnabled(false);

	emit onAccept(ui->Driver->currentText(), ui->Server->currentText(),
			    ui->Database->currentText(), ui->User->text(),
			    ui->Password->text(), ui->Path->text());
}

void ConnectDialog::reject(void)
{
	ui->Password->clear(); QDialog::reject();
}

void ConnectDialog::refused(const QString& Error)
{
	QMessageBox::critical(this, tr("Error"), Error); setEnabled(true);
}

void ConnectDialog::connected(bool OK)
{
	if (!OK) return;

	QSettings Settings("K-OSP", "Indexer");

	Settings.beginGroup("Database");
	Settings.setValue("server", ui->Server->currentText());
	Settings.setValue("path", ui->Database->currentText());
	Settings.setValue("driver", ui->Driver->currentText());
	Settings.setValue("root", ui->Path->text());
	Settings.setValue("user", ui->User->text());
	Settings.endGroup();

	Settings.beginGroup("History");

	QStringList Server = Settings.value("server").toStringList();
	QStringList Path = Settings.value("path").toStringList();

	if (!Server.contains(ui->Server->currentText()))
	{
		Settings.setValue("server", Server << ui->Server->currentText());
		ui->Server->addItem(ui->Server->currentText());
	}

	if (!Path.contains(ui->Database->currentText()))
	{
		Settings.setValue("path", Path << ui->Database->currentText());
		ui->Database->addItem(ui->Database->currentText());
	}

	Settings.endGroup();

	ui->Password->clear();
	setEnabled(true);

	QDialog::accept();
}
