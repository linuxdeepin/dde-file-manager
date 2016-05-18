#ifndef DSTATUSBAR_H
#define DSTATUSBAR_H

#include <QStatusBar>
#include <QHBoxLayout>
#include <QLabel>

class FMEvent;

class DStatusBar : public QFrame
{
    Q_OBJECT
public:
    DStatusBar(QWidget * parent = 0);
private:
    QHBoxLayout * m_layout;
    QLabel * m_label;
public slots:
    void itemSelected(const FMEvent &event, int number);
};

#endif // DSTATUSBAR_H
