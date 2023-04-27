QT       += core gui winextras axcontainer

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    core/clipboard.cpp \
    core/contextmenu.cpp \
    core/filemanager.cpp \
    core/filetransfer.cpp \
    dialog/aboutdialog.cpp \
    dialog/connectdialog.cpp \
    dialog/fileprogressdialog.cpp \
    main.cpp \
    mainwindow.cpp \
    model/localfilemodel.cpp \
    model/remotefilemodel.cpp \
    model/treeitem.cpp \
    model/treemodel.cpp \
    util/utils.cpp \
    view/localdirdockwidget.cpp \
    view/panelwidget.cpp \
    view/remotedockwidget.cpp \
    view/treeview.cpp

HEADERS += \
    core/clipboard.h \
    core/connecttype.h \
    core/contextmenu.h \
    core/filemanager.h \
    core/filename.h \
    core/filetransfer.h \
    core/sshsettings.h \
    dialog/aboutdialog.h \
    dialog/connectdialog.h \
    dialog/fileprogressdialog.h \
    mainwindow.h \
    model/localfilemodel.h \
    model/remotefilemodel.h \
    model/treeitem.h \
    model/treemodel.h \
    util/utils.h \
    view/localdirdockwidget.h \
    view/panelwidget.h \
    view/remotedockwidget.h \
    view/treeview.h

FORMS += \
    dialog/aboutdialog.ui \
    dialog/connectdialog.ui \
    dialog/fileprogressdialog.ui \
    mainwindow.ui \
    view/localdirdockwidget.ui \
    view/panelwidget.ui \
    view/remotedockwidget.ui

RC_ICONS = FlyScp.ico
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    FlyScp.qrc
