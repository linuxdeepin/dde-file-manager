#include "dsplitter.h"
#include <QDebug>

DSplitter::DSplitter(Qt::Orientation orientation, QWidget *parent)
    :QSplitter(orientation, parent)
{

}

void DSplitter::moveSplitter(int pos, int index)
{
    QSplitter::moveSplitter(pos, index);
}

