#include "favoritesettingsdialog.h"
#include "model/favoritedirmodel.h"
#include "core/dirfavorite.h"
#include "util/utils.h"
#include "ui_favoritesettingsdialog.h"

FavoriteSettingsDialog::FavoriteSettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FavoriteSettingsDialog)
    , dirFavorite(0)
    , model(new FavoriteDirModel(this))
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui->treeView->setModel(model);

    connect(ui->treeView, &QTreeView::clicked, this, [=](QModelIndex const& index){
        ui->lineEditFilePath->setText(model->item(index.row())->filePath);
        ui->lineEditCommand->setText(model->item(index.row())->command);
    });
    connect(ui->lineEditFilePath, &QLineEdit::textChanged, this, [=](){
        QModelIndex index = ui->treeView->currentIndex();
        if(index.isValid())
        {
            FavoriteItem* item = dirFavorite->item(index.row());
            item->filePath = ui->lineEditFilePath->text();
        }
    });

    connect(ui->lineEditCommand, &QLineEdit::textChanged, this, [=](){
        QModelIndex index = ui->treeView->currentIndex();
        if(index.isValid())
        {
            FavoriteItem* item = dirFavorite->item(index.row());
            item->command = ui->lineEditCommand->text();
        }
    });
    connect(ui->btnNew, &QPushButton::clicked, this, [=](){
        QString caption = Utils::getText(tr("Menu's Caption"));
        if(!caption.isEmpty())
        {
            FavoriteItem item;
            item.caption = caption;
            dirFavorite->addItem(item);
            model->refresh();
        }
    });
    connect(ui->btnUpMove, &QPushButton::clicked, this, [=](){
        QModelIndex index = ui->treeView->currentIndex();
        if(index.isValid())
        {
            dirFavorite->upMove(index.row());
            model->refresh();
        }
    });
    connect(ui->btnDownMove, &QPushButton::clicked, this, [=](){
        QModelIndex index = ui->treeView->currentIndex();
        if(index.isValid())
        {
            dirFavorite->downMove(index.row());
            model->refresh();
        }
    });
    connect(ui->btnRename, &QPushButton::clicked, this, [=](){
        QModelIndex index = ui->treeView->currentIndex();
        if(index.isValid())
            ui->treeView->edit(index);
    });
    connect(ui->btnDelete, &QPushButton::clicked, this, [=](){
        QModelIndex index = ui->treeView->currentIndex();
        if(index.isValid())
        {
            dirFavorite->removeItem(index.row());
            model->refresh();
        }
    });
}

FavoriteSettingsDialog::~FavoriteSettingsDialog()
{
    delete ui;
}

void FavoriteSettingsDialog::setDirFavorite(DirFavorite* manager)
{
    dirFavorite = manager;
    model->setDirFavorite(manager);
}
