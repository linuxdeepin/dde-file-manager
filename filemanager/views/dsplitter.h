#ifndef DSPLITTER_H
#define DSPLITTER_H

#include <QSplitter>

class DSplitter : public QSplitter
{
    Q_OBJECT
public:
    DSplitter(Qt::Orientation orientation, QWidget* parent = 0);
    void moveSplitter(int pos, int index);

};

#endif // DSPLITTER_H
