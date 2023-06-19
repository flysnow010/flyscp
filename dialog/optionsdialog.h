#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

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
    void layoutOptionChanged();
    void displayOptionChanged();
    void iconsOptionChanged();
    void fontOptionChanged();
    void colorOptionChanged();
    void langOptionChanged();
    void operationOptionChanged();

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

private:
    Ui::OptionsDialog *ui;
};

#endif // OPTIONSDIALOG_H
