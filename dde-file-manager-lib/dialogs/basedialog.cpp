#include "basedialog.h"

#include <dwindowclosebutton.h>
#include <dtitlebar.h>

#include <QApplication>
#include <QDesktopWidget>
#include <QWindow>
#include <QShowEvent>
#include <QDebug>

BaseDialog::BaseDialog(QWidget *parent) : DAbstractDialog(parent)
{
    m_titlebar = new DTitlebar(this);

    m_titlebar->setWindowFlags(Qt::WindowCloseButtonHint | Qt::WindowTitleHint);
}

BaseDialog::~BaseDialog()
{

}

void BaseDialog::setTitle(const QString &title)
{
    m_titlebar->setTitle(title);
}

void BaseDialog::resizeEvent(QResizeEvent *event)
{
    m_titlebar->setFixedWidth(event->size().width());

    DAbstractDialog::resizeEvent(event);
}
