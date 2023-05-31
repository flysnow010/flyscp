#ifndef SERCHFILEDIALOG_H
#define SERCHFILEDIALOG_H

#include <QDialog>

namespace Ui {
class SerchFileDialog;
}

class SerchFileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SerchFileDialog(QWidget *parent = nullptr);
    ~SerchFileDialog();

    void setSearchPath(QString  const& filePath);
private:
    Ui::SerchFileDialog *ui;
};

#endif // SERCHFILEDIALOG_H
