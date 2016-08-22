#ifndef DSPLITTERHANDLE_H
#define DSPLITTERHANDLE_H

#include <QSplitterHandle>

class DSplitterHandle : public QSplitterHandle
{
public:
    DSplitterHandle(Qt::Orientation o, QSplitter *parent);
protected:
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
};

#endif // DSPLITTERHANDLE_H
