#include "optionsdialog.h"
#include "util/utils.h"

#include "ui_optionsdialog.h"
#include <QPixmap>
#include <QIcon>
#include <QFontDialog>

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
    qRegisterMetaType<LayoutOption>("LayoutOption");
    qRegisterMetaType<DisplayOption>("DisplayOption");
    qRegisterMetaType<IconsOption>("IconsOption");
    qRegisterMetaType<FontOption>("FontOption");
    qRegisterMetaType<ColorOption>("ColorOption");
    qRegisterMetaType<LanguageOption>("LanguageOption");
    qRegisterMetaType<OperationOption>("OperationOption");
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
            emit layoutOptionChanged(updateUIToLayoutOption());
        }
        else if(ui->stackedWidget->currentIndex() == DisplayPage) {
            emit displayOptionChanged(updateUIToDisplayOption());
        }
        else if(ui->stackedWidget->currentIndex() == IconsPage) {
            emit iconsOptionChanged(updateUIToIconsOption());
        }
        else if(ui->stackedWidget->currentIndex() == FontPage) {
            emit fontOptionChanged(updateUIToFontOption());
        }
        else if(ui->stackedWidget->currentIndex() == ColorPage) {
            emit colorOptionChanged(updateUIToColorOption());
        }
        else if(ui->stackedWidget->currentIndex() == LangPage) {
            emit langOptionChanged(updateUIToLanguageOption());
        }
        else if(ui->stackedWidget->currentIndex() == OperationPage) {
            emit operationOptionChanged(updateUIToOperationOption());
        }
    });

    connect(ui->cbbFileIconSize, SIGNAL(currentIndexChanged(int)), this, SLOT(setFileIconSize(int)));
    connect(ui->cbbToolBarIconSize, SIGNAL(currentIndexChanged(int)), this, SLOT(setToolBarIconSize(int)));
    connect(ui->btnFileList, &QPushButton::clicked, this, [=](){
        bool ok;
        QFont font = QFontDialog::getFont(&ok, ui->labelFileListText->font(), this);
        if (ok)
        {
            ui->labelFileListText->setFont(font);
            ui->labelFileListFont->setText(QString("%1,%2").arg(font.family()).arg(font.pointSize()));
        }
    });
    connect(ui->btnMainWindow, &QPushButton::clicked, this, [=](){
        bool ok;
        QFont font = QFontDialog::getFont(&ok, ui->labelMainWindowText->font(), this);
        if (ok)
        {
            ui->labelMainWindowText->setFont(font);
            ui->labelMainWindowFont->setText(QString("%1,%2").arg(font.family()).arg(font.pointSize()));
        }
    });
    connect(ui->btnDialog, &QPushButton::clicked, this, [=](){
        bool ok;
        QFont font = QFontDialog::getFont(&ok, ui->labelDialogText->font(), this);
        if (ok)
        {
            ui->labelDialogText->setFont(font);
            ui->labelDialogFont->setText(QString("%1,%2").arg(font.family()).arg(font.pointSize()));
        }
    });
}

void OptionsDialog::setFileIconSize(int index)
{
    int size = ui->cbbFileIconSize->itemData(index).toInt();
    ui->labelFileIcon->setPixmap(Utils::dirIcon().pixmap(QSize(size, size)));
}

void OptionsDialog::setToolBarIconSize(int index)
{
    int size = ui->cbbToolBarIconSize->itemData(index).toInt();
    ui->labelToolBarIcon->setPixmap(QIcon(":/image/control.png").pixmap(QSize(size, size)));
}

void OptionsDialog::updateOptionToUI()
{
    updateLayoutOptionToUI();
    updateDisplayOptionToUI();
    updateIconsOptionToUI();
    updateFontOptionToUI();
    updateColorOptionToUI();
    updateLanguageOptionToUI();
    updateOperationOptionToUI();
}

void OptionsDialog::updateUIToOption()
{
    emit layoutOptionChanged(updateUIToLayoutOption());
    emit displayOptionChanged(updateUIToDisplayOption());
    emit iconsOptionChanged(updateUIToIconsOption());
    emit fontOptionChanged(updateUIToFontOption());
    emit colorOptionChanged(updateUIToColorOption());
    emit langOptionChanged(updateUIToLanguageOption());
    emit operationOptionChanged(updateUIToOperationOption());
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

LayoutOption OptionsDialog::updateUIToLayoutOption()
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

    return option;
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

DisplayOption OptionsDialog::updateUIToDisplayOption()
{
    DisplayOption option;

    option.isShowHideAndSystemFile = ui->cbShowHideAndSystemFile->isChecked();
    option.isShowParentDirInRootDrive = ui->cbShowParentDirInRootDrive->isChecked();
    option.isDirSortByName = ui->rbDirSortByName->isChecked();
    option.isShowToolbarTooltips = ui->cbShowToolbarTooltips->isChecked();
    option.isShowDriveTooltips = ui->cbShowDriveTooltips->isChecked();
    option.isShowFilenameTooltips = ui->cbShowFilenameTooltips->isChecked();

    return option;
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

    QList<int> fileIconSize({16, 24, 32}) ;
    int fileIconSizeIndex = fileIconSize.indexOf(option.fileIconSize);
    foreach(auto size, fileIconSize)
         ui->cbbFileIconSize->addItem(QString("%1x%1").arg(size), size);
    ui->cbbFileIconSize->setCurrentIndex(fileIconSizeIndex);
    setFileIconSize(fileIconSizeIndex);

    QList<int> toolBarIconSize({16, 20, 24, 32});
    int toolbarIconSizeIndex = toolBarIconSize.indexOf(option.toolbarIconSize);
    foreach(auto size, toolBarIconSize)
         ui->cbbToolBarIconSize->addItem(QString("%1x%1").arg(size), size);
    ui->cbbToolBarIconSize->setCurrentIndex(toolbarIconSizeIndex);
    setToolBarIconSize(toolbarIconSizeIndex);
}

IconsOption OptionsDialog::updateUIToIconsOption()
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

    return option;
}

void OptionsDialog::updateFontOptionToUI()
{
    FontOption const& option = theOptionManager.fontOption();

    ui->labelFileListFont->setText(option.fileList.caption());
    ui->labelMainWindowFont->setText(option.mainWindow.caption());
    ui->labelDialogFont->setText(option.dialog.caption());
    ui->labelFileListText->setFont(option.fileList.font());
    ui->labelMainWindowText->setFont(option.mainWindow.font());
    ui->labelDialogText->setFont(option.dialog.font());
}

FontOption OptionsDialog::updateUIToFontOption()
{
    FontOption option;
    option.fileList.setFont(ui->labelFileListText->font());
    option.mainWindow.setFont(ui->labelMainWindowText->font());
    option.dialog.setFont(ui->labelDialogText->font());
    return option;
}

void OptionsDialog::updateColorOptionToUI()
{
    ColorOption const& option = theOptionManager.colorOption();
}

ColorOption OptionsDialog::updateUIToColorOption()
{
    ColorOption option;
    return option;
}

void OptionsDialog::updateLanguageOptionToUI()
{
    LanguageOption const& option = theOptionManager.languageOption();
}

LanguageOption OptionsDialog::updateUIToLanguageOption()
{
    LanguageOption option;
    return option;
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

OperationOption OptionsDialog::updateUIToOperationOption()
{
    OperationOption option;

    option.isOnlyOneMainProgramRun = ui->cbOnlyOneMainProgramRun->isChecked();
    option.isGoToRootWhenChangeDrive = ui->cbGoToRootWhenChangeDrive->isChecked();
    option.isSelectFileNameWhenRenaming = ui->cbSelectFileNameWhenRenaming->isChecked();
    option.isLeftButtonSelect = ui->rbLeftButtonSelect->isChecked();
    return option;
}
