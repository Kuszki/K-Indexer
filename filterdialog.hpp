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

#ifndef FILTERDIALOG_HPP
#define FILTERDIALOG_HPP

#include <QtWidgets>
#include <QtCore>
#include <QtSql>

#include "sqlhighlighter.hpp"
#include "filterwidget.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class FilterDialog; }
QT_END_NAMESPACE

class FilterDialog : public QDialog
{

		Q_OBJECT

	private:

		Ui::FilterDialog* ui;
		QSqlDatabase& database;

	public:

		explicit FilterDialog(QSqlDatabase& db,
						  QWidget* Parent = nullptr);
		virtual ~FilterDialog(void) override;

		QString getFilterRules(void) const;
		QString getAdvancedRules(void) const;
		QString getQueryRules(void) const;

		QVariantMap getFieldsRules(void) const;

	private slots:

		void resetButtonClicked(void);

		void helperIndexChanged(int Index);

		void tooltipShowRequest(const QModelIndex& Index);
		void variablePasteRequest(const QModelIndex& Index);

	public slots:

		virtual void accept(void) override;

		void setFields(const QVariantList& list,
					const QVariantList& queries);

		void setupDialog(int user);

	signals:

		void onFiltersUpdate(const QString&);

};

#endif // FILTERDIALOG_HPP
