#include "optionsdialog.h"
#include "ui_optionsdialog.h"

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
    layout->setData(0, Qt::UserRole, 0);
    display->setData(0, Qt::UserRole, 1);
    icons->setData(0, Qt::UserRole, 2);
    font->setData(0, Qt::UserRole, 3);
    color->setData(0, Qt::UserRole, 4);
    language->setData(0, Qt::UserRole, 5);
    operation->setData(0, Qt::UserRole, 6);
    display->addChild(icons);
    display->addChild(font);
    display->addChild(color);
    display->addChild(language);
    ui->treeWidget->addTopLevelItem(layout);
    ui->treeWidget->addTopLevelItem(display);
    ui->treeWidget->addTopLevelItem(operation);
    display->setExpanded(true);

    connect(ui->treeWidget, &QTreeWidget::itemSelectionChanged, this, [=](){
        QTreeWidgetItem *item = ui->treeWidget->currentItem();
        if(item)
            ui->stackedWidget->setCurrentIndex(item->data(0, Qt::UserRole).toInt());
    });

    ui->treeWidget->setCurrentItem(layout);

    setStyleForLabel(ui->labelLayout);
    setStyleForLabel(ui->labelDisplay);
    setStyleForLabel(ui->labelIcons);
    setStyleForLabel(ui->labelFont);
    setStyleForLabel(ui->labelColor);
    setStyleForLabel(ui->labelLanguage);
    setStyleForLabel(ui->labelOperation);
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
