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

#ifndef EXPORTDIALOG_HPP
#define EXPORTDIALOG_HPP

#include <QtWidgets>
#include <QtCore>

QT_BEGIN_NAMESPACE
namespace Ui {	class ExportDialog; }
QT_END_NAMESPACE

class ExportDialog : public QDialog
{

		Q_OBJECT

	private:

		Ui::ExportDialog *ui;

	public:

		explicit ExportDialog(QWidget *parent = nullptr);
		virtual ~ExportDialog(void) override;

		QVariantList getUsers(void) const;

	public slots:

		void setUsers(const QVariantMap& map);

		virtual void accept(void) override;

	private slots:

		void openClicked(void);
		void dataChanged(void);

	signals:

		void onAccepted(const QString&,
		                const QVariantList&,
		                int, int, int,
		                const QDateTime&,
		                const QDateTime&);

};

#endif // EXPORTDIALOG_HPP
