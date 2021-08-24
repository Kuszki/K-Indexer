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

#include "mainwindow.hpp"

#include <QApplication>
#include <QLibraryInfo>
#include <QTranslator>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	a.setApplicationName("K-Indexer");
	a.setOrganizationName("Łukasz \"Kuszki\" Dróżdż");
	a.setOrganizationDomain("https://github.com/Kuszki");
	a.setApplicationVersion("1.0");

	QTranslator qtTranslator;
	qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
	a.installTranslator(&qtTranslator);

	QTranslator baseTranslator;
	baseTranslator.load("qtbase_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
	a.installTranslator(&baseTranslator);

	QTranslator appTranslator;
	appTranslator.load("K-Indexer_" + QLocale::system().name());
	a.installTranslator(&appTranslator);

	MainWindow w;
	w.show();

	return a.exec();
}
