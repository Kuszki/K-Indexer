QT      += core gui sql widgets concurrent pdf
CONFIG  += c++19

SOURCES += \
    aboutdialog.cpp \
    connectdialog.cpp \
    exportdialog.cpp \
    filterdialog.cpp \
    filterwidget.cpp \
    imagedock.cpp \
    importdialog.cpp \
    itemmodel.cpp \
    itemsdock.cpp \
    main.cpp \
    mainwindow.cpp \
    metadock.cpp \
    scandialog.cpp \
    sqleditordialog.cpp \
    sqlhighlighter.cpp \
    summarydock.cpp \
    threadworker.cpp

HEADERS += \
    aboutdialog.hpp \
    connectdialog.hpp \
    exportdialog.hpp \
    filterdialog.hpp \
    filterwidget.hpp \
    imagedock.hpp \
    importdialog.hpp \
    itemmodel.hpp \
    itemsdock.hpp \
    mainwindow.hpp \
    metadock.hpp \
    scandialog.hpp \
    sqleditordialog.hpp \
    sqlhighlighter.hpp \
    summarydock.hpp \
    threadworker.hpp

FORMS += \
    aboutdialog.ui \
    connectdialog.ui \
    exportdialog.ui \
    filterdialog.ui \
    filterwidget.ui \
    imagedock.ui \
    importdialog.ui \
    itemsdock.ui \
    mainwindow.ui \
    metadock.ui \
    scandialog.ui \
    sqleditordialog.ui \
    summarydock.ui

TRANSLATIONS += \
    K-Indexer_pl.ts

RESOURCES += \
    resources.qrc
