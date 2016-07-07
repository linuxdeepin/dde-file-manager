#ifndef DSTATUSBAR_H
#define DSTATUSBAR_H

#include <QStatusBar>
#include <QHBoxLayout>
#include <QLabel>
#include <QSizeGrip>
#include "dpicturesequenceview.h"


DWIDGET_USE_NAMESPACE

class FMEvent;

class DStatusBar : public QFrame
{
    Q_OBJECT
public:
    DStatusBar(QWidget * parent = 0);

    void initUI();
    void initConnect();

public slots:
    void itemSelected(const FMEvent &event, int number);
    void itemCounted(const FMEvent &event, int number);
    void showLoadingIncator(const FMEvent &event, bool loading);
    void showSearchingIncator(const FMEvent &event, bool searching);

protected:
    void resizeEvent(QResizeEvent* event);

private:
    QString m_counted;
    QString m_selected;

    QHBoxLayout * m_layout;
    QLabel * m_label;
    DPictureSequenceView* m_loadingIndicator;

    QSizeGrip* m_sizeGrip;
};

#endif // DSTATUSBAR_H
