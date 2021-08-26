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

#include "mainwindow.hpp"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	QSettings Settings("K-OSP", "Indexer");

	ui->setupUi(this);

	ui->actionConnect->setEnabled(true);
	ui->actionDisconnect->setEnabled(false);
	ui->actionImport->setEnabled(false);
	ui->actionFind->setEnabled(false);

	items = new ItemsDock(database, this);
	meta = new MetaDock(database, this);

	addDockWidget(Qt::LeftDockWidgetArea, items);
	addDockWidget(Qt::RightDockWidgetArea, meta);

	Settings.beginGroup("Window");
	setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::TabPosition::North);
	restoreGeometry(Settings.value("geometry").toByteArray());
	restoreState(Settings.value("state").toByteArray());
	Settings.endGroup();

	Settings.beginGroup("Docks");
	ui->actionAllowclose->setChecked(Settings.value("close", true).toBool());
	ui->actionAllowfloat->setChecked(Settings.value("float", true).toBool());
	ui->actionAllowmove->setChecked(Settings.value("move", true).toBool());
	ui->actionLockdocks->setChecked(Settings.value("lock", false).toBool());
	Settings.endGroup();

	if (isMaximized()) setGeometry(QApplication::desktop()->availableGeometry(this));

	connect(ui->actionPageup, &QAction::triggered,
		   ui->image, &ImageWidget::nextImage);

	connect(ui->actionPagedown, &QAction::triggered,
		   ui->image, &ImageWidget::prevImage);

	connect(ui->actionZoomin, &QAction::triggered,
		   ui->image, &ImageWidget::zoomIn);

	connect(ui->actionZoomout, &QAction::triggered,
		   ui->image, &ImageWidget::zoomOut);

	connect(ui->actionZoomorg, &QAction::triggered,
		   ui->image, &ImageWidget::zoomOrg);

	connect(ui->actionZoomfit, &QAction::triggered,
		   ui->image, &ImageWidget::zoomFit);

	connect(ui->actionRotateleft, &QAction::triggered,
		   ui->image, &ImageWidget::rotateLeft);

	connect(ui->actionRotateright, &QAction::triggered,
		   ui->image, &ImageWidget::rotateRight);

	connect(ui->actionConnect, &QAction::triggered,
		   this, &MainWindow::connectActionClicked);

	connect(ui->actionDisconnect, &QAction::triggered,
		   this, &MainWindow::disconnectActionClicked);

	connect(ui->actionAllowclose, &QAction::toggled,
		   this, &MainWindow::dockOptionsChanged);

	connect(ui->actionAllowfloat, &QAction::toggled,
		   this, &MainWindow::dockOptionsChanged);

	connect(ui->actionAllowmove, &QAction::toggled,
		   this, &MainWindow::dockOptionsChanged);

	connect(ui->actionLockdocks, &QAction::toggled,
		   this, &MainWindow::dockOptionsChanged);

	connect(ui->actionSave, &QAction::triggered,
		   meta, &MetaDock::saveRecord);

	connect(ui->actionUndochange, &QAction::triggered,
		   meta, &MetaDock::rollbackRecord);

	connect(this, &MainWindow::onDatabaseLogin,
		   items, &ItemsDock::setupDatabase);

	connect(this, &MainWindow::onDatabaseLogout,
		   items, &ItemsDock::clearDatabase);

	connect(this, &MainWindow::onDatabaseLogin,
		   meta, &MetaDock::setupDatabase);

	connect(this, &MainWindow::onDatabaseLogout,
		   meta, &MetaDock::clearDatabase);

	connect(items, &ItemsDock::onImageSelected,
		   ui->image, &ImageWidget::setImage);

	connect(items, &ItemsDock::onItemSelected,
		   meta, &MetaDock::setupRecord);

	connect(meta, &MetaDock::onRecordSave,
		   this, &MainWindow::metaDataSaved);

	connect(items, &ItemsDock::onItemSelected,
		   this, &MainWindow::recordIndexSelected);

	dockOptionsChanged();
}

MainWindow::~MainWindow(void)
{
	QSettings Settings("K-OSP", "Indexer");

	Settings.beginGroup("Window");
	Settings.setValue("state", saveState());
	Settings.setValue("geometry", saveGeometry());
	Settings.endGroup();

	Settings.beginGroup("Docks");
	Settings.setValue("close", ui->actionAllowclose->isChecked());
	Settings.setValue("float", ui->actionAllowfloat->isChecked());
	Settings.setValue("move", ui->actionAllowmove->isChecked());
	Settings.setValue("lock", ui->actionLockdocks->isChecked());
	Settings.endGroup();

	delete ui;
}

void MainWindow::openDatabase(const QString& driver, const QString& server, const QString& name, const QString& user, const QString& pass, const QString& path)
{
	static int defaultPort = 0;

	if (database.isOpen()) database.close();

	database = QSqlDatabase::addDatabase(driver);
	imgPath = path;
	userID = 0;

	if (!defaultPort) defaultPort = database.port();

	if (!server.contains(':')) database.setPort(defaultPort);
	else database.setPort(server.section(':', 1).toInt());

	database.setHostName(server.section(':', 0, 0));
	database.setDatabaseName(name);
	database.setUserName(user);
	database.setPassword(pass);

	if (!database.open()) emit onDatabaseError(database.lastError().text());
	else
	{
		const QString hash = QCryptographicHash::hash(
							 pass.toUtf8(),
							 QCryptographicHash::Sha1)
						 .toHex();

		QSqlQuery query(database);

		query.prepare("SELECT id FROM users WHERE name = ? AND pass = ?");
		query.addBindValue(user);
		query.addBindValue(hash);

		if (query.exec() && query.next()) userID = query.value(0).toInt();
		else emit onDatabaseError(tr("User authentication error"));

		if (!userID) database.close();
	}

	ui->actionConnect->setEnabled(!userID);
	ui->actionDisconnect->setEnabled(userID);
	ui->actionFind->setEnabled(userID);
	ui->actionImport->setEnabled(userID);

	ui->image->setPrefix(path);

	emit onDatabaseLogin(userID);
}

void MainWindow::connectActionClicked(void)
{
	ConnectDialog* dialog = new ConnectDialog(this); dialog->open();

	connect(dialog, &ConnectDialog::onAccept, this, &MainWindow::openDatabase);
	connect(dialog, &ConnectDialog::finished, dialog, &ConnectDialog::deleteLater);

	connect(this, &MainWindow::onDatabaseLogin, dialog, &ConnectDialog::connected);
	connect(this, &MainWindow::onDatabaseError, dialog, &ConnectDialog::refused);
}

void MainWindow::disconnectActionClicked(void)
{
	database.close();

	ui->actionConnect->setEnabled(true);
	ui->actionDisconnect->setEnabled(false);
	ui->actionFind->setEnabled(false);

	ui->image->clear();

	emit onDatabaseLogout();
}

void MainWindow::lockActionClicked(void)
{

}

void MainWindow::dockOptionsChanged(void)
{
	QDockWidget::DockWidgetFeatures flags = QDockWidget::NoDockWidgetFeatures;

	if (ui->actionAllowfloat->isChecked())
		flags |= QDockWidget::DockWidgetFloatable;

	if (ui->actionAllowmove->isChecked())
		flags |= QDockWidget::DockWidgetMovable;

	if (ui->actionAllowclose->isChecked())
		flags |= QDockWidget::DockWidgetClosable;

	if (ui->actionLockdocks->isChecked())
		flags = QDockWidget::NoDockWidgetFeatures;

	for (const auto& w : this->children())
	{
		if (auto d = dynamic_cast<QDockWidget*>(w))
		{
			d->setFeatures(flags);
		}
		else if (auto b = dynamic_cast<QToolBar*>(w))
		{
			b->setFloatable(ui->actionAllowfloat->isChecked() &&
						 !ui->actionLockdocks->isChecked());
			b->setMovable(ui->actionAllowmove->isChecked() &&
					    !ui->actionLockdocks->isChecked());
		}
	}
}

void MainWindow::recordIndexSelected(int index)
{
	sheetID = index;
}

void MainWindow::metaDataSaved(int sheet, bool ok, const QString& msg)
{
	if (ok) ui->statusbar->showMessage(tr("Record saved"));
	else ui->statusbar->showMessage(tr("Error: %1").arg(msg));
}

