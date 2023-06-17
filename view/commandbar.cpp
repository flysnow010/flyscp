#include "commandbar.h"
#include "ui_commandbar.h"

#include <QKeyEvent>

CommandBar::CommandBar(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::CommandBar)
{
    ui->setupUi(this);
    ui->cbbCommand->installEventFilter(this);
}

CommandBar::~CommandBar()
{
    delete ui;
}

void CommandBar::setDir(QString const& dir)
{
    ui->labelDir->setText(dir);
}

bool CommandBar::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() == Qt::Key_Return)
        {
            QString command = ui->cbbCommand->currentText();
            if(ui->cbbCommand->findText(command) < 0)
                ui->cbbCommand->addItem(command);
            ui->cbbCommand->clearEditText();
            emit commanded(command);
        }
    }
    return QFrame::eventFilter(obj, event);
}
