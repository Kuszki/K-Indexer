QT      += core gui sql widgets
CONFIG  += c++19

SOURCES += \
    aboutdialog.cpp \
    connectdialog.cpp \
    exportdialog.cpp \
    filterdialog.cpp \
    filterwidget.cpp \
    imagedock.cpp \
    importdialog.cpp \
    itemsdock.cpp \
    main.cpp \
    mainwindow.cpp \
    metadock.cpp \
    sqleditordialog.cpp \
    sqlhighlighter.cpp \
    threadworker.cpp

HEADERS += \
    aboutdialog.hpp \
    connectdialog.hpp \
    exportdialog.hpp \
    filterdialog.hpp \
    filterwidget.hpp \
    imagedock.hpp \
    importdialog.hpp \
    itemsdock.hpp \
    mainwindow.hpp \
    metadock.hpp \
    sqleditordialog.hpp \
    sqlhighlighter.hpp \
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
    sqleditordialog.ui

TRANSLATIONS += \
    K-Indexer_pl.ts

RESOURCES += \
    resources.qrc

