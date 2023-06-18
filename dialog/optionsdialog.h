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

private:
    void setStyleForLabel(QLabel* label);
private:
    Ui::OptionsDialog *ui;
};

#endif // OPTIONSDIALOG_H
