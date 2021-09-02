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

#ifndef SQLHIGHLIGHTER_HPP
#define SQLHIGHLIGHTER_HPP

#include <QtWidgets>
#include <QtCore>

class SqlHighlighter : public QSyntaxHighlighter
{

		Q_OBJECT

	public: enum STYLE
	{
		NUMBERS,
		KEYWORDS,
		OPERATORS,
		STRINGS,
		MATHS,
		COMMENTS
	};

	protected:

		struct SqlHighlighterRule
		{
			QTextCharFormat Format;
			QRegExp Expresion;
		};

		QMap<STYLE, SqlHighlighterRule> Rules;

		virtual void highlightBlock(const QString& Text) override;

	public:

		static const QStringList SqlKeywords;
		static const QStringList SqlOperators;
		static const QStringList SqlBuiltin;

		explicit SqlHighlighter(QTextDocument* Parent);
		virtual ~SqlHighlighter(void) override;

		void SetFormat(STYLE Style, const QTextCharFormat& Format);

		QTextCharFormat GetFormat(STYLE Style) const;

		static QStandardItemModel* getSqlHelperModel(QObject* Parent,
					const QVariantList& Variables = QVariantList());

};

#endif // SQLHIGHLIGHTER_HPP
