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

#include "imagedock.hpp"
#include "ui_imagedock.h"

ImageDock::ImageDock(QWidget *parent)
	: QDockWidget(parent)
	, ui(new Ui::ImageWidget)
{
	ui->setupUi(this);

	ui->prev->setVisible(false);
	ui->next->setVisible(false);
	ui->listView->setVisible(false);

	model = new QStandardItemModel(0, 1, this);

	auto oldModel = ui->listView->model();
	auto oldSelect = ui->listView->selectionModel();

	ui->listView->setModel(model);

	oldModel->deleteLater();
	oldSelect->deleteLater();

	connect(ui->listView->selectionModel(), &QItemSelectionModel::currentRowChanged,
		   this, &ImageDock::imageIndexChanged);

	connect(ui->listView->selectionModel(), &QItemSelectionModel::selectionChanged,
		   this, &ImageDock::selectionRangeChanged);

	connect(ui->prev, &QPushButton::clicked,
		   this, &ImageDock::prevImage);

	connect(ui->next, &QPushButton::clicked,
		   this, &ImageDock::nextImage);
}

ImageDock::~ImageDock(void)
{
	delete ui;
}

void ImageDock::wheelEvent(QWheelEvent* event)
{
	QWidget::wheelEvent(event);

	if (currentImage.isNull()) return;
	else if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier))
	{
		if (event->angleDelta().y() > 0) zoomIn();
		else if (event->angleDelta().y() < 0) zoomOut();
	}
}

void ImageDock::setImage(const QString& path)
{
	while (model->rowCount()) model->removeRow(0);

	QImageReader reader(current = prefix + path);
	int imageId = 0;

	list.clear();
	currentIndex = 0;

	if (reader.canRead()) do
	{
		reader.jumpToImage(imageId++);
		const QImage img = reader.read();

		QPixmap pixmap;
		pixmap.convertFromImage(img);

		QStandardItem* item = new QStandardItem(pixmap, QString::number(imageId));
		model->appendRow(item);

		list.append(pixmap);
	}
	while (imageId < reader.imageCount());

	ui->listView->setVisible(list.size() > 1);
	ui->next->setVisible(list.size() > 1);
	ui->prev->setVisible(list.size() > 1);

	ui->scrollArea->repaint();

	setIndex(0);
}

void ImageDock::setPrefix(const QString& path)
{
	if (path.isEmpty()) prefix.clear();
	else prefix = path + '/';
}

void ImageDock::setIndex(int index)
{
	if (index < 0 || index >= list.size())
	{
		ui->label->setText(tr("Unable to load image"));

		currentImage = QPixmap();
		currentIndex = 0;
	}
	else
	{
		ui->listView->selectionModel()->select(
					model->index(index, 0),
					QItemSelectionModel::ClearAndSelect);

		currentImage = list[index];
		currentIndex = index;

		zoomFit();
	}
}

void ImageDock::nextImage(void)
{
	if (list.isEmpty()) return;

	if (currentIndex + 1 >= list.size()) setIndex(0);
	else setIndex(currentIndex + 1);
}

void ImageDock::prevImage(void)
{
	if (list.isEmpty()) return;

	if (currentIndex - 1 < 0) setIndex(list.size() - 1);
	else setIndex(currentIndex - 1);
}

void ImageDock::zoomIn(void)
{
	if (currentImage.isNull()) return;

	ui->label->setPixmap(currentImage.transformed(QTransform().rotate(rotation))
					 .scaledToHeight(int((scale += 0.1) * currentImage.height())));
}

void ImageDock::zoomOut(void)
{
	if (currentImage.isNull()) return;

	ui->label->setPixmap(currentImage.transformed(QTransform().rotate(rotation))
					 .scaledToHeight(int((scale -= 0.1) * currentImage.height())));
}

void ImageDock::zoomOrg(void)
{
	if (currentImage.isNull()) return;

	ui->label->setPixmap(currentImage.transformed(QTransform().rotate(rotation)));

	scale = 1.0;
}

void ImageDock::zoomFit(void)
{
	if (currentImage.isNull()) return;

	auto Img = currentImage.transformed(QTransform().rotate(rotation))
			 .scaled(ui->scrollArea->size(), Qt::KeepAspectRatio);

	scale = double(Img.height()) / double(currentImage.height());

	ui->label->setPixmap(Img);
}

void ImageDock::rotateLeft(void)
{
	if (currentImage.isNull()) return;

	if ((rotation -= 90) <= -360) rotation = 0;

	ui->label->setPixmap(currentImage.transformed(QTransform().rotate(rotation))
					 .scaledToHeight(int(scale * currentImage.height())));
}

void ImageDock::rotateRight(void)
{
	if (currentImage.isNull()) return;

	if ((rotation += 90) >= 360) rotation = 0;

	ui->label->setPixmap(currentImage.transformed(QTransform().rotate(rotation))
					 .scaledToHeight(int(scale * currentImage.height())));
}

void ImageDock::openFile(void)
{
	if (list.isEmpty()) return;

	QDesktopServices::openUrl(QUrl::fromLocalFile(current));
}

void ImageDock::openFolder(void)
{
	if (list.isEmpty()) return;

	const QString path = QFileInfo(current).dir().absolutePath();
	QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void ImageDock::clear(void)
{
	while (model->rowCount()) model->removeRow(0);

	ui->prev->setVisible(false);
	ui->next->setVisible(false);
	ui->listView->setVisible(false);

	ui->label->setText(tr("Select document"));

	currentImage = QPixmap();
	currentIndex = 0;
}

void ImageDock::imageIndexChanged(const QModelIndex& index)
{
	if (!index.isValid()) setIndex(-1);
	else setIndex(index.row());
}

void ImageDock::selectionRangeChanged(const QItemSelection& s, const QItemSelection& d)
{
	if (s.isEmpty() && !d.isEmpty())
		ui->listView->selectionModel()->select(d, QItemSelectionModel::ClearAndSelect);
}
