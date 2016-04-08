#ifndef DFILEMENU_H
#define DFILEMENU_H

#include <DMenu>

DWIDGET_USE_NAMESPACE

class DFileMenu : public DMenu
{
    Q_OBJECT

public:
    explicit DFileMenu(DMenu * parent = 0);
    void setUrls(const QList<QString> &urls);
    QList<QString> getUrls();
    void setDir(const QString &dir);
    QString getDir();
private:
    QList<QString> m_urls;
    QString m_dir;
};

#endif // DFILEMENU_H
