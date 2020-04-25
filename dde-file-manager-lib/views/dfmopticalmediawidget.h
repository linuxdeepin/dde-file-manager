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
    //fix: 动态获取刻录选中文件的字节大小
    static qint64 g_selectBurnFilesSize;

    explicit DFMOpticalMediaWidget(QWidget* parent);
    ~DFMOpticalMediaWidget();

    void updateDiscInfo(QString dev);

    //fix: 动态更新光驱磁盘状态
private slots:
    void selectBurnFilesOptionUpdate();

private:
    QScopedPointer<DFMOpticalMediaWidgetPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DFMOpticalMediaWidget)
};

#endif // DFMOPTICALMEDIAWIDGET_H
