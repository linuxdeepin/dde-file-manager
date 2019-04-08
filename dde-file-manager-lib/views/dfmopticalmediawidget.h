#ifndef DFMOPTICALMEDIAWIDGET_H
#define DFMOPTICALMEDIAWIDGET_H

#include "app/define.h"
#include "dfileservices.h"

#include <DtkWidgets>

class DFMOpticalMediaWidgetPrivate;
class DFMOpticalMediaWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DFMOpticalMediaWidget(QWidget* parent);
    ~DFMOpticalMediaWidget();

    void updateDiscInfo(QString dev);

private:
    QScopedPointer<DFMOpticalMediaWidgetPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DFMOpticalMediaWidget)
};

#endif // DFMOPTICALMEDIAWIDGET_H
