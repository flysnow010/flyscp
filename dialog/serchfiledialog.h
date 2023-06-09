#ifndef SERCHFILEDIALOG_H
#define SERCHFILEDIALOG_H

#include <QDialog>

namespace Ui {
class SerchFileDialog;
}

class QStringListModel;
class SerchFileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SerchFileDialog(QWidget *parent = nullptr);
    ~SerchFileDialog();

    void setSearchPath(QString  const& filePath);
signals:
    void viewFile(QString const& fileName);
    void goToFile(QString const& fileName);

private slots:
    void searchFiles();
private:
    void setSeearchState(bool isSearching);
    void startSearch(bool isStart);
    void insertText(int row,  QString const& text);
private:
    Ui::SerchFileDialog *ui;
    QStringListModel* model;
    QStringList fileNames;
    bool isSearching;
};

#endif // SERCHFILEDIALOG_H
