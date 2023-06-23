#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H
#include "core/optionsmanager.h"

#include <QDialog>

namespace Ui {
class OptionsDialog;
}
class QLabel;
class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionsDialog(QWidget *parent = nullptr);
    ~OptionsDialog();

    void updateUIToOption();

signals:
    void layoutOptionChanged(LayoutOption const&);
    void displayOptionChanged(DisplayOption const&);
    void iconsOptionChanged(IconsOption const&);
    void fontOptionChanged(FontOption const&);
    void colorOptionChanged(ColorOption const&);
    void langOptionChanged(LanguageOption const&);
    void operationOptionChanged(OperationOption const&);
private slots:
    void setFileIconSize(int index);
    void setToolBarIconSize(int index);
private:
    void createConnects();
    void setStyleForLabel(QLabel* label);
    void updateOptionToUI();

    void updateLayoutOptionToUI();
    void updateUIToLayoutOption();

    void updateDisplayOptionToUI();
    void updateUIToDisplayOption();

    void updateIconsOptionToUI();
    void updateUIToIconsOption();

    void updateFontOptionToUI();
    void updateUIToFontOption();

    void updateColorOptionToUI();
    void updateUIToColorOption();

    void updateLanguageOptionToUI();
    void updateUIToLanguageOption();

    void updateOperationOptionToUI();
    void updateUIToOperationOption();

    void updateColorOptionToUI(ColorOption const& option);

    QIcon createFontIcon(ColorOption const& option, QString const& text);
    QIcon createBack1Icon(ColorOption const& option, QString const& text);
    QIcon createBack2Icon(ColorOption const& option, QString const& text);
    QIcon createMarkIcon(ColorOption const& option, QString const& text);
    QIcon createCursorIcon(ColorOption const& option, QString const& text);
    QPixmap createExample(ColorOption const& option);

    void setCurrntTreeItem(int index);
    void saveSettings();
    void loadSettings();
private:
    Ui::OptionsDialog *ui;
    QSize colorSize;
    ColorOption colorOption;
};

#endif // OPTIONSDIALOG_H
