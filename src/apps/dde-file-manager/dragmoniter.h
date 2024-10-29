#ifndef DRAGMONITER_H
#define DRAGMONITER_H

#include <QObject>
#include <QDBusContext>

namespace dfm_drag {

class DragMoniter : public QObject, public QDBusContext
{
    Q_OBJECT
public:
    explicit DragMoniter(QObject *parent = nullptr);
    void registerDBus();
    void unRegisterDBus();
protected:
    bool eventFilter(QObject *watched, QEvent *event);
signals:
    void dragEnter(const QStringList &);
};

}
#endif   // DRAGMONITER_H
