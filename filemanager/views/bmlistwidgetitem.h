#ifndef BMLISTWIDGETITEM_H
#define BMLISTWIDGETITEM_H

#include <QListWidgetItem>
#include <QListWidget>

class DCheckableButton;
class BMListWidget;

class BMListWidgetItem : public QListWidgetItem
{
public:
    BMListWidgetItem(BMListWidget * widget,
                     const QString &text,
                     const QString &url,
                     const QString &normal,
                     const QString &hover);
    QString getUrl();
private:
    QString m_url;
    QString m_normal;
    QString m_hover;
};

#endif // BMLISTWIDGETITEM_H
