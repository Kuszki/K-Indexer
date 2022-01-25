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

#ifndef IMAGEDOCK_H
#define IMAGEDOCK_H

#include <QtConcurrent>
#include <QtWidgets>
#include <QtCore>
#include <QtPdf>

QT_BEGIN_NAMESPACE
namespace Ui {	class ImageWidget; }
QT_END_NAMESPACE

class ImageDock : public QDockWidget
{

		Q_OBJECT

	private:

		Ui::ImageWidget *ui;

		QStandardItemModel* model;
		QList<QPixmap> list;

		QPoint lastMousePos;

		QString prefix;
		QString current;

		QPixmap currentImage;
		int currentIndex = 0;
		int currentCount = 0;

		double scale = 1.0;
		int rotation = 0;

		bool doPreview = true;
		bool doAutofit = true;

	public:

		explicit ImageDock(QWidget *parent = nullptr);
		virtual ~ImageDock(void) override;

		static QList<QPixmap> loadImages(const QString& path);
		static QList<QPixmap> loadPreviews(const QList<QPixmap>& list);

		QPixmap getImage(int index) const;

		bool isPreview(void) const;
		bool isAutofit(void) const;

	protected:

		virtual void wheelEvent(QWheelEvent* event) override;
		virtual void mousePressEvent(QMouseEvent* event) override;
		virtual void mouseMoveEvent(QMouseEvent* event) override;

	public slots:

		void setImage(const QString& path);
		void setPrefix(const QString& path);

		void setIndex(int index);

		void setPreview(bool enable);
		void setAutofit(bool enable);

		void nextImage(void);
		void prevImage(void);

		void zoomIn(void);
		void zoomOut(void);
		void zoomOrg(void);
		void zoomFit(void);

		void rotateLeft(void);
		void rotateRight(void);

		void openFile(void);
		void openFolder(void);

		void clear(void);

	private slots:

		void imageIndexChanged(const QModelIndex& index);

		void selectionRangeChanged(const QItemSelection& s,
		                           const QItemSelection& d);

};

#endif // IMAGEDOCK_H
