#include "optionsdialog.h"
#include "util/utils.h"
#include "core/optionsmanager.h"
#include "ui_optionsdialog.h"
#include <QPixmap>
#include <QIcon>

int const LayoutPage    = 0;
int const DisplayPage   = 1;
int const IconsPage     = 2;
int const FontPage      = 3;
int const ColorPage     = 4;
int const LangPage      = 5;
int const OperationPage = 6;

OptionsDialog::OptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionsDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui->treeWidget->clear();

    QTreeWidgetItem* layout = new QTreeWidgetItem(QStringList() << "Layout");
    QTreeWidgetItem* display = new QTreeWidgetItem(QStringList() << "Display");
    QTreeWidgetItem* icons = new QTreeWidgetItem(QStringList() << "Icons");
    QTreeWidgetItem* font = new QTreeWidgetItem(QStringList() << "Font");
    QTreeWidgetItem* color = new QTreeWidgetItem(QStringList() << "Color");
    QTreeWidgetItem* language = new QTreeWidgetItem(QStringList() << "Language");
    QTreeWidgetItem* operation = new QTreeWidgetItem(QStringList() << "Operation");

    layout->setData(0, Qt::UserRole, LayoutPage);
    display->setData(0, Qt::UserRole, DisplayPage);
    icons->setData(0, Qt::UserRole, IconsPage);
    font->setData(0, Qt::UserRole, FontPage);
    color->setData(0, Qt::UserRole, ColorPage);
    language->setData(0, Qt::UserRole, LangPage);
    operation->setData(0, Qt::UserRole, OperationPage);
    display->addChild(icons);
    display->addChild(font);
    display->addChild(color);
    display->addChild(language);
    ui->treeWidget->addTopLevelItem(layout);
    ui->treeWidget->addTopLevelItem(display);
    ui->treeWidget->addTopLevelItem(operation);
    display->setExpanded(true);


    ui->treeWidget->setCurrentItem(layout);

    setStyleForLabel(ui->labelLayout);
    setStyleForLabel(ui->labelDisplay);
    setStyleForLabel(ui->labelIcons);
    setStyleForLabel(ui->labelFont);
    setStyleForLabel(ui->labelColor);
    setStyleForLabel(ui->labelLanguage);
    setStyleForLabel(ui->labelOperation);

    updateOptionToUI();

    createConnects();
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}

void OptionsDialog::setStyleForLabel(QLabel* label)
{
    label->setStyleSheet("QLabel{"
                  "background-color: #FFBFCDDB;"
                  "margin: 0px;"
                  "padding-left: 2px;"
                  "border-width: 0px;"
                  "border-radius: 2px;}");
}

void OptionsDialog::createConnects()
{
    connect(ui->treeWidget, &QTreeWidget::itemSelectionChanged, this, [=](){
        QTreeWidgetItem *item = ui->treeWidget->currentItem();
        if(item)
            ui->stackedWidget->setCurrentIndex(item->data(0, Qt::UserRole).toInt());
    });
    connect(ui->btnApply, &QPushButton::clicked, this, [=](){
        if(ui->stackedWidget->currentIndex() == LayoutPage) {
            updateUIToLayoutOption();
            emit layoutOptionChanged();
        }
        else if(ui->stackedWidget->currentIndex() == DisplayPage) {
            updateUIToDisplayOption();
            emit displayOptionChanged();
        }
        else if(ui->stackedWidget->currentIndex() == IconsPage) {
            updateUIToIconsOption();
            emit iconsOptionChanged();
        }
        else if(ui->stackedWidget->currentIndex() == FontPage) {
            updateUIToFontOption();
            emit fontOptionChanged();
        }
        else if(ui->stackedWidget->currentIndex() == ColorPage) {
            updateUIToColorOption();
            emit colorOptionChanged();
        }
        else if(ui->stackedWidget->currentIndex() == LangPage) {
            updateUIToLanguageOption();
            emit langOptionChanged();
        }
        else if(ui->stackedWidget->currentIndex() == OperationPage) {
            updateUIToOperationOption();
            emit langOptionChanged();
        }
    });
}

void OptionsDialog::updateOptionToUI()
{
    updateLayoutOptionToUI();
    updateDisplayOptionToUI();
    updateIconsOptionToUI();
    updateUIToFontOption();
    updateColorOptionToUI();
    updateLanguageOptionToUI();
    updateOperationOptionToUI();
}

void OptionsDialog::updateUIToOption()
{
    updateUIToLayoutOption();
    updateUIToDisplayOption();
    updateUIToIconsOption();
    updateUIToFontOption();
    updateUIToColorOption();
    updateUIToLanguageOption();
    updateUIToOperationOption();
}

void OptionsDialog::updateLayoutOptionToUI()
{
    LayoutOption const& option = theOptionManager.layoutOption();

    ui->cbShowToolBar->setChecked(option.isShowToolBar);
    ui->cbShowDriveButtons->setChecked(option.isShowDriveButtons);
    ui->cbShowStatusBar->setChecked(option.isShowStatusBar);
    ui->gbShowCurrentDir->setChecked(option.isShowCurrentDir);
    ui->cbShowDeskNavigationButton->setChecked(option.isShowDeskNavigationButton);
    ui->cbShowFavoriteButton->setChecked(option.isShowFavoriteButton);
    ui->cbShowHistoryButton->setChecked(option.isShowHistoryButton);
    ui->cbShowSortHeader->setChecked(option.isShowSortHeader);
    ui->cbShowCommandLine->setChecked(option.isShowCommandLine);
    ui->cbShowFunctionKeyButtons->setChecked(option.isShowFunctionKeyButtons);
    if(option.showStyle == "WindowsVista")
        ui->rbVista->setChecked(true);
    else if(option.showStyle == "Windows")
        ui->rbWindow->setChecked(true);
    else if(option.showStyle == "Fusion")
        ui->rbFusion->setChecked(true);
}

void OptionsDialog::updateUIToLayoutOption()
{
    LayoutOption option;

    option.isShowToolBar = ui->cbShowToolBar->isChecked();
    option.isShowDriveButtons = ui->cbShowDriveButtons->isChecked();
    option.isShowStatusBar = ui->cbShowStatusBar->isChecked();
    option.isShowCurrentDir = ui->gbShowCurrentDir->isChecked();
    option.isShowDeskNavigationButton = ui->cbShowDeskNavigationButton->isChecked();
    option.isShowFavoriteButton = ui->cbShowFavoriteButton->isChecked();
    option.isShowHistoryButton = ui->cbShowHistoryButton->isChecked();
    option.isShowSortHeader = ui->cbShowSortHeader->isChecked();
    option.isShowFunctionKeyButtons = ui->cbShowFunctionKeyButtons->isChecked();
    if(ui->rbVista->isChecked())
        option.showStyle = "WindowsVista";
    else if(ui->rbWindow->isChecked())
        option.showStyle = "Windows";
    else if(ui->rbFusion->isChecked())
        option.showStyle = "Fusion";

    theOptionManager.setLayoutOption(option);
}

void OptionsDialog::updateDisplayOptionToUI()
{
    DisplayOption const& option = theOptionManager.displayOption();

    ui->cbShowHideAndSystemFile->setChecked(option.isShowHideAndSystemFile);
    ui->cbShowParentDirInRootDrive->setChecked(option.isShowParentDirInRootDrive);
    ui->rbDirSortByName->setChecked(option.isDirSortByName);
    ui->cbShowToolbarTooltips->setChecked(option.isShowToolbarTooltips);
    ui->cbShowDriveTooltips->setChecked(option.isShowDriveTooltips);
    ui->cbShowFilenameTooltips->setChecked(option.isShowFilenameTooltips);
}

void OptionsDialog::updateUIToDisplayOption()
{
    DisplayOption option;

    option.isShowHideAndSystemFile = ui->cbShowHideAndSystemFile->isChecked();
    option.isShowParentDirInRootDrive = ui->cbShowParentDirInRootDrive->isChecked();
    option.isDirSortByName = ui->rbDirSortByName->isChecked();
    option.isShowToolbarTooltips = ui->cbShowToolbarTooltips->isChecked();
    option.isShowDriveTooltips = ui->cbShowDriveTooltips->isChecked();
    option.isShowFilenameTooltips = ui->cbShowFilenameTooltips->isChecked();

    theOptionManager.setDialayOption(option);
}

void OptionsDialog::updateIconsOptionToUI()
{
    IconsOption const& option = theOptionManager.iconOption();

    ui->rbShowAllIconIncludeExeAndLink->setChecked(option.isShowAllIconIncludeExeAndLink);
    ui->cbShowExeLinkNotOnUDisk->setChecked(option.isShowExeLinkNotOnUDisk);
    ui->cbShowExeLinkNotOnNet->setChecked(option.isShowExeLinkNotOnNet);
    ui->rbShowAllIcon->setChecked(option.isShowAllIcon);
    ui->rbShowStandardIcon->setChecked(option.isShowStandardIcon);
    ui->rbNoShowIcon->setChecked(option.isNoShowIcon);
    ui->cbShowIconForFilesystem->setChecked(option.isShowIconForFilesystem);
    ui->cbShowIconForVirtualFolder->setChecked(option.isShowIconForVirtualFolder);
    ui->cbShowOverlayIcon->setChecked(option.isShowOverlayIcon);

    ui->cbbFileIconSize->clear();
    ui->cbbFileIconSize->addItem("16x16", 16);
    ui->cbbFileIconSize->addItem("32x32", 32);

    ui->cbbToolBarIconSize->clear();
    ui->cbbToolBarIconSize->addItem("16x16", 16);
    ui->cbbToolBarIconSize->addItem("20x20", 20);
    ui->cbbToolBarIconSize->addItem("24x24", 24);
    ui->cbbToolBarIconSize->addItem("32x32", 32);
    ui->labelFileIcon->setPixmap(Utils::dirIcon().pixmap(QSize(32, 32)));
    ui->labelToolBarIcon->setPixmap(QIcon(":/image/control.png").pixmap(QSize(32, 32)));
}

void OptionsDialog::updateUIToIconsOption()
{
    IconsOption option;

    option.isShowAllIconIncludeExeAndLink = ui->rbShowAllIconIncludeExeAndLink->isChecked();
    option.isShowExeLinkNotOnUDisk = ui->cbShowExeLinkNotOnUDisk->isChecked();
    option.isShowExeLinkNotOnNet = ui->cbShowExeLinkNotOnNet->isChecked();
    option.isShowAllIcon = ui->rbShowAllIcon->isChecked();
    option.isShowStandardIcon = ui->rbShowStandardIcon->isChecked();
    option.isNoShowIcon = ui->rbNoShowIcon->isChecked();
    option.isShowIconForFilesystem = ui->cbShowIconForFilesystem->isChecked();
    option.isShowIconForVirtualFolder = ui->cbShowIconForVirtualFolder->isChecked();
    option.isShowOverlayIcon = ui->cbShowOverlayIcon->isChecked();

    option.fileIconSize = ui->cbbFileIconSize->currentData().toInt();
    option.toolbarIconSize = ui->cbbToolBarIconSize->currentData().toInt();

    theOptionManager.setIconsOption(option);
}

void OptionsDialog::updateFontOptionToUI()
{
    FontOption const& option = theOptionManager.fontOption();
}

void OptionsDialog::updateUIToFontOption()
{
    FontOption option;
    theOptionManager.setFontOption(option);
}

void OptionsDialog::updateColorOptionToUI()
{
    ColorOption const& option = theOptionManager.colorOption();
}

void OptionsDialog::updateUIToColorOption()
{
    ColorOption option;
    theOptionManager.setColorOption(option);
}

void OptionsDialog::updateLanguageOptionToUI()
{
    LanguageOption const& option = theOptionManager.languageOption();
}

void OptionsDialog::updateUIToLanguageOption()
{
    LanguageOption option;
    theOptionManager.setLanguageOption(option);
}

void OptionsDialog::updateOperationOptionToUI()
{
    OperationOption const& option = theOptionManager.operationOption();

    ui->cbOnlyOneMainProgramRun->setChecked(option.isOnlyOneMainProgramRun);
    ui->cbGoToRootWhenChangeDrive->setChecked(option.isGoToRootWhenChangeDrive);
    ui->cbSelectFileNameWhenRenaming->setChecked(option.isSelectFileNameWhenRenaming);
    ui->rbLeftButtonSelect->setChecked(option.isLeftButtonSelect);
    ui->rbRightButtonSelect->setChecked(!option.isLeftButtonSelect);
}

void OptionsDialog::updateUIToOperationOption()
{
    OperationOption option;

    option.isOnlyOneMainProgramRun = ui->cbOnlyOneMainProgramRun->isChecked();
    option.isGoToRootWhenChangeDrive = ui->cbGoToRootWhenChangeDrive->isChecked();
    option.isSelectFileNameWhenRenaming = ui->cbSelectFileNameWhenRenaming->isChecked();
    option.isLeftButtonSelect = ui->rbLeftButtonSelect->isChecked();
    theOptionManager.setOperationOption(option);
}

