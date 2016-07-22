#include "dsplitter.h"
#include <QDebug>
#include <QSizePolicy>

SplitterHandle::SplitterHandle(Qt::Orientation orientation, QSplitter *parent):
    QSplitterHandle(orientation, parent)
{

}

void SplitterHandle::enterEvent(QEvent *e)
{
    Q_UNUSED(e)

    setCursor(Qt::ArrowCursor);
}



DSplitter::DSplitter(Qt::Orientation orientation, QWidget *parent)
    :QSplitter(orientation, parent)
{

}

void DSplitter::moveSplitter(int pos, int index)
{
    QSplitter::moveSplitter(pos, index);
}


QSplitterHandle *DSplitter::createHandle()
{
    return new SplitterHandle(orientation(), this);
}



