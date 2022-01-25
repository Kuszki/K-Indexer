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

QList<QPixmap> ImageDock::loadImages(const QString& path)
{
	static QPixmap (*conv_proc)(const QImage&) =
	[] (const QImage& img) -> QPixmap
	{
		return QPixmap::fromImage(img);
	};

	QList<QImage> images;

	if (path.endsWith(".pdf", Qt::CaseInsensitive))
	{
		QPdfDocument pdf; pdf.load(path);

		for (int i = 0; i < pdf.pageCount(); ++i)
		{
			QSize size = (5*pdf.pageSize(i)).toSize();
			QImage img = pdf.render(i, size);

			if (!img.isNull()) images.append(img);
		}
	}
	else
	{
		QImageReader reader(path);
		int imageId = 0;

		if (reader.canRead()) do
		{
			reader.jumpToImage(imageId++);
			images.append(reader.read());
		}
		while (imageId < reader.imageCount());
	}

	return QtConcurrent::blockingMapped(images, conv_proc);
}

QList<QPixmap> ImageDock::loadPreviews(const QList<QPixmap>& list)
{
	static QPixmap (*scale_proc)(const QPixmap&) =
	[] (const QPixmap& img) -> QPixmap
	{
		return img.scaledToHeight(128, Qt::SmoothTransformation);
	};

	return QtConcurrent::blockingMapped(list, scale_proc);
}

QPixmap ImageDock::getImage(int index) const
{
	if (index < 0 || index >= currentCount) return QPixmap();

	if (!list.isEmpty()) return list.value(index, QPixmap());
	else if (doPreview) return QPixmap();

	if (current.endsWith(".pdf", Qt::CaseInsensitive))
	{
		QPdfDocument pdf; pdf.load(current);

		QSize size = (5*pdf.pageSize(index)).toSize();
		QImage img = pdf.render(index, size);

		return QPixmap::fromImage(img);
	}
	else
	{
		QImageReader reader(current);

		if (!reader.canRead()) return QPixmap();
		if (index && !reader.jumpToImage(index)) return QPixmap();;

		return QPixmap::fromImage(reader.read());
	}
}

bool ImageDock::isPreview(void) const
{
	return doPreview;
}

bool ImageDock::isAutofit(void) const
{
	return doAutofit;
}

void ImageDock::wheelEvent(QWheelEvent* event)
{
	if (!currentImage.isNull() &&
	    QApplication::keyboardModifiers()
	    .testFlag(Qt::ControlModifier))
	{
		if (event->angleDelta().y() > 0) zoomIn();
		else if (event->angleDelta().y() < 0) zoomOut();

		event->accept();
	}
	else QDockWidget::wheelEvent(event);
}

void ImageDock::mousePressEvent(QMouseEvent* event)
{
	if (event->buttons() == Qt::MiddleButton)
	{
		lastMousePos = event->pos();

		event->accept();
	}
	else QDockWidget::mousePressEvent(event);
}

void ImageDock::mouseMoveEvent(QMouseEvent* event)
{
	if (event->buttons() == Qt::MiddleButton)
	{
		auto dPos = event->pos() - lastMousePos;

		const int y = ui->scrollArea->verticalScrollBar()->value();
		ui->scrollArea->verticalScrollBar()->setValue(y - dPos.y());

		const int x = ui->scrollArea->horizontalScrollBar()->value();
		ui->scrollArea->horizontalScrollBar()->setValue(x - dPos.x());

		lastMousePos = event->pos();

		event->accept();
	}
	else QDockWidget::mouseMoveEvent(event);
}

void ImageDock::setImage(const QString& path)
{
	setUpdatesEnabled(false);

	if (!QFile::exists(prefix + path)) clear();
	else current = prefix + path;

	if (doPreview) list = loadImages(current);
	else list.clear();

	if (model->rowCount()) model->setRowCount(0);

	if (doPreview)
	{
		currentCount = list.count(); int i(0);

		for (const auto& p : loadPreviews(list))
		{
			QStandardItem* item = new QStandardItem();

			item->setTextAlignment(Qt::AlignCenter);
			item->setText(QString::number(++i));
			item->setIcon(p);

			model->appendRow(item);
		}
	}
	else
	{
		QImageReader reader(current);
		currentCount = reader.imageCount();

		for (int i = 1; i <= currentCount; ++i)
		{
			QStandardItem* item = new QStandardItem();

			item->setTextAlignment(Qt::AlignCenter);
			item->setText(QString::number(i));

			model->appendRow(item);
		}
	}

	const int size = ui->listView->sizeHintForRow(0) +
	                 ui->listView->verticalScrollBar()->height();

	const bool multi = currentCount > 1;

	ui->listView->setViewMode(doPreview ? QListView::IconMode : QListView::ListMode);

	ui->listView->setMinimumHeight(size);
	ui->listView->setMaximumHeight(size);

	ui->listView->setVisible(multi);
	ui->next->setVisible(multi);
	ui->prev->setVisible(multi);

	setUpdatesEnabled(true);

	setIndex(0);
}

void ImageDock::setPrefix(const QString& path)
{
	if (path.isEmpty()) prefix.clear();
	else prefix = path + '/';
}

void ImageDock::setIndex(int index)
{
	currentImage = getImage(index);
	currentIndex = currentImage.isNull() ? 0 : index;

	if (!currentImage.isNull()) doAutofit ? zoomFit() : zoomOrg();
	else ui->label->setText(tr("Unable to load image"));

	ui->listView->selectionModel()->select(
	               model->index(currentIndex, 0),
	               QItemSelectionModel::ClearAndSelect);
}

void ImageDock::setPreview(bool enable)
{
	if (doPreview == enable) return;
	else doPreview = enable;

	if (currentImage.isNull()) return;

	const QString path = current.mid(prefix.length());
	const int index = currentIndex;

	setImage(path);
	setIndex(index);
}

void ImageDock::setAutofit(bool enable)
{
	doAutofit = enable;
}

void ImageDock::nextImage(void)
{
	if (currentCount <= 0) return;

	if (currentIndex + 1 >= currentCount) setIndex(0);
	else setIndex(currentIndex + 1);
}

void ImageDock::prevImage(void)
{
	if (currentCount <= 0) return;

	if (currentIndex - 1 < 0) setIndex(currentCount - 1);
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
