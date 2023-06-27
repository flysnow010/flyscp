#ifndef OPTIONSMANAGER_H
#define OPTIONSMANAGER_H
#include <QString>
#include <QFont>
#include <QSettings>

struct LayoutOption
{
    bool isShowToolBar = true;
    bool isShowDriveButtons = true;
    bool isShowCurrentDir = true;
    bool isShowDeskNavigationButton = true;
    bool isShowFavoriteButton = true;
    bool isShowHistoryButton = true;
    bool isShowSortHeader = true;
    bool isShowCommandLine = true;
    bool isShowStatusBar = true;
    bool isShowFunctionKeyButtons = true;

    QString showStyle = "windowsvista";//windowsvista Windows Fusion

    bool operator != (LayoutOption const& r) const
    {
        return isShowToolBar != r.isShowToolBar
                || isShowToolBar != r.isShowToolBar
                || isShowDriveButtons != r.isShowDriveButtons
                || isShowCurrentDir != r.isShowCurrentDir
                || isShowDeskNavigationButton != r.isShowDeskNavigationButton
                || isShowFavoriteButton != r.isShowFavoriteButton
                || isShowHistoryButton != r.isShowHistoryButton
                || isShowSortHeader != r.isShowSortHeader
                || isShowCommandLine != r.isShowCommandLine
                || isShowStatusBar != r.isShowStatusBar
                || isShowFunctionKeyButtons != r.isShowFunctionKeyButtons
                || showStyle != r.showStyle;
    }
    void save(QSettings & settings);
    void load(QSettings & settings);
};

struct DisplayOption
{
    bool isShowHideAndSystemFile = false;
    bool isShowParentDirInRootDrive = false;
    bool isDirSortByName = false;
    bool isShowToolbarTooltips = true;
    bool isShowDriveTooltips = true;
    bool isShowFilenameTooltips = true;

    bool operator != (DisplayOption const& r) const
    {
        return isShowHideAndSystemFile != r.isShowHideAndSystemFile
                || isShowParentDirInRootDrive != r.isShowParentDirInRootDrive
                || isDirSortByName != r.isDirSortByName
                || isDirSortByName != r.isDirSortByName
                || isShowToolbarTooltips != r.isShowToolbarTooltips
                || isShowDriveTooltips != r.isShowDriveTooltips
                || isShowFilenameTooltips != r.isShowFilenameTooltips;
    }
    void save(QSettings & settings);
    void load(QSettings & settings);
};

struct IconsOption
{
    bool isShowAllIconIncludeExeAndLink = true;
    bool isShowExeLinkNotOnUDisk = false;
    bool isShowExeLinkNotOnNet = false;
    bool isShowAllIcon = false;
    bool isShowStandardIcon = false;
    bool isNoShowIcon = false;
    bool isShowIconForFilesystem = true;
    bool isShowIconForVirtualFolder = true;
    bool isShowOverlayIcon = true;
    int fileIconSize = 16;//16,20,24,32
    int toolbarIconSize = 16;//16,20,24,32

    bool operator != (IconsOption const& r) const
    {
        return isShowAllIconIncludeExeAndLink != r.isShowAllIconIncludeExeAndLink
                || isShowExeLinkNotOnUDisk != r.isShowExeLinkNotOnUDisk
                || isShowExeLinkNotOnNet != r.isShowExeLinkNotOnNet
                || isShowAllIcon != r.isShowAllIcon
                || isShowStandardIcon != r.isShowStandardIcon
                || isNoShowIcon != r.isNoShowIcon
                || isShowIconForFilesystem != r.isShowIconForFilesystem
                || isShowIconForVirtualFolder != r.isShowIconForVirtualFolder
                || isShowOverlayIcon != r.isShowOverlayIcon
                || fileIconSize != r.fileIconSize
                || toolbarIconSize != r.toolbarIconSize;
    }
    void save(QSettings & settings);
    void load(QSettings & settings);
};

struct FontInfo
{
    QString name = "SimSun";
    int size = 9;
    bool isBold = false;
    bool isItalic = false;

    inline void setFont(QFont const& font)
    {
        name = font.family();
        size = font.pointSize();
        isBold = font.bold();
        isItalic = font.italic();
    }

    inline QFont font() const
    {
        QFont font(name);
        font.setPointSize(size);
        font.setBold(isBold);
        font.setItalic(isItalic);
        return font;
    }
    inline QString caption() const { return QString("%1,%2").arg(name).arg(size); }

    bool operator != (FontInfo const& r) const
    {
        return name != r.name
                || size != r.size
                || isBold != r.isBold
                || isItalic != r.isItalic;
    }
    void save(QSettings & settings, QString const& type);
    void load(QSettings & settings, QString const& type);
};
struct FontOption
{
    FontInfo fileList;
    FontInfo mainWindow;
    FontInfo dialog;

    void save(QSettings & settings);
    void load(QSettings & settings);
    bool operator != (FontOption const& r) const
    {
        return fileList != r.fileList
                || mainWindow != r.mainWindow
                || dialog != r.dialog;
    }
};

struct ColorOption
{
    QString fontColor = "#454545";
    QString background1Color = "#ffffff";
    QString background2Color = "#f9f9f9";
    QString markColor = "#C7E3FA";
    QString cursorColor = "#FF8728";

    bool operator != (ColorOption const& r) const
    {
        return fontColor != r.fontColor
                || background1Color != r.background1Color
                || background2Color != r.background2Color
                || markColor != r.markColor
                || cursorColor != r.cursorColor;
    }
    void save(QSettings & settings);
    void load(QSettings & settings);
};

struct LanguageOption
{
    QString language;
    bool operator != (LanguageOption const& r) const
    {
        return language != r.language;
    }
    void save(QSettings & settings);
    void load(QSettings & settings);
};

struct OperationOption
{
    bool isOnlyOneMainProgramRun = false;
    bool isGoToRootWhenChangeDrive = false;
    bool isSelectFileNameWhenRenaming = false;
    bool isLeftButtonSelect = true;

    bool operator != (OperationOption const& r) const
    {
        return isOnlyOneMainProgramRun != r.isOnlyOneMainProgramRun
                || isGoToRootWhenChangeDrive != r.isGoToRootWhenChangeDrive
                || isSelectFileNameWhenRenaming != r.isSelectFileNameWhenRenaming
                || isLeftButtonSelect != r.isLeftButtonSelect;
    }
    void save(QSettings & settings);
    void load(QSettings & settings);
};

class OptionsManager
{
public:
    static OptionsManager& Instance();
    void load(QString const& name);
    void save(QString const& name);

    inline void setLayoutOption(LayoutOption const& option) { layoutOption_ = option; }
    inline void setDialayOption(DisplayOption const& option) { displayOption_ = option; }
    inline void setIconsOption(IconsOption const& option) { iconOption_ = option; }
    inline void setFontOption(FontOption const& option) { fontOption_ = option; }
    inline void setColorOption(ColorOption const& option) { colorOption_ = option; }
    inline void setLanguageOption(LanguageOption const& option) { languageOption_ = option; }
    inline void setOperationOption(OperationOption const& option) { operationOption_ = option; }

    inline LayoutOption const& layoutOption() const { return layoutOption_; }
    inline DisplayOption const&  displayOption() const { return displayOption_; }
    inline IconsOption const&  iconOption() const { return iconOption_; }
    inline FontOption const& fontOption() const { return fontOption_; }
    inline ColorOption const&  colorOption() const { return colorOption_; }
    inline LanguageOption const&  languageOption() const { return languageOption_; }
    inline OperationOption const& operationOption() const { return operationOption_; }
private:
    OptionsManager();
    OptionsManager(OptionsManager const&);
    void operator =(OptionsManager const&);
    LayoutOption layoutOption_;
    DisplayOption displayOption_;
    IconsOption  iconOption_;
    FontOption   fontOption_;
    ColorOption  colorOption_;
    LanguageOption languageOption_;
    OperationOption operationOption_;
};
#define theOptionManager OptionsManager::Instance()
#endif // OPTIONSMANAGER_H
