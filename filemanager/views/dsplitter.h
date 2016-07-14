#ifndef DSPLITTER_H
#define DSPLITTER_H

#include <QSplitter>
#include <QSplitterHandle>


class SplitterHandle: public QSplitterHandle
{
    Q_OBJECT

public:
    explicit SplitterHandle(Qt::Orientation orientation, QSplitter* parent);

protected:
    void enterEvent(QEvent* e);
};


class DSplitter : public QSplitter
{
    Q_OBJECT

public:
    DSplitter(Qt::Orientation orientation, QWidget* parent = 0);
    void moveSplitter(int pos, int index);

protected:
    QSplitterHandle* createHandle();
};

#endif // DSPLITTER_H
