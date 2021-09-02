/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Firebird database editor                                               *
 *  Copyright (C) 2016  Łukasz "Kuszki" Dróżdż  lukasz.kuszki@gmail.com    *
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

#include "filterwidget.hpp"
#include "ui_filterwidget.h"

const QStringList FilterWidget::Operators =
{
	"=", "<>", ">=", ">", "<=", "<", "BETWEEN",
	"LIKE", "NOT LIKE",
	"IN", "NOT IN",
	"IS NULL", "IS NOT NULL"
};

FilterWidget::FilterWidget(const QVariantMap& field, QWidget* Parent)
: QWidget(Parent), ui(new Ui::FilterWidget)
{
	ui->setupUi(this); setParameters(field);

	connect(ui->Field, &QCheckBox::toggled,
		   this, &FilterWidget::onStatusChanged);
}

FilterWidget::~FilterWidget(void)
{
	delete ui;
}

QPair<QString, QVariant> FilterWidget::getBinding(void) const
{
	const bool IS = ui->Operator->currentText() == "IS NULL" || ui->Operator->currentText() == "IS NOT NULL";
	const bool IN = ui->Operator->currentText() == "IN" || ui->Operator->currentText() == "NOT IN";
	const bool BT = ui->Operator->currentText() == "BETWEEN";

	if (IS)
	{
		return qMakePair(QString("%1 %2").arg(objectName()).arg(ui->Operator->currentText()), QVariant());
	}
	else if (IN)
	{
		const auto Value = getValue().toList();

		if (Value.isEmpty()) return qMakePair(QString("%1 IS NULL").arg(objectName()), QVariant());

		QString Key = QString("%1 %2 (?").arg(objectName()).arg(ui->Operator->currentText());

		for (int i = 1; i < Value.size(); ++i) Key.append(", ?");

		return qMakePair(Key.append(")"), Value);
	}
	else if (BT)
	{
		return qMakePair(QString("%1 BETWEEN ? AND ?").arg(objectName()), getValue());
	}
	else
	{
		return qMakePair(QString("%1 %2 ?").arg(objectName()).arg(ui->Operator->currentText()), getValue());
	}
}

QString FilterWidget::getCondition(void) const
{
	const bool IS = ui->Operator->currentText() == "IS NULL" || ui->Operator->currentText() == "IS NOT NULL";
	const bool IN = ui->Operator->currentText() == "IN" || ui->Operator->currentText() == "NOT IN";
	const bool BT = ui->Operator->currentText() == "BETWEEN";

	if (IS)
	{
		return QString("%1 %2")
				.arg(objectName())
				.arg(ui->Operator->currentText());
	}
	else if (IN)
	{
		return QString("%1 %2 ('%3')")
				.arg(objectName())
				.arg(ui->Operator->currentText())
				.arg(getValue().toStringList().join("', '"));
	}
	else if (BT)
	{
		return QString("%1 BETWEEN '%2' AND '%3'")
				.arg(objectName())
				.arg(getValue().toList()[0].toString())
				.arg(getValue().toList()[1].toString());
	}
	else
	{
		return QString("%1 %2 '%3'")
				.arg(objectName())
				.arg(ui->Operator->currentText())
				.arg(getValue().toString());
	}
}

QVariant FilterWidget::getValue(void) const
{
	if (ui->Operator->currentText() == "IS NULL" || ui->Operator->currentText() == "IS NOT NULL") return QVariant();

	if (ui->Operator->currentText() == "IN" || ui->Operator->currentText() == "NOT IN" || ui->Operator->currentText() == "BETWEEN")
	{
		if (auto W = dynamic_cast<QComboBox*>(Simple))
		{
			auto M = dynamic_cast<QStandardItemModel*>(W->model());

			QVariantList Checked;

			for (int i = 1; i < M->rowCount(); ++i)
				if (M->item(i)->checkState() == Qt::Checked)
				{
					Checked << M->item(i)->data();
				}

			return Checked;
		}
		else if (auto W = dynamic_cast<QSpinBox*>(Simple))
		{
			if (auto N = dynamic_cast<QSpinBox*>(Widget))
			{
				return QList<QVariant>() << N->value() << W->value();
			}
			else return QVariant();
		}
		else if (auto W = dynamic_cast<QDoubleSpinBox*>(Simple))
		{
			if (auto N = dynamic_cast<QDoubleSpinBox*>(Widget))
			{
				return QList<QVariant>() << N->value() << W->value();
			}
			else return QVariant();
		}
		else if (auto W = dynamic_cast<QDateEdit*>(Simple))
		{
			if (auto N = dynamic_cast<QDateEdit*>(Widget))
			{
				return QList<QVariant>() << N->date().toString("dd.MM.yyyy") << W->date().toString("dd.MM.yyyy");
			}
			else return QVariant();
		}
		else if (auto W = dynamic_cast<QDateTimeEdit*>(Simple))
		{
			if (auto N = dynamic_cast<QDateTimeEdit*>(Widget))
			{
				return QList<QVariant>() << N->date().toString("dd.MM.yyyy hh:mm:ss") << W->date().toString("dd.MM.yyyy hh:mm:ss");
			}
			else return QVariant();
		}
		else return QVariant();
	}
	else if (auto W = dynamic_cast<QComboBox*>(Widget))
	{
		if (W->property("MASK").toBool())
		{
			auto M = dynamic_cast<QStandardItemModel*>(W->model());

			int Mask = 0;

			for (int i = 1; i < M->rowCount(); ++i)
				if (M->item(i)->checkState() == Qt::Checked)
				{
					Mask |= 1 << M->item(i)->data().toInt();
				}

			return Mask;
		}
		else
		{
			const auto Text = W->currentText();
			const int Index = W->findText(Text);

			if (Index == -1) return Text;
			else return W->itemData(Index);
		}

	}
	else if (auto W = dynamic_cast<QLineEdit*>(Widget))
	{
		return W->text().trimmed().replace("'", "''");
	}
	else if (auto W = dynamic_cast<QSpinBox*>(Widget))
	{
		return W->value();
	}
	else if (auto W = dynamic_cast<QDoubleSpinBox*>(Widget))
	{
		return W->value();
	}
	else if (auto W = dynamic_cast<QDateEdit*>(Widget))
	{
		return W->date().toString("dd.MM.yyyy");
	}
	else if (auto W = dynamic_cast<QDateTimeEdit*>(Widget))
	{
		return W->dateTime().toString("dd.MM.yyyy hh:mm:ss");
	}
	else return QVariant();
}

QString FilterWidget::getLabel(void) const
{
	return ui->Field->text();
}

void FilterWidget::operatorChanged(const QString& Name)
{
	const bool IS = Name == "IS NULL" || Name == "IS NOT NULL";
	const bool IN = Name == "IN" || Name == "NOT IN";
	const bool BT = Name == "BETWEEN";

	if (Widget) Widget->setVisible(BT || (!IS && (!IN || !Simple)));
	if (Simple) Simple->setVisible(BT || (!IS && IN));
}

void FilterWidget::editFinished(void)
{
	emit onValueUpdate(objectName(), getValue());
}

void FilterWidget::resetIndex(void)
{
	if (auto C = qobject_cast<QComboBox*>(sender())) C->setCurrentIndex(0);
}

void FilterWidget::setParameters(const QVariantMap& Field)
{
	const QString fname = Field.value("name").toString();
	const QString flabel = Field.value("label", fname).toString();
	const QVariantMap fdict = Field.value("dict").toMap();
	const int ftype = Field.value("type").toInt();

	ui->Field->setText(flabel); ui->Field->setToolTip(fname);
	ui->Operator->clear(); QStringList Operators = FilterWidget::Operators;

	if (Simple) Simple->deleteLater(); Simple = nullptr;
	if (Widget) Widget->deleteLater(); Widget = nullptr;

	if (!fdict.isEmpty() || ftype == QVariant::Bool)
	{
		Operators.removeOne(">"); Operators.removeOne("<");
		Operators.removeOne(">="); Operators.removeOne("<=");
	}

	if (ftype != QVariant::String)
	{
		Operators.removeOne("LIKE"); Operators.removeOne("NOT LIKE");
	}

	if (ftype != QVariant::Int && ftype != QVariant::Double &&
	    ftype != QVariant::Date && ftype != QVariant::DateTime)
	{
		Operators.removeOne("BETWEEN");
	}

	if (!fdict.isEmpty())
	{
		auto Model = new QStandardItemModel(fdict.size(), 1);
		auto Item = new QStandardItem(tr("Select values")); int j = 0;

		for (auto i = fdict.constBegin(); i != fdict.constEnd(); ++i)
		{
			auto Item = new QStandardItem(i.key());

			Item->setData(i.value());
			Item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
			Item->setCheckState(Qt::Unchecked);

			Model->setItem(j++, Item);
		}

		Model->sort(0);
		Item->setFlags(Qt::ItemIsEnabled);
		Model->insertRow(0, Item);

		auto Combo = new QComboBox(this); Widget = Combo;
		auto Mixed = new QComboBox(this); Simple = Mixed;

		Combo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);

		for (auto i = fdict.constBegin(); i != fdict.constEnd(); ++i)
		{
			Combo->addItem(i.key(), i.value());
		}

		Combo->model()->sort(0);
		Combo->setEditable(true);
		Combo->setProperty("MASK", false);

		Model->setParent(Widget);
		Mixed->setModel(Model);

		connect(Mixed, &QComboBox::currentTextChanged, this, &FilterWidget::resetIndex);

		Datatype = QVariant::Int;
	}
	else
	{
		switch (ftype)
		{
			case QVariant::Int:
			{
				auto SpinA = new QSpinBox(this); Widget = SpinA;
				auto SpinB = new QSpinBox(this); Simple = SpinB;

				SpinA->setSingleStep(1);
				SpinA->setRange(INT32_MIN, INT32_MAX);

				SpinB->setSingleStep(1);
				SpinB->setRange(INT32_MIN, INT32_MAX);

				Datatype = QVariant::Int;
			}
			break;
			case QVariant::Bool:
			{
				auto Combo = new QComboBox(this); Widget = Combo;

				Combo->addItem(tr("Yes"), 1);
				Combo->addItem(tr("No"), 0);
				Combo->setProperty("MASK", false);

				Datatype = QVariant::Bool;
			}
			break;
			case QVariant::Double:
			{
				auto SpinA = new QDoubleSpinBox(this); Widget = SpinA;
				auto SpinB = new QDoubleSpinBox(this); Simple = SpinB;

				SpinA->setSingleStep(1.0);
				SpinA->setRange(-Q_INFINITY, Q_INFINITY);

				SpinB->setSingleStep(1.0);
				SpinB->setRange(-Q_INFINITY, Q_INFINITY);

				Datatype = QVariant::Double;
			}
			break;
			case QVariant::Date:
			{
				auto DateA = new QDateEdit(this); Widget = DateA;
				auto DateB = new QDateEdit(this); Simple = DateB;

				DateA->setDisplayFormat("dd.MM.yyyy");
				DateA->setCalendarPopup(true);

				DateB->setDisplayFormat("dd.MM.yyyy");
				DateB->setCalendarPopup(true);

				Datatype = QVariant::Date;
			}
			break;
			case QVariant::DateTime:
			{
				auto DateA = new QDateTimeEdit(this); Widget = DateA;
				auto DateB = new QDateTimeEdit(this); Simple = DateB;

				DateA->setDisplayFormat("dd.MM.yyyy hh:mm:ss");
				DateA->setCalendarPopup(true);

				DateB->setDisplayFormat("dd.MM.yyyy hh:mm:ss");
				DateB->setCalendarPopup(true);

				Datatype = QVariant::DateTime;
			}
			break;
			default:
			{
				auto Edit = new QLineEdit(this); Widget = Edit;

				Edit->setClearButtonEnabled(true);

				Datatype = QVariant::String;

				if (ftype != QVariant::String)
				{
					const int Index = Operators.indexOf("IS NULL");

					Operators.insert(Index, "NOT LIKE");
					Operators.insert(Index, "LIKE");
				}
			}
			break;
		}
	}

	if (!Simple && !qobject_cast<QLineEdit*>(Widget)) Simple = new QLineEdit(this);

	if (Widget)
	{
		Widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
		Widget->setEnabled(ui->Field->isChecked());

		ui->horizontalLayout->addWidget(Widget);

		connect(ui->Field, &QCheckBox::toggled, Widget, &QWidget::setEnabled);
	}

	if (Simple)
	{
		Simple->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
		Simple->setEnabled(ui->Field->isChecked());

		ui->horizontalLayout->addWidget(Simple);

		connect(ui->Field, &QCheckBox::toggled, Simple, &QWidget::setEnabled);
	}

	setObjectName(fname);
	ui->Operator->addItems(Operators);
	operatorChanged(ui->Operator->currentText());
}

void FilterWidget::setValue(const QVariant& Value)
{
	if (auto W = dynamic_cast<QComboBox*>(Simple))
	{
		auto M = dynamic_cast<QStandardItemModel*>(W->model());

		for (int i = 1; i < M->rowCount(); ++i)
		{
			const bool Checked = Value.toList().contains(M->item(i)->data());

			M->item(i)->setCheckState(Checked ? Qt::Checked : Qt::Unchecked);
		}
	}

	if (auto W = dynamic_cast<QSpinBox*>(Simple))
	{
		W->setValue(Value.toInt());
	}

	if (auto W = dynamic_cast<QDoubleSpinBox*>(Simple))
	{
		W->setValue(Value.toDouble());
	}

	if (auto W = dynamic_cast<QComboBox*>(Widget))
	{
		W->setCurrentText(Value.toString());
	}
	else if (auto W = dynamic_cast<QLineEdit*>(Widget))
	{
		W->setText(Value.toString());
	}
	else if (auto W = dynamic_cast<QSpinBox*>(Widget))
	{
		W->setValue(Value.toInt());
	}
	else if (auto W = dynamic_cast<QDoubleSpinBox*>(Widget))
	{
		W->setValue(Value.toDouble());
	}
	else if (auto W = dynamic_cast<QDateEdit*>(Widget))
	{
		W->setDate(Value.toDate());
	}
	else if (auto W = dynamic_cast<QDateTimeEdit*>(Widget))
	{
		W->setDateTime(Value.toDateTime());
	}
}


void FilterWidget::setChecked(bool Checked)
{
	ui->Field->setChecked(Checked);
}

bool FilterWidget::isChecked(void) const
{
	return isEnabled() && ui->Field->isChecked();
}

void FilterWidget::reset(void)
{
	setChecked(false); setValue(QVariant());
}
