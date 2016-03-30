#include "bmlistwidget.h"
#include "dcheckablebutton.h"
#include "dscrollbar.h"
#include "dhorizseparator.h"
#include <QDropEvent>
#include <QDebug>

BMListWidget::BMListWidget(QWidget *parent)
    :QListWidget(parent)
{
    m_group = new QButtonGroup;
    setFocusPolicy(Qt::NoFocus);
    setObjectName("ListWidget");
    DScrollBar * scrollbar = new DScrollBar;
    setVerticalScrollBar(scrollbar);
    setAutoScroll(true);
    setAcceptDrops(true);
    setDragEnabled(true);
    setDropIndicatorShown(true);
    connect(m_group, SIGNAL(buttonClicked(int)), this, SLOT(buttonSelected(int)));
}

void BMListWidget::setItemWidget(QListWidgetItem *item, DCheckableButton *widget)
{
    m_group->addButton(widget, m_group->buttons().size());
    QListWidget::setItemWidget(item, widget);
}

void BMListWidget::addSeparator(const QSize &size)
{
    QListWidgetItem * item = new QListWidgetItem(this);
    item->setFlags(Qt::NoItemFlags);
    item->setSizeHint(size);
    QListWidget::setItemWidget(item, new DHorizSeparator);
}

void BMListWidget::dragEnterEvent(QDragEnterEvent *event)
{
    qDebug() << "list drag enter";
    event->accept();
}

void BMListWidget::dropEvent(QDropEvent *event)
{
    qDebug() << "list drop";
}

void BMListWidget::buttonSelected(int id)
{
    DCheckableButton * button = (DCheckableButton*)m_group->button(id);
    emit urlSelected(button->getUrl());
}
