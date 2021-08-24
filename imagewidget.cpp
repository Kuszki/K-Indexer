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

#include "imagewidget.hpp"
#include "ui_imagewidget.h"

ImageWidget::ImageWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ImageWidget)
{
	ui->setupUi(this);

	model = new QStandardItemModel(0, 1, this);

	auto oldModel = ui->listView->model();
	auto oldSelect = ui->listView->selectionModel();

	ui->listView->setModel(model);

	oldModel->deleteLater();
	oldSelect->deleteLater();

	connect(ui->listView->selectionModel(), &QItemSelectionModel::currentRowChanged,
		   this, &ImageWidget::imageIndexChanged);

	connect(ui->listView->selectionModel(), &QItemSelectionModel::selectionChanged,
		   this, &ImageWidget::selectionRangeChanged);

	connect(ui->prev, &QPushButton::clicked,
		   this, &ImageWidget::prevImage);

	connect(ui->next, &QPushButton::clicked,
		   this, &ImageWidget::nextImage);
}

ImageWidget::~ImageWidget(void)
{
	delete ui;
}

void ImageWidget::setImage(const QString path)
{
	while (model->rowCount()) model->removeRow(0);

	QImageReader reader(path);
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

	setIndex(0);
}

void ImageWidget::setIndex(int index)
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

void ImageWidget::nextImage(void)
{
	if (currentIndex + 1 >= list.size()) setIndex(0);
	else setIndex(currentIndex + 1);
}

void ImageWidget::prevImage(void)
{
	if (currentIndex - 1 < 0) setIndex(list.size() - 1);
	else setIndex(currentIndex - 1);
}

void ImageWidget::zoomIn(void)
{
	ui->label->setPixmap(currentImage.transformed(QTransform().rotate(rotation))
					 .scaledToHeight(int((scale += 0.1) * currentImage.height())));
}

void ImageWidget::zoomOut(void)
{
	ui->label->setPixmap(currentImage.transformed(QTransform().rotate(rotation))
					 .scaledToHeight(int((scale -= 0.1) * currentImage.height())));
}

void ImageWidget::zoomOrg(void)
{
	ui->label->setPixmap(currentImage.transformed(QTransform().rotate(rotation)));

	scale = 1.0;
}

void ImageWidget::zoomFit(void)
{
	auto Img = currentImage.transformed(QTransform().rotate(rotation))
			 .scaled(ui->scrollArea->size(), Qt::KeepAspectRatio);

	scale = double(Img.height()) / double(currentImage.height());

	ui->label->setPixmap(Img);
}

void ImageWidget::rotateLeft(void)
{
	if ((rotation -= 90) <= -360) rotation = 0;

	ui->label->setPixmap(currentImage.transformed(QTransform().rotate(rotation))
					 .scaledToHeight(int(scale * currentImage.height())));
}

void ImageWidget::rotateRight(void)
{
	if ((rotation += 90) >= 360) rotation = 0;

	ui->label->setPixmap(currentImage.transformed(QTransform().rotate(rotation))
					 .scaledToHeight(int(scale * currentImage.height())));
}

void ImageWidget::imageIndexChanged(const QModelIndex& index)
{
	if (!index.isValid()) setIndex(-1);
	else setIndex(index.row());
}

void ImageWidget::selectionRangeChanged(const QItemSelection& s, const QItemSelection& d)
{
	if (s.isEmpty() && !d.isEmpty())
		ui->listView->selectionModel()->select(d, QItemSelectionModel::ClearAndSelect);
}
