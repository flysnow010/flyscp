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
    LayoutOption updateUIToLayoutOption();

    void updateDisplayOptionToUI();
    DisplayOption updateUIToDisplayOption();

    void updateIconsOptionToUI();
    IconsOption updateUIToIconsOption();

    void updateFontOptionToUI();
    FontOption updateUIToFontOption();

    void updateColorOptionToUI();
    ColorOption updateUIToColorOption();

    void updateLanguageOptionToUI();
    LanguageOption updateUIToLanguageOption();

    void updateOperationOptionToUI();
    OperationOption updateUIToOperationOption();
private:
    Ui::OptionsDialog *ui;
};

#endif // OPTIONSDIALOG_H
