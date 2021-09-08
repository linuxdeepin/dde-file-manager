#ifndef DISPLAYDEFAULTVIEW_P_H
#define DISPLAYDEFAULTVIEW_P_H

#include "dfm_filemanager_service_global.h"

#include <QUrl>
#include <QObject>

DSB_FM_BEGIN_NAMESPACE

class DisplayDefaultView;
class DisplayDefaultViewPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(DisplayDefaultView)
    DisplayDefaultView * const q_ptr;
    QUrl url;

    explicit DisplayDefaultViewPrivate(DisplayDefaultView *qq);
};

DSB_FM_END_NAMESPACE

#endif // DISPLAYDEFAULTVIEW_P_H
