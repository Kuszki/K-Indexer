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

#ifndef FILTERWIDGET_HPP
#define FILTERWIDGET_HPP

#include <QtWidgets>
#include <QtCore>
#include <QtSql>

QT_BEGIN_NAMESPACE
namespace Ui { class FilterWidget; }
QT_END_NAMESPACE

class FilterWidget : public QWidget
{

		Q_OBJECT

	public:

		static const QStringList Operators;

	private:

		QVariant::Type Datatype;

		QWidget* Simple = nullptr;
		QWidget* Widget = nullptr;

		Ui::FilterWidget* ui;

	public:

		explicit FilterWidget(const QVariantMap& field,
						  QWidget* Parent = nullptr);
		virtual ~FilterWidget(void) override;

		QPair<QString, QVariant> getBinding(void) const;

		QString getCondition(void) const;
		QVariant getValue(void) const;
		QString getLabel(void) const;

	private slots:

		void operatorChanged(const QString& Name);

		void editFinished(void);
		void resetIndex(void);

	public slots:

		void setParameters(const QVariantMap& Field);
		void setValue(const QVariant& Value);
		void setChecked(bool Checked);

		bool isChecked(void) const;

		void reset(void);

	signals:

		void onValueUpdate(const QString&, const QVariant&);

		void onStatusChanged(bool);

};

#endif // FILTERWIDGET_HPP
