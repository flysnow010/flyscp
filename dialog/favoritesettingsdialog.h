#ifndef FAVORITESETTINGSDIALOG_H
#define FAVORITESETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class FavoriteSettingsDialog;
}
class FavoriteDirModel;
class DirFavorite;

class FavoriteSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FavoriteSettingsDialog(QWidget *parent = nullptr);
    ~FavoriteSettingsDialog();

    void setDirFavorite(DirFavorite* manager);

private:
    Ui::FavoriteSettingsDialog *ui;
    DirFavorite* dirFavorite;
    FavoriteDirModel* model;
};

#endif // FAVORITESETTINGSDIALOG_H
