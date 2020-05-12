#ifndef WIDGETMANAGER_H
#define WIDGETMANAGER_H

//#include "dialogframeless.h"

#include <DDialog>

QT_BEGIN_NAMESPACE
class QStackedWidget;
class QMouseEvent;
QT_END_NAMESPACE

class WidgetStartVault;
class WidgetSetUnlockMethod;
class WidgetSaveKey;
class WidgetActivceVaultFinished;

DWIDGET_USE_NAMESPACE

class WidgetManager : public DDialog
{
    Q_OBJECT
public:
    WidgetManager(QWidget *parent = nullptr);

private slots:
    void slotNextWidget();

private:
    // 窗口容器
    QStackedWidget              *m_pStackedWidget;

    WidgetStartVault            *m_pStartVaultWidget;
    WidgetSetUnlockMethod       *m_pSetUnclockMethodWidget;
    WidgetSaveKey               *m_SaveKeyWidget;
    WidgetActivceVaultFinished  *m_ActiveVaultFinishedWidget;
};

#endif // WIDGETMANAGER_H
