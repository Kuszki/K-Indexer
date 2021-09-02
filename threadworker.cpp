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

#include "threadworker.hpp"

ThreadWorker::ThreadWorker(QSqlDatabase& db, QObject *parent)
	: QObject(parent)
	, database(db)
{}

ThreadWorker::~ThreadWorker(void) {}

void ThreadWorker::exportData(const QString& path, const QVariantList& users, int status, int validation, int lock, const QDateTime& from, const QDateTime& to)
{
	const char* sep = path.endsWith(".csv") ? ";" : "\t";

	QFile file(path);
	QSqlQuery query(database);

	QSet<int> indS, indV, indL;

	if (!file.open(QFile::WriteOnly | QFile::Text))
	{
		emit onFinish(tr("Unable to create output file"), 1); return;
	}
	else emit onSetup(0, 0);

	if (status == 1) query.prepare("SELECT id, user, time FROM main WHERE user IS NOT NULL");
	else if (status == 2) query.prepare("SELECT id, user, time FROM main WHERE user IS NULL");
	else query.prepare("SELECT id, user, time FROM main");

	if (query.exec()) while (query.next())
	{
		const auto time = query.value(2).toDateTime();

		const bool ok = (from.isNull() || time >= from) &&
					 (to.isNull() || time <= to);

		if (ok && users.contains(query.value(1)))
		{
			indS.insert(query.value(0).toInt());
		}
	}

	if (validation == 1) query.prepare("SELECT id FROM main WHERE id NOT IN (SELECT id FROM invalid)");
	else if (validation == 2) query.prepare("SELECT id FROM main WHERE id IN (SELECT id FROM invalid)");
	else indV = indS;

	if (validation && query.exec()) while (query.next())
	{
		indV.insert(query.value(0).toInt());
	}

	if (lock == 2) query.prepare("SELECT id FROM main WHERE id NOT IN (SELECT sheet FROM locked)");
	else if (lock == 1) query.prepare("SELECT id FROM main WHERE id IN (SELECT sheet FROM locked)");
	else indL = indS;

	if (lock && query.exec()) while (query.next())
	{
		indL.insert(query.value(0).toInt());
	}

	const QSet<int> indexes = indS & indV & indL;

	if (indexes.isEmpty())
	{
		emit onFinish(tr("No data to export"), 2); return;
	}
	else emit onSetup(0, indexes.size());

	query.prepare("SELECT * FROM main");

	if (query.exec())
	{
		const auto record = query.record();
		const int count = record.count();

		int step = 0;

		for (int i = 0; i < count; ++i)
		{
			file.write(record.fieldName(i)
					 .toLocal8Bit());

			file.write(i == count - 1 ? "\n" : sep);
		}

		while (query.next())
			if (indexes.contains(query.value(0).toInt()))
			{
				for (int i = 0; i < count; ++i)
				{
					file.write(query.value(i)
							 .toString()
							 .toUtf8()
							 .toPercentEncoding());

					file.write(i == count - 1 ? "\n" : sep);
				}

				emit onProgress(++step);
			}

		emit onFinish(tr("Exported %n row(s)", nullptr, step));
	}
	else emit onFinish(tr("Unable to fetch data"), 1);
}

void ThreadWorker::importData(const QString& path, const QString& logs, QVariantMap map, bool header)
{
	const char sep = path.endsWith(".csv") ? ';' : '\t';
	const int keyID = map.take("path").toInt();

	const QSqlRecord record = database.record("main");
	const QStringList keys = map.keys();

	QFile file(path), logfile(logs);
	QSqlQuery query(database);
	QTextStream logstream(&logfile);

	QMap<QString, QStringList> rows;
	QMap<QString, int> found;
	QStringList sets;

	int step = 0, total = 0;

	if (!logs.isEmpty()) logfile.open(QFile::WriteOnly | QFile::Text);

	if (!file.open(QFile::ReadOnly | QFile::Text))
	{
		emit onFinish(tr("Unable to open input file"), 1); return;
	}
	else emit onSetup(0, 0);

	if (header) file.readLine();

	while (!file.atEnd())
	{
		QByteArrayList array = file.readLine()
						   .trimmed()
						   .split(sep);



		QStringList line;

		for (const auto& col : array)
			line.append(
				QString::fromUtf8(
					QByteArray::fromPercentEncoding(col)));

		QString name = line.value(keyID);

		if (!name.isEmpty()) rows.insert(name, line);
	}

	if (rows.isEmpty())
	{
		emit onFinish(tr("No data to import"), 2); return;
	}

	query.prepare("SELECT id, path FROM main");

	if (query.exec()) while (query.next())
	{
		found.insert(query.value(1).toString(),
				   query.value(0).toInt());
	}

	for (int i = 0; i < keys.size(); ++i)
	{
		sets.append(QString("%1 = ?").arg(keys[i]));
	}

	if (!rows.isEmpty()) emit onSetup(0, rows.size());

	query.prepare(QString("UPDATE main SET %1 WHERE path = ?").arg(sets.join(", ")));

	for (auto i = found.cbegin(); i != found.cend(); ++i) if (rows.contains(i.key()))
	{
		const QString name = i.key();

		for (int j = 0; j < keys.size(); ++j)
		{
			const QString col = keys[j];
			const int index = map[col].toInt();

			QVariant val = rows[name].value(index);
			val.convert(int(record.field(keys[j]).type()));

			query.addBindValue(val);
		}

		query.addBindValue(name);

		if (query.exec()) total += 1;
		else if (logfile.isOpen())
		{
			logstream << name << ": " << query.lastError().text() << Qt::endl;
		}

		emit onProgress(++step);
	}

	query.prepare(QString("INSERT INTO main (%1, path) VALUES (?%2)")
			    .arg(keys.join(", "))
			    .arg(QString(", ?").repeated(keys.size())));

	for (auto i = rows.cbegin(); i != rows.cend(); ++i) if (!found.contains(i.key()))
	{
		const QString name = i.key();

		for (int j = 0; j < keys.size(); ++j)
		{
			const QString col = keys[j];
			const int index = map[col].toInt();

			QVariant val = i.value().value(index);
			val.convert(int(record.field(keys[j]).type()));

			query.addBindValue(val);
		}

		query.addBindValue(i.key());

		if (query.exec()) total += 1;
		else if (logfile.isOpen())
		{
			logstream << name << ": " << query.lastError().text() << Qt::endl;
		}

		emit onProgress(++step);
	}

	emit onFinish(tr("Imported %n row(s)", nullptr, total));
}
