#ifndef WAITITEM_H
#define WAITITEM_H
#include <QFrame>

#include <dtkwidget_global.h>

#include <QDebug>

DWIDGET_BEGIN_NAMESPACE
class DSpinner;
DWIDGET_END_NAMESPACE

DWIDGET_USE_NAMESPACE

class QWidget;
class QLabel;
class WaitItem : public QFrame
{
public:
    explicit WaitItem(QWidget *parent = nullptr);
    ~WaitItem();

    void initSize(const QSize &);
    void setContantText(const QString &);
    void setAnimation();

private:
    QLabel *m_icon = nullptr;
    QLabel *m_contant = nullptr;
    DSpinner *m_animationSpinner = nullptr;
    double m_proportion = 874 / 1900.0;//根据设计计算比例
    double m_movedistance = 0.0;//移动的相对位置
    QSize m_iconsize = QSize(30, 30);
    QSize m_contantsize = QSize(200, 20);
    QSize m_spinnersize = QSize(24, 24);
};

#endif // WAITITEM_H
