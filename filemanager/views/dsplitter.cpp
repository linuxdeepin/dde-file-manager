#include "dsplitter.h"
#include <QDebug>
#include <QSizePolicy>

DSplitter::DSplitter(Qt::Orientation orientation, QWidget *parent)
    :QSplitter(orientation, parent)
{

}

void DSplitter::moveSplitter(int pos, int index)
{
    QSplitter::moveSplitter(pos, index);
}

void DSplitter::mousePressEvent(QMouseEvent *e)
{
    QSplitter::mousePressEvent(e);
}

void DSplitter::mouseReleaseEvent(QMouseEvent *e)
{
    QSplitter::mouseReleaseEvent(e);
}

