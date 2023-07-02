#include "colortoolbutton.h"

#include <QMenu>
#include <QPainter>
#include <QListWidget>
#include <QGridLayout>
#include <QWidgetAction>
#include <QColorDialog>

ColorToolButton::ColorToolButton(QWidget *parent)
    : QToolButton(parent)
    , colorMenu(createColorMenu())
{
    setPopupMode(MenuButtonPopup);
    setMenu(colorMenu);
    connect(this, &QToolButton::clicked, this, [=](){
        colorMenu->exec(mapToGlobal(rect().bottomLeft()));
        repaint();
    });
}

void ColorToolButton::setDefaultColor(QColor const& color)
{
    defaultColor = color;
    defaultAction->setIcon(createColorIcon(color));
}

QIcon ColorToolButton::createColorIcon(QColor const& color)
{
    QPixmap pixmap(16, 16);
    QPainter painter(&pixmap);
    painter.setPen(Qt::NoPen);
    painter.fillRect(QRect(0, 0, 16, 16), color);

    return QIcon(pixmap);
}

QAction* ColorToolButton::createGridColorAction(QMenu* menu)
{
    QColor gridColors[6][8] = {
        { QColor(0, 0, 0),      QColor(170, 0, 0),     QColor(0, 85, 0),     QColor(170, 85, 0),
          QColor(0, 170, 0),    QColor(170, 170, 0),   QColor(0, 255, 0),    QColor(170, 250, 0) },
        { QColor(0, 0, 127),    QColor(170, 0, 127),   QColor(0, 85, 127),   QColor(170, 85, 127),
          QColor(0, 170, 127),  QColor(170, 170, 127), QColor(0, 255, 127),  QColor(170, 255, 127) },
        { QColor(0, 0, 255),    QColor(170, 0, 255),   QColor(0, 85, 255),   QColor(170, 85, 255),
          QColor(0, 170, 255),  QColor(170, 170, 255), QColor(0, 255, 255),  QColor(170, 255, 255) },
        { QColor(85, 0, 0),     QColor(255, 0, 0),     QColor(85, 85, 0),    QColor(255, 85, 0),
          QColor(85, 170, 0),   QColor(255, 170, 0),   QColor(85, 255, 0),   QColor(255, 255, 0)},
        { QColor(85, 0, 127),   QColor(255, 0, 127),   QColor(85, 85, 127),  QColor(255, 85, 127),
          QColor(85, 170, 127), QColor(255, 170, 127), QColor(85, 255, 127), QColor(255, 255, 127) },
        { QColor(85, 0, 255),   QColor(255, 0, 255),   QColor(85, 85, 255),  QColor(255, 85, 255),
          QColor(85, 170, 255), QColor(255, 170, 255), QColor(85, 255, 255), QColor(255, 255, 255)}
    };
    QListWidget *colorListWidget = new QListWidget;
    colorListWidget->setFixedSize(210, 140);
    colorListWidget->setViewMode(QListView::IconMode);
    colorListWidget->setSpacing(4);
    colorListWidget->setSelectionRectVisible(false);
    colorListWidget->setMovement(QListView::Static);
    for (int row = 0; row < 6; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            QListWidgetItem* item = new QListWidgetItem(colorListWidget);
            item->setSizeHint(QSize(18, 18));
            item->setData(Qt::UserRole, gridColors[row][col]);
            item->setIcon(createColorIcon(gridColors[row][col]));
            colorListWidget->addItem(item);
        }
    }
    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->setContentsMargins(25, 0, 0, 0);
    gridLayout->addWidget(colorListWidget, 0, 0);
    QWidget *widget = new QWidget(menu);
    widget->setLayout(gridLayout);
    QWidgetAction* customAction = new QWidgetAction(menu);
    customAction->setDefaultWidget(widget);
    connect(colorListWidget, &QListWidget::clicked,
            this, [=]{
        QListWidgetItem* item = colorListWidget->currentItem();
        if(item)
        {
            emit selectedColor(item->data(Qt::UserRole).value<QColor>());
            colorMenu->close();
        }
    });
    return customAction;
}

QMenu* ColorToolButton::createColorMenu()
{
    QMenu *menu = new QMenu(this);
    defaultAction = new QAction(tr("Default"), menu);
    QAction* gridColorAction = createGridColorAction(menu);
    QAction* customColorAction = new QAction(tr("More..."), menu);

    connect(defaultAction, &QAction::triggered, this, [=](){
        emit selectedColor(defaultColor);
    });
    connect(customColorAction, &QAction::triggered, this, [=](){
        QColorDialog dialog;
        if(dialog.exec() != QDialog::Accepted)
            return;
        emit selectedColor(dialog.currentColor());
    });

    menu->addAction(defaultAction);
    menu->addSeparator();
    menu->addAction(gridColorAction);
    menu->addSeparator();
    menu->addAction(customColorAction);
    menu->setStyleSheet("QMenu {background-color: #f0f0f0; border: 1px solid #e3e3e3;}"
                        "QMenu::item { background-color: transparent;}"
                        "QMenu::item:selected { background-color: #3399ff;}"
                        "QListView{border: none; background-color: #f0f0f0;}"
                        "QListView::item:hover {background: #3399ff;}"
                        "QListView::item:selected{background: #3399ff;}"
                        );
    return menu;
}
