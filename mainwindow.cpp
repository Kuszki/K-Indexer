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

#include "mainwindow.hpp"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	QSettings Settings("K-OSP", "Indexer");

	ui->setupUi(this);

	ui->centralwidget->deleteLater();

	ui->actionConnect->setEnabled(true);
	ui->actionDisconnect->setEnabled(false);
	ui->actionImport->setEnabled(false);
	ui->actionExport->setEnabled(false);
	ui->actionFind->setEnabled(false);
	ui->actionNext->setEnabled(false);
	ui->actionPrevious->setEnabled(false);
	ui->actionLocknext->setEnabled(false);
	ui->actionSave->setEnabled(false);
	ui->actionLock->setEnabled(false);
	ui->actionUnlock->setEnabled(false);
	ui->actionCommit->setEnabled(false);
	ui->actionUndochange->setEnabled(false);
	ui->actionQuerydialog->setEnabled(false);

	wthread = new QThread(this);
	wthread->start();

	image = new ImageDock(this);
	items = new ItemsDock(database, this);
	meta = new MetaDock(database, this);

	filter = new FilterDialog(database, this);

	addDockWidget(Qt::RightDockWidgetArea, image);
	addDockWidget(Qt::LeftDockWidgetArea, items);
	addDockWidget(Qt::LeftDockWidgetArea, meta);

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

	Settings.beginGroup("Settings");
	ui->actionMessages->setChecked(Settings.value("messages", true).toBool());
	Settings.endGroup();

	if (isMaximized()) setGeometry(QApplication::desktop()->availableGeometry(this));

	connect(ui->actionConnect, &QAction::triggered,
		   this, &MainWindow::connectActionClicked);

	connect(ui->actionDisconnect, &QAction::triggered,
		   this, &MainWindow::disconnectActionClicked);

	connect(ui->actionQuerydialog, &QAction::triggered,
		   this, &MainWindow::queryActionClicked);

	connect(ui->actionLock, &QAction::triggered,
		   this, &MainWindow::lockActionClicked);

	connect(ui->actionUnlock, &QAction::triggered,
		   this, &MainWindow::unlockActionClicked);

	connect(ui->actionLocknext, &QAction::triggered,
		   this, &MainWindow::nextjobActionClicked);

	connect(ui->actionCommit, &QAction::triggered,
		   this, &MainWindow::commitActionClicked);

	connect(ui->actionExport, &QAction::triggered,
		   this, &MainWindow::exportActionClicked);

	connect(ui->actionImport, &QAction::triggered,
		   this, &MainWindow::importActionClicked);

	connect(ui->actionAbout, &QAction::triggered,
		   this, &MainWindow::aboutActionClicked);

	connect(ui->actionAllowclose, &QAction::toggled,
		   this, &MainWindow::dockOptionsChanged);

	connect(ui->actionAllowfloat, &QAction::toggled,
		   this, &MainWindow::dockOptionsChanged);

	connect(ui->actionAllowmove, &QAction::toggled,
		   this, &MainWindow::dockOptionsChanged);

	connect(ui->actionLockdocks, &QAction::toggled,
		   this, &MainWindow::dockOptionsChanged);

	connect(ui->actionPageup, &QAction::triggered,
		   image, &ImageDock::nextImage);

	connect(ui->actionPagedown, &QAction::triggered,
		   image, &ImageDock::prevImage);

	connect(ui->actionZoomin, &QAction::triggered,
		   image, &ImageDock::zoomIn);

	connect(ui->actionZoomout, &QAction::triggered,
		   image, &ImageDock::zoomOut);

	connect(ui->actionZoomorg, &QAction::triggered,
		   image, &ImageDock::zoomOrg);

	connect(ui->actionZoomfit, &QAction::triggered,
		   image, &ImageDock::zoomFit);

	connect(ui->actionRotateleft, &QAction::triggered,
		   image, &ImageDock::rotateLeft);

	connect(ui->actionRotateright, &QAction::triggered,
		   image, &ImageDock::rotateRight);

	connect(ui->actionOpenfile, &QAction::triggered,
		   image, &ImageDock::openFile);

	connect(ui->actionOpenfolder, &QAction::triggered,
		   image, &ImageDock::openFolder);

	connect(ui->actionSave, &QAction::triggered,
		   meta, &MetaDock::saveRecord);

	connect(ui->actionUndochange, &QAction::triggered,
		   meta, &MetaDock::rollbackRecord);

	connect(ui->actionNext, &QAction::triggered,
		   items, &ItemsDock::selectNext);

	connect(ui->actionPrevious, &QAction::triggered,
		   items, &ItemsDock::selectPrevious);

	connect(ui->actionFind, &QAction::triggered,
		   filter, &FilterDialog::show);

	connect(this, &MainWindow::onDatabaseLogout,
		   filter, &FilterDialog::close);

	connect(this, &MainWindow::onDatabaseLogin,
		   items, &ItemsDock::setupDatabase);

	connect(this, &MainWindow::onDatabaseLogout,
		   items, &ItemsDock::clearDatabase);

	connect(this, &MainWindow::onDocumentLock,
		   items, &ItemsDock::refreshList);

	connect(this, &MainWindow::onDocumentUnlock,
		   items, &ItemsDock::refreshList);

	connect(this, &MainWindow::onDatabaseLogin,
		   meta, &MetaDock::setupDatabase);

	connect(this, &MainWindow::onDatabaseLogout,
		   meta, &MetaDock::clearDatabase);

	connect(this, &MainWindow::onDocumentLock,
		   meta, &MetaDock::lockRecord);

	connect(this, &MainWindow::onDocumentUnlock,
		   meta, &MetaDock::unlockRecord);

	connect(this, &MainWindow::onImagepathUpdate,
		   image, &ImageDock::setPrefix);

	connect(this, &MainWindow::onDatabaseLogout,
		   image, &ImageDock::clear);

	connect(this, &MainWindow::onDatabaseLogin,
		   filter, &FilterDialog::setupDialog);

	connect(items, &ItemsDock::onImageSelected,
		   image, &ImageDock::setImage);

	connect(items, &ItemsDock::onItemSelected,
		   meta, &MetaDock::setupRecord);

	connect(meta, &MetaDock::onRecordSave,
		   this, &MainWindow::metaDataSaved);

	connect(items, &ItemsDock::onItemSelected,
		   this, &MainWindow::recordIndexSelected);

	connect(filter, &FilterDialog::onFiltersUpdate,
		   items, &ItemsDock::setFilter);

	connect(items, &ItemsDock::onFilterClicked,
		   filter, &FilterDialog::show);

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

	Settings.beginGroup("Settings");
	Settings.setValue("messages", ui->actionMessages->isChecked());
	Settings.endGroup();

	wthread->exit();
	wthread->wait();

	delete ui;
}

void MainWindow::openDatabase(const QString& driver, const QString& server, const QString& name, const QString& user, const QString& pass, const QString& path)
{
	if (database.isOpen()) database.close();

	database = QSqlDatabase::addDatabase(driver);
	imgPath = path;
	userID = -1;

	if (server.contains(':'))
		database.setPort(server.section(':', 1).toInt());

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

		if (userID < 0) database.close();
	}

	ui->actionConnect->setEnabled(userID == -1);
	ui->actionDisconnect->setEnabled(userID != -1);
	ui->actionFind->setEnabled(userID != -1);
	ui->actionImport->setEnabled(userID != -1);
	ui->actionExport->setEnabled(userID != -1);
	ui->actionLocknext->setEnabled(userID != -1);
	ui->actionNext->setEnabled(userID != -1);
	ui->actionPrevious->setEnabled(userID != -1);
	ui->actionQuerydialog->setEnabled(userID != -1);

	if (userID >= 0)
	{
		emit onDatabaseLogin(userID);
		emit onImagepathUpdate(path);
	}
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
	ui->actionConnect->setEnabled(true);
	ui->actionDisconnect->setEnabled(false);
	ui->actionImport->setEnabled(false);
	ui->actionExport->setEnabled(false);
	ui->actionFind->setEnabled(false);
	ui->actionNext->setEnabled(false);
	ui->actionPrevious->setEnabled(false);
	ui->actionLocknext->setEnabled(false);
	ui->actionSave->setEnabled(false);
	ui->actionLock->setEnabled(false);
	ui->actionUnlock->setEnabled(false);
	ui->actionUndochange->setEnabled(false);
	ui->actionQuerydialog->setEnabled(false);

	emit onDatabaseLogout();

	database.close();
}

void MainWindow::lockActionClicked(void)
{
	const QString title = tr("Unable to lock document");

	QSqlQuery query(database);
	int code = 0;

	query.prepare("SELECT sheet, user FROM locks WHERE sheet = ?");
	query.addBindValue(sheetID);

	if (query.exec())
	{
		if (query.next())
		{
			code = query.value(1).toInt() == userID ? 1 : 2;
		}
	}
	else code = -1;

	switch (code)
	{
		case 1:
			showErrorMessage(tr("Document is already locked"), title);
		return;
		case 2:
			showErrorMessage(tr("Document is locked by another user"), title);
		return;
		case -1:
			showErrorMessage(tr("Unable to execute query"), title);
		return;
	}

	query.prepare("SELECT user FROM main WHERE id = ?");
	query.addBindValue(sheetID);

	if (query.exec())
	{
		if (query.next())
		{
			code = query.value(0).toInt() == userID ||
				  query.value(0).toInt() == 0 ||
				  query.value(0).isNull() ||
				  userID == 0 ? 0 : 3;
		}
	}
	else code = -1;

	switch (code)
	{
		case 3:
			showErrorMessage(tr("Document is assigned to another user"), title);
		return;
		case -1:
			showErrorMessage(tr("Unable to execute query"), title);
		return;
	}

	query.prepare("INSERT INTO locks (sheet, user) VALUES (?, ?)");
	query.addBindValue(sheetID);
	query.addBindValue(userID);

	if (query.exec())
	{
		ui->statusbar->showMessage(tr("Document locked"));

		emit onDocumentLock(sheetID);
	}
	else showErrorMessage(tr("Unable to execute query"), title);
}

void MainWindow::unlockActionClicked(void)
{
	const QString title = tr("Unable to unlock document");

	QSqlQuery query(database);
	int code = 0;

	query.prepare("SELECT sheet, user FROM locks WHERE sheet = ?");
	query.addBindValue(sheetID);

	if (query.exec())
	{
		if (query.next())
		{
			code = query.value(1).toInt() == userID ||
				  userID == 0 ?
					  0 : 1;
		}
		else code = 2;
	}
	else code = -1;

	switch (code)
	{
		case 1:
			showErrorMessage(tr("Document is locked by another user"), title);
		return;
		case 2:
			showErrorMessage(tr("Document is not locked"), title);
		return;
		case -1:
			showErrorMessage(tr("Unable to execute query"), title);
		return;
	}

	query.prepare("DELETE FROM locks WHERE sheet = ?");
	query.addBindValue(sheetID);

	if (query.exec())
	{
		ui->statusbar->showMessage(tr("Document unlocked"));

		emit onDocumentUnlock(sheetID);
	}
	else showErrorMessage(tr("Unable to execute query"), title);
}

void MainWindow::nextjobActionClicked(void)
{
	QSqlQuery query(database), insert(database);

	query.prepare("SELECT id, path FROM main WHERE "
			    "id NOT IN (SELECT sheet FROM locks) AND "
			    "user IS NULL");

	insert.prepare("INSERT INTO locks (sheet, user) VALUES (?, ?)");

	if (query.exec()) while (query.next())
	{
		insert.addBindValue(query.value(0));
		insert.addBindValue(userID);

		if (insert.exec())
		{
			const QString path = query.value(1).toString();
			const int id = query.value(0).toInt();

			insert.finish();
			insert.clear();

			query.finish();
			query.clear();

			items->refreshList();
			items->selectItem(id);
			meta->setupRecord(id);
			image->setImage(path);

			ui->statusbar->showMessage(tr("Document locked"));

			recordIndexSelected(id); return;
		}
	}

	showErrorMessage(tr("Unable to lock next document"), tr("Error"));
}

void MainWindow::commitActionClicked(void)
{
	if (meta->saveRecord()) unlockActionClicked();
}

void MainWindow::exportActionClicked(void)
{
	ExportDialog* dialog = new ExportDialog(this); dialog->open();

	QSqlQuery query(database);
	QVariantMap map;

	query.prepare("SELECT id, name FROM users WHERE id = ? OR 0 = ?");
	query.addBindValue(userID);
	query.addBindValue(userID);

	if (query.exec()) while (query.next())
	{
		map.insert(query.value(1).toString(), query.value(0));
	}

	if (!map.isEmpty()) dialog->setUsers(map);

	connect(dialog, &ExportDialog::finished, dialog, &ExportDialog::deleteLater);
	connect(dialog, &ExportDialog::onAccepted, this, &MainWindow::performExport);
}

void MainWindow::importActionClicked(void)
{
	ImportDialog* dialog = new ImportDialog(this); dialog->open();

	const QSqlRecord record = database.record("main");
	QStringList list;

	for (int i = 1; i < record.count(); ++i)
	{
		list.append(record.fieldName(i));
	}

	if (!list.isEmpty()) dialog->setFields(list);

	connect(dialog, &ImportDialog::finished, dialog, &ImportDialog::deleteLater);
	connect(dialog, &ImportDialog::onAccepted, this, &MainWindow::performImport);
}

void MainWindow::queryActionClicked(void)
{
	SqleditorDialog* dialog = new SqleditorDialog(database, userID == 0, this); dialog->show();

	connect(dialog, &SqleditorDialog::finished, dialog, &SqleditorDialog::deleteLater);
	connect(this, &MainWindow::onDatabaseLogout, dialog, &SqleditorDialog::deleteLater);
}

void MainWindow::aboutActionClicked(void)
{
	AboutDialog* dialog = new AboutDialog(this); dialog->open();

	connect(dialog, &AboutDialog::finished, dialog, &AboutDialog::deleteLater);
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
	ui->actionSave->setEnabled(index);
	ui->actionLock->setEnabled(index);
	ui->actionUnlock->setEnabled(index);
	ui->actionCommit->setEnabled(index);
	ui->actionUndochange->setEnabled(index);

	sheetID = index;
}

void MainWindow::metaDataSaved(int ok, const QString& msg)
{
	if (ok == 0) ui->statusbar->showMessage(tr("No changes to save"));
	else if (ok == 1) ui->statusbar->showMessage(tr("Data saved"));
	else showErrorMessage(msg, tr("Unable to save data"));

	if (ok == 1) items->refreshList();
}

void MainWindow::showInfoMessage(const QString& msg, const QString& title)
{
	if (!ui->actionMessages->isChecked()) ui->statusbar->showMessage(msg);
	else QMessageBox::information(this, title, msg, QMessageBox::Ok);
}

void MainWindow::showErrorMessage(const QString& msg, const QString& title)
{
	if (!ui->actionMessages->isChecked()) ui->statusbar->showMessage(msg);
	else QMessageBox::critical(this, title, msg, QMessageBox::Ok);
}

void MainWindow::showWarningMessage(const QString& msg, const QString& title)
{
	if (!ui->actionMessages->isChecked()) ui->statusbar->showMessage(msg);
	else QMessageBox::warning(this, title, msg, QMessageBox::Ok);
}

void MainWindow::performExport(const QString& path, const QVariantList& users, int status, int validation, int lock, const QDateTime& from, const QDateTime& to)
{
	ThreadWorker* worker = new ThreadWorker(database);
	worker->moveToThread(wthread);

	QProgressBar* progress = new QProgressBar();
	progress->setRange(0, 0);

	ui->statusbar->addPermanentWidget(progress);
	this->setEnabled(false);

	connect(worker, &ThreadWorker::onFinish, this, &MainWindow::finishExport);

	connect(worker, &ThreadWorker::onFinish, worker, &ThreadWorker::deleteLater);

	connect(worker, &ThreadWorker::onFinish, progress, &QProgressBar::deleteLater);
	connect(worker, &ThreadWorker::onSetup, progress, &QProgressBar::setRange);
	connect(worker, &ThreadWorker::onProgress, progress, &QProgressBar::setValue);

	connect(this, &MainWindow::onExportRequest, worker, &ThreadWorker::exportData);

	emit onExportRequest(path, users, status, validation, lock, from, to);
}

void MainWindow::performImport(const QString& path, const QString& logs, const QVariantMap& map, bool header)
{
	ThreadWorker* worker = new ThreadWorker(database);
	worker->moveToThread(wthread);

	QProgressBar* progress = new QProgressBar();
	progress->setRange(0, 0);

	ui->statusbar->addPermanentWidget(progress);
	this->setEnabled(false);

	connect(worker, &ThreadWorker::onFinish, this, &MainWindow::finishImport);

	connect(worker, &ThreadWorker::onFinish, items, &ItemsDock::refreshList);

	connect(worker, &ThreadWorker::onFinish, worker, &ThreadWorker::deleteLater);

	connect(worker, &ThreadWorker::onFinish, progress, &QProgressBar::deleteLater);
	connect(worker, &ThreadWorker::onSetup, progress, &QProgressBar::setRange);
	connect(worker, &ThreadWorker::onProgress, progress, &QProgressBar::setValue);

	connect(this, &MainWindow::onImportRequest, worker, &ThreadWorker::importData);

	emit onImportRequest(path, logs, map, header, userID);
}

void MainWindow::finishExport(const QString& msg, int code)
{
	switch (code)
	{
		case 1:
			showErrorMessage(msg, tr("Export error"));
		break;
		case 2:
			showWarningMessage(msg, tr("Export finished"));
		break;
		default:
			showInfoMessage(msg, tr("Export finished"));
	}

	setEnabled(true);
}

void MainWindow::finishImport(const QString& msg, int code)
{
	switch (code)
	{
		case 1:
			showErrorMessage(msg, tr("Import error"));
		break;
		case 2:
			showWarningMessage(msg, tr("Import finished"));
		break;
		default:
			showInfoMessage(msg, tr("Import finished"));
	}

	setEnabled(true);
}

