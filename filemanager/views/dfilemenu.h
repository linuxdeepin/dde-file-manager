#ifndef DFILEMENU_H
#define DFILEMENU_H

#include <DMenu>

#include "durl.h"

DWIDGET_USE_NAMESPACE

class AbstractFileInfo;

class DFileMenu : public DMenu
{
    Q_OBJECT

public:
    explicit DFileMenu(DMenu * parent = 0);

    void setUrls(const DUrlList &urls);
    DUrlList getUrls();

    void setWindowId(int windowId);
    int getWindowId() const;

    void setFileInfo(const AbstractFileInfo *info);
    const AbstractFileInfo *fileInfo() const;

private:
    DUrlList m_urls;
    const AbstractFileInfo *m_fileInfo = Q_NULLPTR;

    int m_windowId = -1;
};

#endif // DFILEMENU_H
