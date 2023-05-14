QT       += core gui winextras axcontainer

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

INCLUDEPATH += ./include
LIBS += -L$$(PWD)\lib -lssh.dll
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    core/basedir.cpp \
    core/clipboard.cpp \
    core/contextmenu.cpp \
    core/dirfavorite.cpp \
    core/dirhistory.cpp \
    core/filemanager.cpp \
    core/filetransfer.cpp \
    dialog/aboutdialog.cpp \
    dialog/connectdialog.cpp \
    dialog/fileoperateconfirmdialog.cpp \
    dialog/fileprogressdialog.cpp \
    dialog/permissionsdialog.cpp \
    dialog/propertydialog.cpp \
    main.cpp \
    mainwindow.cpp \
    model/localdirmodel.cpp \
    model/remotedirmodel.cpp \
    model/treeitem.cpp \
    model/treemodel.cpp \
    sftp/sftpsession.cpp \
    ssh/channel.cpp \
    ssh/dir.cpp \
    ssh/file.cpp \
    ssh/fileinfo.cpp \
    ssh/session.cpp \
    ssh/sftp.cpp \
    util/utils.cpp \
    view/localdirdockwidget.cpp \
    view/panelwidget.cpp \
    view/remotedockwidget.cpp \
    view/titlebarwidget.cpp \
    view/toolbuttons.cpp \
    view/treeview.cpp

HEADERS += \
    core/basedir.h \
    core/clipboard.h \
    core/connecttype.h \
    core/contextmenu.h \
    core/dirfavorite.h \
    core/dirhistory.h \
    core/filemanager.h \
    core/filename.h \
    core/filetransfer.h \
    core/sshsettings.h \
    dialog/aboutdialog.h \
    dialog/connectdialog.h \
    dialog/fileoperateconfirmdialog.h \
    dialog/fileprogressdialog.h \
    dialog/permissionsdialog.h \
    dialog/propertydialog.h \
    mainwindow.h \
    model/localdirmodel.h \
    model/remotedirmodel.h \
    model/treeitem.h \
    model/treemodel.h \
    sftp/sftpsession.h \
    ssh/channel.h \
    ssh/dir.h \
    ssh/file.h \
    ssh/fileinfo.h \
    ssh/session.h \
    ssh/sftp.h \
    ssh/sshprivate.h \
    util/utils.h \
    view/localdirdockwidget.h \
    view/panelwidget.h \
    view/remotedockwidget.h \
    view/titlebarwidget.h \
    view/toolbuttons.h \
    view/treeview.h

FORMS += \
    dialog/aboutdialog.ui \
    dialog/connectdialog.ui \
    dialog/fileoperateconfirmdialog.ui \
    dialog/fileprogressdialog.ui \
    dialog/permissionsdialog.ui \
    dialog/propertydialog.ui \
    mainwindow.ui \
    view/localdirdockwidget.ui \
    view/panelwidget.ui \
    view/remotedockwidget.ui \
    view/titlebarwidget.ui \
    view/toolbuttons.ui

RC_ICONS = FlyScp.ico
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    FlyScp.qrc
