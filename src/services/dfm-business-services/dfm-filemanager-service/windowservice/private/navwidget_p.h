#ifndef NAVWIDGET_P_H
#define NAVWIDGET_P_H

#include "dfm_filemanager_service_global.h"

#include <DButtonBox>

#include <QObject>
#include <QHBoxLayout>

DSB_FM_BEGIN_NAMESPACE

class NavWidget;
class NavWidgetPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(NavWidget)
    NavWidget * const q_ptr;
    Dtk::Widget::DButtonBox *buttonBox = nullptr;
    Dtk::Widget::DButtonBoxButton *navBackButton = nullptr;
    Dtk::Widget::DButtonBoxButton *navForwardButton = nullptr;
    QHBoxLayout *hboxLayout = nullptr;

    explicit NavWidgetPrivate(NavWidget* qq);

private Q_SLOTS:
   void doButtonClicked();
};


DSB_FM_END_NAMESPACE

#endif // NAVWIDGET_P_H
