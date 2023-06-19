#include "optionsmanager.h"
#include <QSettings>
#include <QCoreApplication>

void LayoutOption::save(QSettings & settings)
{
    settings.beginGroup("Layout");

    settings.setValue("isShowToolBar", isShowToolBar);
    settings.setValue("isShowDriveButtons", isShowDriveButtons);
    settings.setValue("isShowCurrentDir", isShowCurrentDir);
    settings.setValue("isShowDeskNavigationButton", isShowDeskNavigationButton);
    settings.setValue("isShowFavoriteButton", isShowFavoriteButton);
    settings.setValue("isShowHistoryButton", isShowHistoryButton);
    settings.setValue("isShowSortHeader", isShowSortHeader);
    settings.setValue("isShowCommandLine", isShowCommandLine);
    settings.setValue("isShowStatusBar", isShowStatusBar);
    settings.setValue("isShowFunctionKeyButtons", isShowFunctionKeyButtons);
    settings.setValue("showStyle", showStyle);

    settings.endGroup();
}

void LayoutOption::load(QSettings & settings)
{
    settings.beginGroup("Layout");

    isShowToolBar = settings.value("isShowToolBar", true).toBool();
    isShowDriveButtons = settings.value("isShowDriveButtons", true).toBool();
    isShowCurrentDir = settings.value("isShowCurrentDir", true).toBool();
    isShowDeskNavigationButton = settings.value("isShowDeskNavigationButton", true).toBool();
    isShowFavoriteButton = settings.value("isShowFavoriteButton", true).toBool();
    isShowHistoryButton = settings.value("isShowHistoryButton", true).toBool();
    isShowSortHeader = settings.value("isShowSortHeader", true).toBool();
    isShowCommandLine = settings.value("isShowCommandLine", true).toBool();
    isShowFunctionKeyButtons = settings.value("isShowFunctionKeyButtons", true).toBool();
    showStyle = settings.value("showStyle", "WindowsVista").toString();

    settings.endGroup();
}

void DisplayOption::save(QSettings & settings)
{
    settings.beginGroup("Dialay");
    settings.setValue("isShowHideAndSystemFile", isShowHideAndSystemFile);
    settings.setValue("isShowParentDirInRootDrive", isShowParentDirInRootDrive);
    settings.setValue("isDirSortByName", isDirSortByName);
    settings.setValue("isShowToolbarTooltips", isShowToolbarTooltips);
    settings.setValue("isShowDriveTooltips", isShowDriveTooltips);
    settings.setValue("isShowFilenameTooltips", isShowFilenameTooltips);
    settings.endGroup();
}

void DisplayOption::load(QSettings & settings)
{
    settings.beginGroup("Dialay");

    isShowHideAndSystemFile = settings.value("isShowToolBar", true).toBool();
    isShowParentDirInRootDrive = settings.value("isShowParentDirInRootDrive", false).toBool();
    isDirSortByName = settings.value("isDirSortByName", true).toBool();
    isShowToolbarTooltips = settings.value("isShowToolbarTooltips", true).toBool();
    isShowDriveTooltips = settings.value("isShowDriveTooltips", true).toBool();
    isShowFilenameTooltips = settings.value("isShowFilenameTooltips", true).toBool();

    settings.endGroup();
}

void IconsOption::save(QSettings & settings)
{
    settings.beginGroup("Icons");

    settings.setValue("isShowAllIconIncludeExeAndLink", isShowAllIconIncludeExeAndLink);
    settings.setValue("isShowExeLinkNotOnUDisk", isShowExeLinkNotOnUDisk);
    settings.setValue("isShowExeLinkNotOnNet", isShowExeLinkNotOnNet);
    settings.setValue("isShowAllIcon", isShowAllIcon);
    settings.setValue("isShowStandardIcon", isShowStandardIcon);
    settings.setValue("isNoShowIcon", isNoShowIcon);
    settings.setValue("isShowIconForFilesystem", isShowIconForFilesystem);
    settings.setValue("isShowIconForVirtualFolder", isShowIconForVirtualFolder);
    settings.setValue("isShowOverlayIcon", isShowOverlayIcon);
    settings.setValue("fileIconSize", fileIconSize);
    settings.setValue("toolbarIconSize", toolbarIconSize);

    settings.endGroup();
}

void IconsOption::load(QSettings & settings)
{
    settings.beginGroup("Icons");

    isShowAllIconIncludeExeAndLink = settings.value("isShowAllIconIncludeExeAndLink", true).toBool();
    isShowExeLinkNotOnUDisk = settings.value("isShowExeLinkNotOnUDisk", false).toBool();
    isShowExeLinkNotOnNet = settings.value("isShowExeLinkNotOnNet", false).toBool();
    isShowAllIcon = settings.value("isShowAllIcon", false).toBool();
    isShowStandardIcon = settings.value("isShowStandardIcon", false).toBool();
    isNoShowIcon = settings.value("isNoShowIcon", false).toBool();
    isShowIconForFilesystem = settings.value("isShowIconForFilesystem", true).toBool();
    isShowIconForVirtualFolder = settings.value("isShowIconForVirtualFolder", true).toBool();
    isShowOverlayIcon = settings.value("isShowOverlayIcon", true).toBool();
    fileIconSize = settings.value("fileIconSize", 16).toInt();
    toolbarIconSize = settings.value("toolbarIconSize", 20).toInt();

    settings.endGroup();
}

void FontInfo::save(QSettings & settings, QString const& type)
{
    settings.beginGroup(type);
    settings.setValue("name", name);
    settings.setValue("size", size);
    settings.setValue("isBold", isBold);
    settings.setValue("isItalic", isItalic);
    settings.endGroup();
}

void FontInfo::load(QSettings & settings, QString const& type)
{
    settings.beginGroup(type);
    name = settings.value("name", name).toString();
    size = settings.value("size", 9).toInt();
    isBold = settings.value("isBold", false).toBool();
    isItalic = settings.value("isItalic", false).toBool();
    settings.endGroup();
}

void FontOption::save(QSettings & settings)
{
    settings.beginGroup("Font");
    fileList.save(settings, "fileList");
    mainWindow.save(settings, "mainWindow");
    dialog.save(settings, "dialog");
    settings.endGroup();
}

void FontOption::load(QSettings & settings)
{
    settings.beginGroup("Font");
    settings.endGroup();
}

void ColorOption::save(QSettings & settings)
{
    settings.beginGroup("Color");

    settings.setValue("fontColor", fontColor);
    settings.setValue("background1Color", background1Color);
    settings.setValue("background2Color", background2Color);
    settings.setValue("markColor", markColor);
    settings.setValue("cursorColor", cursorColor);

    settings.endGroup();
}

void ColorOption::load(QSettings & settings)
{
    settings.beginGroup("Color");

    fontColor = settings.value("fontColor", fontColor).toString();
    background1Color = settings.value("background1Color", background1Color).toString();
    background2Color = settings.value("background2Color", background2Color).toString();
    markColor = settings.value("markColor", markColor).toString();
    cursorColor = settings.value("cursorColor", cursorColor).toString();

    settings.endGroup();
}

void LanguageOption::save(QSettings & settings)
{
    settings.beginGroup("Lang");
    settings.setValue("language", language);
    settings.endGroup();
}

void LanguageOption::load(QSettings & settings)
{
    settings.beginGroup("Lang");
    language = settings.value("language", language).toString();
    settings.endGroup();
}


void OperationOption::save(QSettings & settings)
{
    settings.beginGroup("Operation");

    settings.setValue("isOnlyOneMainProgramRun", isOnlyOneMainProgramRun);
    settings.setValue("isGoToRootWhenChangeDrive", isGoToRootWhenChangeDrive);
    settings.setValue("isSelectFileNameWhenRenaming", isSelectFileNameWhenRenaming);
    settings.setValue("isLeftButtonSelect", isLeftButtonSelect);

    settings.endGroup();
}

void OperationOption::load(QSettings & settings)
{
    settings.beginGroup("Operation");

    isOnlyOneMainProgramRun = settings.value("isOnlyOneMainProgramRun", false).toBool();
    isGoToRootWhenChangeDrive = settings.value("isGoToRootWhenChangeDrive", false).toBool();
    isSelectFileNameWhenRenaming = settings.value("isSelectFileNameWhenRenaming", false).toBool();
    isLeftButtonSelect = settings.value("isLeftButtonSelect", true).toBool();

    settings.endGroup();
}

OptionsManager::OptionsManager()
{
}

OptionsManager& OptionsManager::Instance()
{
    static OptionsManager theManger;
    return theManger;
}

void OptionsManager::load(QString const& name)
{
    QSettings settings(QCoreApplication::applicationName(),
                       QCoreApplication::applicationVersion());
    settings.beginGroup(name);
    layoutOption_.load(settings);
    displayOption_.load(settings);
    iconOption_.load(settings);
    fontOption_.load(settings);
    colorOption_.load(settings);
    languageOption_.load(settings);
    operationOption_.load(settings);
    settings.endGroup();
}

void OptionsManager::save(QString const& name)
{
    QSettings settings(QCoreApplication::applicationName(),
                       QCoreApplication::applicationVersion());

    settings.beginGroup(name);

    layoutOption_.save(settings);
    displayOption_.save(settings);
    iconOption_.save(settings);
    fontOption_.save(settings);
    colorOption_.save(settings);
    languageOption_.save(settings);
    operationOption_.save(settings);

    settings.endGroup();
}
