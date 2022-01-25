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

#ifndef SCANDIALOG_HPP
#define SCANDIALOG_HPP

#include <QtWidgets>
#include <QtCore>

QT_BEGIN_NAMESPACE
namespace Ui { class ScanDialog; }
QT_END_NAMESPACE

class ScanDialog : public QDialog
{
		Q_OBJECT

	private:

		Ui::ScanDialog *ui;

	public:

		explicit ScanDialog(QWidget *parent = nullptr);
		virtual ~ScanDialog(void) override;

		QStringList getFilter(void) const;

	public slots:

		virtual void accept(void) override;

		void setFilter(const QStringList& filter);

	private slots:

		void openClicked(void);

	signals:

		void onAccepted(const QString&, const QStringList&);

};

#endif // SCANDIALOG_HPP
