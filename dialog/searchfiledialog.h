#ifndef SEARCHFILEDIALOG_H
#define SEARCHFILEDIALOG_H

#include <QDialog>

namespace Ui {
class SearchFileDialog;
}

class StringListModel;
class QMenu;
class QComboBox;
class SearchFileDialog : public QDialog
{
    Q_OBJECT

public:
    enum Mode { Local, Remote };
    explicit SearchFileDialog(Mode mode = Mode::Local, QWidget *parent = nullptr);
    ~SearchFileDialog();

    void setSearchPath(QString  const& filePath);
signals:
    void viewFile(QString const& fileName);
    void goToFile(QString const& fileName);

private slots:
    void searchFiles();
private:
    void setSeearchState(bool isSearching);
    void startSearch(bool isStart);
    void insertText(int row, QString const& text);
    void createDriverMenu();
    void saveSettings();
    void loadSettings();
    void addCurentItem(QComboBox* cb);
private:
    Ui::SearchFileDialog *ui;
    Mode mode_;
    QMenu* dirverMenu;
    StringListModel* model;
    QStringList fileNames;
    bool isSearching;
};

#endif // SEARCHFILEDIALOG_H
