/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the FOO module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

function Component()
{
    installer.setDefaultPageVisible(QInstaller.ComponentSelection, false);
    installer.setDefaultPageVisible(QInstaller.StartMenuSelection, false);    
}

Component.prototype.isDefault = function()
{
    return true;
}

Component.prototype.createOperations = function()
{
    try {
        // call the base create operations function
        component.createOperations();
        if (systemInfo.productType === "windows")
        {
            component.addOperation("CreateShortcut", "@TargetDir@/FlyScp.exe", "@UserStartMenuProgramsPath@/FlyScp1.0/FlyScp1.0.lnk",
                "workingDirectory=@TargetDir@", "iconPath=@TargetDir@/FlyScp.exe",
            "iconId=0", "description=Run FlyScp1.0");
            component.addOperation("CreateShortcut", "@TargetDir@/uninstall.exe", "@UserStartMenuProgramsPath@/FlyScp1.0/uninstall FlyScp1.0.lnk",
                "workingDirectory=@TargetDir@", "iconPath=@TargetDir@/uninstall.exe",
            "iconId=0", "description=Uninstall FlyScp1.0");
            component.addOperation("CreateShortcut", "@TargetDir@/FlyScp.exe", "@DesktopDir@/FlyScp1.0.lnk",
                "workingDirectory=@TargetDir@", "iconPath=@TargetDir@/FlyScp.exe",
            "iconId=0", "description=Run FlyScp1.0");
        }
        else if(systemInfo.productType === "osx")
        {
            var fileName = QDesktopServices.storageLocation(QDesktopServices.DesktopLocation) + "/FlyScp1.0";
            component.addOperation("CreateLink", fileName, "@TargetDir@/FlyScp.app");
        }
        else
        {
		    var fileName = QDesktopServices.storageLocation(QDesktopServices.DesktopLocation) + "/FlyScp1.desktop";
		    component.addOperation("CreateDesktopEntry", fileName, "Name=FlyScp1.0\nGenericName=FlyScp1.0\nExec=@TargetDir@/start\nTerminal=false\nIcon=@TargetDir@/FlyScp1.png\nType=Application\nCategoreis=KDE");
		}
    } catch (e) {
        console.log(e);
    }
}
