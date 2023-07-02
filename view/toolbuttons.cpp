#include "toolbuttons.h"
#include "ui_toolbuttons.h"

ToolButtons::ToolButtons(QWidget *parent) :
    QFrame(parent),    ui(new Ui::ToolButtons)
{
    ui->setupUi(this);

    connect(ui->btnView, SIGNAL(clicked()), SIGNAL(viewClicked()));
    connect(ui->btnEdit, SIGNAL(clicked()), SIGNAL(editClicked()));
    connect(ui->btnCopy, SIGNAL(clicked()), SIGNAL(copyClicked()));
    connect(ui->btnMove, SIGNAL(clicked()), SIGNAL(moveClicked()));
    connect(ui->btnNewFolder, SIGNAL(clicked()), SIGNAL(newFolderClicked()));
    connect(ui->btnNewFile, SIGNAL(clicked()), SIGNAL(newFileClicked()));
    connect(ui->btnDelete, SIGNAL(clicked()), SIGNAL(deleteClicked()));
    connect(ui->btnExit, SIGNAL(clicked()), SIGNAL(exitClicked()));
}

ToolButtons::~ToolButtons()
{
    delete ui;
}

void ToolButtons::setButtonFont(QFont const& font)
{
    ui->btnView->setFont(font);
    ui->btnEdit->setFont(font);
    ui->btnCopy->setFont(font);
    ui->btnMove->setFont(font);
    ui->btnNewFolder->setFont(font);
    ui->btnNewFile->setFont(font);
    ui->btnDelete->setFont(font);
    ui->btnExit->setFont(font);
}

void ToolButtons::retranslateUi()
{
    ui->retranslateUi(this);
}


