#ifndef DFILEMENU_H
#define DFILEMENU_H

#include <DMenu>

#include "durl.h"
#include "abstractfileinfo.h"

DWIDGET_USE_NAMESPACE

class AbstractFileInfo;

class DFileMenu : public DMenu
{
    Q_OBJECT

public:
    enum MenuSource {
        CrumbButton,
        LeftSideBar,
        BackAndForwardButton,
        UpButton,
        FileView,
        SearchLine,
        Menu,
        Unknow
    };

    explicit DFileMenu(DMenu * parent = 0);

    void setUrls(const DUrlList &urls);
    DUrlList getUrls();

    void setWindowId(int windowId);
    int getWindowId() const;

    void setMenuSource(MenuSource source);
    MenuSource getMenuSource();
private:
    DUrlList m_urls;
    MenuSource m_source;
    int m_windowId = -1;
};

#endif // DFILEMENU_H
