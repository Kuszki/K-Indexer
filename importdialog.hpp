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

#ifndef IMPORTDIALOG_HPP
#define IMPORTDIALOG_HPP

#include <QtWidgets>
#include <QtCore>
#include <QtSql>

QT_BEGIN_NAMESPACE
namespace Ui {	class ImportDialog; }
QT_END_NAMESPACE

class ImportDialog : public QDialog
{

		Q_OBJECT

	private:

		Ui::ImportDialog *ui;

		QList<QLabel*> labels;
		QList<QSpinBox*> spins;

	public:

		explicit ImportDialog(QWidget *parent = nullptr);
		virtual ~ImportDialog(void) override;

	public slots:

		void setFields(const QStringList& rows);
		QVariantMap getFields(void) const;

		virtual void accept(void) override;

	private slots:

		void pathClicked(void);
		void logsClicked(void);

	signals:

		void onAccepted(const QString&,
					 const QString&,
					 const QVariantMap&,
					 bool = false);

};

#endif // IMPORTDIALOG_HPP
