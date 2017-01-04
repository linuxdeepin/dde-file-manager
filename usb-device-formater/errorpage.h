#ifndef ERRORPAGE_H
#define ERRORPAGE_H

#include <QFrame>

class ErrorPage : public QFrame
{
    Q_OBJECT
public:
    explicit ErrorPage(QWidget *parent = 0);
    void initUI();

signals:

public slots:
};

#endif // ERRORPAGE_H
