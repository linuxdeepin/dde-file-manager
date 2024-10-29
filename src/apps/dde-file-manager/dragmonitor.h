#ifndef DRAGMONITER_H
#define DRAGMONITER_H

#include <QObject>
#include <QDBusContext>
namespace dfm_drag {
// NOTE: temp code!!!!!!!!!!!
class DragMoniter : public QObject, public QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.filemanager.drag")

public:
    explicit DragMoniter(QObject *parent = nullptr);
    void registerDBus();
    void unRegisterDBus();

protected:
    bool eventFilter(QObject *watched, QEvent *event);

signals:
    Q_SCRIPTABLE void dragEnter(const QStringList &urls);
};
}

#endif   // DRAGMONITER_H
