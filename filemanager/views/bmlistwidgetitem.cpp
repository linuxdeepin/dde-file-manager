#include "bmlistwidgetitem.h"
#include "dcheckablebutton.h"
#include "bmlistwidget.h"

BMListWidgetItem::BMListWidgetItem(BMListWidget * widget,
                                   const QString &text,
                                   const QString &url,
                                   const QString &normal,
                                   const QString &hover)
    :QListWidgetItem(widget)
{
    m_url = url;
    m_normal = normal;
    m_hover = hover;
}
