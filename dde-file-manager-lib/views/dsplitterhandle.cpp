#include "dsplitterhandle.h"

DSplitterHandle::DSplitterHandle(Qt::Orientation o, QSplitter *parent)
    : QSplitterHandle(o, parent)
{

}

void DSplitterHandle::mousePressEvent(QMouseEvent *e)
{
    QSplitterHandle::mousePressEvent(e);
}

void DSplitterHandle::mouseReleaseEvent(QMouseEvent *e)
{
    QSplitterHandle::mouseReleaseEvent(e);
}
