#ifndef DFMOPTICALMEDIAWIDGET_H
#define DFMOPTICALMEDIAWIDGET_H

#include "app/define.h"
#include "dfileservices.h"

#include <QWidget>

class DFMOpticalMediaWidgetPrivate;
class DFMOpticalMediaWidget : public QWidget
{
    Q_OBJECT
public:
    //fixed:CD display size error
    static quint64 g_totalSize;
    static quint64 g_usedSize;

    explicit DFMOpticalMediaWidget(QWidget* parent);
    ~DFMOpticalMediaWidget();

    void updateDiscInfo(QString dev);

private:
    QScopedPointer<DFMOpticalMediaWidgetPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DFMOpticalMediaWidget)
};

#endif // DFMOPTICALMEDIAWIDGET_H
