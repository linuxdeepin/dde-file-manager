#ifndef DFILEMENU_H
#define DFILEMENU_H

#include <DMenu>

DWIDGET_USE_NAMESPACE

class AbstractFileInfo;

class DFileMenu : public DMenu
{
    Q_OBJECT

public:
    explicit DFileMenu(DMenu * parent = 0);

    void setUrls(const QList<QString> &urls);
    QList<QString> getUrls();

    void setDir(const QString &dir);
    QString getDir() const;

    void setWindowId(int windowId);
    int getWindowId() const;

    void setFileInfo(const AbstractFileInfo *info);
    const AbstractFileInfo *fileInfo() const;

private:
    QList<QString> m_urls;
    QString m_dir;
    const AbstractFileInfo *m_fileInfo = Q_NULLPTR;

    int m_windowId = -1;
};

#endif // DFILEMENU_H
