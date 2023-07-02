#include "optionsdialog.h"
#include "core/languagemanager.h"
#include "util/utils.h"

#include "ui_optionsdialog.h"
#include <QPixmap>
#include <QIcon>
#include <QFontDialog>
#include <QPainter>
#include <QMenu>
#include <QSettings>

int const LayoutPage    = 0;
int const DisplayPage   = 1;
int const IconsPage     = 2;
int const FontPage      = 3;
int const ColorPage     = 4;
int const LangPage      = 5;
int const OperationPage = 6;

OptionsDialog::OptionsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::OptionsDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    colorSize = ui->tbFontColor->iconSize();
    qRegisterMetaType<LayoutOption>("LayoutOption");
    qRegisterMetaType<DisplayOption>("DisplayOption");
    qRegisterMetaType<IconsOption>("IconsOption");
    qRegisterMetaType<FontOption>("FontOption");
    qRegisterMetaType<ColorOption>("ColorOption");
    qRegisterMetaType<LanguageOption>("LanguageOption");
    qRegisterMetaType<OperationOption>("OperationOption");
    ui->treeWidget->clear();

    layout = new QTreeWidgetItem(QStringList() << tr("Layout"));
    display = new QTreeWidgetItem(QStringList() << tr("Display"));
    icons = new QTreeWidgetItem(QStringList() << tr("Icons"));
    font = new QTreeWidgetItem(QStringList() << tr("Font"));
    color = new QTreeWidgetItem(QStringList() << tr("Color"));
    language = new QTreeWidgetItem(QStringList() << tr("Language"));
    operation = new QTreeWidgetItem(QStringList() << tr("Operation"));

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
    loadSettings();
}

OptionsDialog::~OptionsDialog()
{
    saveSettings();
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
            ui->stackedWidget->setCurrentIndex(
                        item->data(0, Qt::UserRole).toInt());
    });
    connect(ui->btnApply, &QPushButton::clicked, this, [=](){
        if(ui->stackedWidget->currentIndex() == LayoutPage)
            updateUIToLayoutOption();
        else if(ui->stackedWidget->currentIndex() == DisplayPage)
            updateUIToDisplayOption();
        else if(ui->stackedWidget->currentIndex() == IconsPage)
            updateUIToIconsOption();
        else if(ui->stackedWidget->currentIndex() == FontPage)
            updateUIToFontOption();
        else if(ui->stackedWidget->currentIndex() == ColorPage)
            updateUIToColorOption();
        else if(ui->stackedWidget->currentIndex() == LangPage)
            updateUIToLanguageOption();
        else if(ui->stackedWidget->currentIndex() == OperationPage)
            updateUIToOperationOption();
    });

    connect(ui->cbbFileIconSize, SIGNAL(currentIndexChanged(int)),
            this, SLOT(setFileIconSize(int)));
    connect(ui->cbbToolBarIconSize, SIGNAL(currentIndexChanged(int)),
            this, SLOT(setToolBarIconSize(int)));

    connect(ui->btnFileList, &QPushButton::clicked, this, [=](){
        bool ok;
        QFont font = QFontDialog::getFont(&ok,
                                          ui->labelFileListText->font(),
                                          this);
        if (ok)
        {
            ui->labelFileListText->setFont(font);
            ui->labelFileListFont->setText(QString("%1,%2")
                                           .arg(font.family())
                                           .arg(font.pointSize()));
        }
    });

    connect(ui->btnMainWindow, &QPushButton::clicked, this, [=](){
        bool ok;
        QFont font = QFontDialog::getFont(&ok,
                                          ui->labelMainWindowText->font(),
                                          this);
        if (ok)
        {
            ui->labelMainWindowText->setFont(font);
            ui->labelMainWindowFont->setText(QString("%1,%2")
                                             .arg(font.family())
                                             .arg(font.pointSize()));
        }
    });
    connect(ui->btnDialog, &QPushButton::clicked, this, [=](){
        bool ok;
        QFont font = QFontDialog::getFont(&ok,
                                          ui->labelDialogText->font(),
                                          this);
        if (ok)
        {
            ui->labelDialogText->setFont(font);
            ui->labelDialogFont->setText(QString("%1,%2")
                                         .arg(font.family())
                                         .arg(font.pointSize()));
        }
    });
    connect(ui->tbFontColor, &ColorToolButton::selectedColor,
            this, [=](QColor const& color)
    {
        colorOption.fontColor = color.name();
        updateColorOptionToUI(colorOption);
    });

    connect(ui->tbBackgroud1, &ColorToolButton::selectedColor,
            this, [=](QColor const& color)
    {
        colorOption.background1Color = color.name();
        updateColorOptionToUI(colorOption);
    });
    connect(ui->tbBackgroud2, &ColorToolButton::selectedColor,
            this, [=](QColor const& color)
    {
        colorOption.background2Color = color.name();
        updateColorOptionToUI(colorOption);
    });
    connect(ui->tbMarkColor, &ColorToolButton::selectedColor,
            this, [=](QColor const& color)
    {
        colorOption.markColor = color.name();
        updateColorOptionToUI(colorOption);
    });
    connect(ui->tbCursorColor, &ColorToolButton::selectedColor,
            this, [=](QColor const& color)
    {
        colorOption.cursorColor = color.name();
        updateColorOptionToUI(colorOption);
    });
}

void OptionsDialog::setFileIconSize(int index)
{
    int size = ui->cbbFileIconSize->itemData(index).toInt();
    ui->labelFileIcon->setPixmap(Utils::dirIcon()
                                 .pixmap(QSize(size, size)));
}

void OptionsDialog::setToolBarIconSize(int index)
{
    int size = ui->cbbToolBarIconSize->itemData(index).toInt();
    ui->labelToolBarIcon->setPixmap(QIcon(":/image/control.png")
                                    .pixmap(QSize(size, size)));
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
    if(option.showStyle == "windowsvista")
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
    option.isShowCommandLine = ui->cbShowCommandLine->isChecked();
    option.isShowSortHeader = ui->cbShowSortHeader->isChecked();
    option.isShowFunctionKeyButtons = ui->cbShowFunctionKeyButtons->isChecked();
    if(ui->rbVista->isChecked())
        option.showStyle = "windowsvista";
    else if(ui->rbWindow->isChecked())
        option.showStyle = "Windows";
    else if(ui->rbFusion->isChecked())
        option.showStyle = "Fusion";

    if(option != theOptionManager.layoutOption())
        emit layoutOptionChanged(option);
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

    if(option != theOptionManager.displayOption())
        emit displayOptionChanged(option);
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

    QList<int> fileIconSize({16, 20, 24, 32}) ;
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

    if(option != theOptionManager.iconOption())
        emit iconsOptionChanged(option);
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

void OptionsDialog::updateUIToFontOption()
{
    FontOption option;
    option.fileList.setFont(ui->labelFileListText->font());
    option.mainWindow.setFont(ui->labelMainWindowText->font());
    option.dialog.setFont(ui->labelDialogText->font());

    if(option != theOptionManager.fontOption())
        emit fontOptionChanged(option);
}

void OptionsDialog::updateColorOptionToUI()
{
    ColorOption colorOption;
    ui->tbFontColor->setDefaultColor(QColor(colorOption.fontColor));
    ui->tbBackgroud1->setDefaultColor(QColor(colorOption.background1Color));
    ui->tbBackgroud2->setDefaultColor(QColor(colorOption.background2Color));
    ui->tbMarkColor->setDefaultColor(QColor(colorOption.markColor));
    ui->tbCursorColor->setDefaultColor(QColor(colorOption.cursorColor));
    updateColorOptionToUI(colorOption);
}

void OptionsDialog::updateColorOptionToUI(ColorOption const& option)
{
    ui->tbFontColor->setIcon(createFontIcon(option, ui->tbFontColor->text()));
    ui->tbBackgroud1->setIcon(createBack1Icon(option, ui->tbBackgroud1->text()));
    ui->tbBackgroud2->setIcon(createBack2Icon(option, ui->tbBackgroud2->text()));
    ui->tbMarkColor->setIcon(createMarkIcon(option, ui->tbMarkColor->text()));
    ui->tbCursorColor->setIcon(createCursorIcon(option, ui->tbCursorColor->text()));
    ui->labelExample->setPixmap(createExample(option));
}

void OptionsDialog::updateUIToColorOption()
{
    if(colorOption != theOptionManager.colorOption())
        emit colorOptionChanged(colorOption);
}

void OptionsDialog::updateLanguageOptionToUI()
{
    LanguageOption const& option = theOptionManager.languageOption();
    LanguageManager manager;
    Languages const& languages = manager.languages();
    foreach(auto lang, languages)
    {
        QListWidgetItem* item = new QListWidgetItem(lang.showText(), ui->lwLanguage);

        item->setData(Qt::UserRole, lang.name);
        if(lang.name == option.language)
            item->setSelected(true);
        ui->lwLanguage->addItem(item);
    }
}

void OptionsDialog::updateUIToLanguageOption()
{
    LanguageOption option;
    QListWidgetItem *item = ui->lwLanguage->currentItem();
    if(item)
    {
        option.language = item->data(Qt::UserRole).toString();
        if(option != theOptionManager.languageOption())
        {
            emit langOptionChanged(option);
            ui->retranslateUi(this);
            updateUI();
        }
    }
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

    if(option != theOptionManager.operationOption())
        emit operationOptionChanged(option);
}

QIcon OptionsDialog::createFontIcon(ColorOption const& option, QString const& text)
{
    QPixmap pixmap(colorSize);
    QPainter painter(&pixmap);

    painter.setPen(Qt::NoPen);
    painter.fillRect(0, 0, colorSize.width(), colorSize.height(), QColor(QString("#FFFFFF")));
    painter.setPen(QColor(option.fontColor));
    painter.drawText(QRect(QPoint(0, 0), colorSize), Qt::AlignHCenter | Qt::AlignVCenter, text);

    return QIcon(pixmap);
}

QIcon OptionsDialog::createBack1Icon(ColorOption const& option, QString const& text)
{
    QPixmap pixmap(colorSize);
    QPainter painter(&pixmap);

    painter.setPen(Qt::NoPen);
    painter.fillRect(QRect(QPoint(0, 0), colorSize), QColor(option.background1Color));
    painter.setPen(QColor(option.fontColor));
    painter.drawText(QRect(QPoint(0, 0), colorSize), Qt::AlignHCenter | Qt::AlignVCenter, text);

    return QIcon(pixmap);
}

QIcon OptionsDialog::createBack2Icon(ColorOption const& option, QString const& text)
{
    QPixmap pixmap(colorSize);
    QPainter painter(&pixmap);

    painter.setPen(Qt::NoPen);
    painter.fillRect(QRect(QPoint(0, 0), colorSize), QColor(option.background2Color));
    painter.setPen(QColor(option.fontColor));
    painter.drawText(QRect(QPoint(0, 0), colorSize), Qt::AlignHCenter | Qt::AlignVCenter, text);

    return QIcon(pixmap);
}

QIcon OptionsDialog::createMarkIcon(ColorOption const& option, QString const& text)
{
    QPixmap pixmap(colorSize);
    QPainter painter(&pixmap);

    painter.setPen(Qt::NoPen);
    painter.fillRect(QRect(QPoint(0, 0), colorSize), QColor(option.markColor));
    painter.setPen(QColor(option.fontColor));
    painter.drawText(QRect(QPoint(0, 0), colorSize), Qt::AlignHCenter | Qt::AlignVCenter, text);

    return QIcon(pixmap);
}

QIcon OptionsDialog::createCursorIcon(ColorOption const& option, QString const& text)
{
    QPixmap pixmap(colorSize);
    QPainter painter(&pixmap);

    painter.setPen(Qt::NoPen);
    painter.fillRect(QRect(QPoint(0, 0), colorSize), QColor(option.markColor));
    painter.setPen(QColor(option.cursorColor));
    painter.drawRect(QRect(QPoint(0, 0), colorSize - QSize(1, 1)));
    painter.setPen(QColor(option.fontColor));
    painter.drawText(QRect(QPoint(0, 0), colorSize), Qt::AlignHCenter | Qt::AlignVCenter, text);

    return QIcon(pixmap);
}

QPixmap OptionsDialog::createExample(ColorOption const& option)
{
    QSize size(200, 161);
    QPixmap pixmap(size);
    QPainter painter(&pixmap);

    painter.setPen(Qt::NoPen);
    painter.fillRect(QRect(QPoint(0, 0), size), QColor(option.background1Color));

    int x = 0;
    int y = 0;
    int w = size.width();
    int h = colorSize.height() + 4;

    painter.setPen(QColor(option.fontColor));
    painter.fillRect(QRect(x, y, size.width(), h), QColor(option.background1Color));
    painter.setPen(QColor(option.fontColor));
    painter.drawText(QRect(x, y, w, h), Qt::AlignHCenter | Qt::AlignVCenter, "Text");
    y += h;
    painter.fillRect(QRect(x, y, w, h), QColor(option.background2Color));
    painter.setPen(QColor(option.fontColor));
    painter.drawText(QRect(x, y, w, h), Qt::AlignHCenter | Qt::AlignVCenter, "Text");
    y += h;
    painter.fillRect(QRect(x, y, w, h), QColor(option.markColor));
    painter.setPen(QColor(option.cursorColor));
    painter.drawRect(QRect(x, y, w - 1, h - 1));
    painter.setPen(QColor(option.fontColor));
    painter.drawText(QRect(x, y, w, h), Qt::AlignHCenter | Qt::AlignVCenter, "Mark+Text");
    y += h;
    painter.fillRect(QRect(x, y, w, h), QColor(option.background2Color));
    painter.setPen(QColor(option.fontColor));
    painter.drawText(QRect(x, y, w, h), Qt::AlignHCenter | Qt::AlignVCenter, "Text");
    y += h;
    painter.fillRect(QRect(x, y, w, h), QColor(option.background1Color));
    painter.setPen(QColor(option.fontColor));
    painter.drawText(QRect(x, y, w, h), Qt::AlignHCenter | Qt::AlignVCenter, "Text");
    y += h;
    painter.fillRect(QRect(x, y, w, h), QColor(option.markColor));
    painter.setPen(QColor(option.cursorColor));
    painter.drawRect(QRect(x, y, w - 1, h - 1));
    painter.setPen(QColor(option.fontColor));
    painter.drawText(QRect(x, y, w, h), Qt::AlignHCenter | Qt::AlignVCenter, "Mark+Text");

    return pixmap;
}

void OptionsDialog::setCurrntTreeItem(int index)
{
    for(int i = 0; i < ui->treeWidget->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = ui->treeWidget->topLevelItem(i);
        if(item->data(0, Qt::UserRole).toInt() == index)
        {
            ui->treeWidget->setCurrentItem(item);
            break;
        }
        for(int j = 0; j < item->childCount(); j++)
        {
            if(item->child(j)->data(0, Qt::UserRole).toInt() == index)
            {
                ui->treeWidget->setCurrentItem(item->child(j));
                break;
            }
        }
    }
}

void OptionsDialog::saveSettings()
{
    QSettings settings(QCoreApplication::applicationName(),
                       QCoreApplication::applicationVersion());
    settings.beginGroup("OptionsDialog");
    settings.setValue("currentIndex", ui->stackedWidget->currentIndex());
    settings.endGroup();
}

void OptionsDialog::loadSettings()
{
    QSettings settings(QCoreApplication::applicationName(),
                       QCoreApplication::applicationVersion());
    settings.beginGroup("OptionsDialog");
    int index = settings.value("currentIndex", 0).toInt();
    ui->stackedWidget->setCurrentIndex(index);
    setCurrntTreeItem(index);
    settings.endGroup();
}

void OptionsDialog::updateUI()
{
    layout->setText(0, tr("Layout"));
    display->setText(0, tr("Display"));
    icons->setText(0, tr("Icons"));
    font->setText(0, tr("Font"));
    color->setText(0, tr("Color"));
    language->setText(0, tr("Language"));
    operation->setText(0, tr("Operation"));
}
