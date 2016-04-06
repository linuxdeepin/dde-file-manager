#ifndef DSPLITTER_H
#define DSPLITTER_H

#include <QSplitter>

class DSplitter : public QSplitter
{
    Q_OBJECT
public:
    DSplitter(Qt::Orientation orientation, QWidget* parent = 0);
    void moveSplitter(int pos, int index);
protected:
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
};

#endif // DSPLITTER_H
