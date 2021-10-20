#ifndef COREBROWSEVIEW_H
#define COREBROWSEVIEW_H

#include "window/browseview.h" // TODO(Huangyu): hide

DSB_FM_USE_NAMESPACE

class CoreBrowseView : public BrowseView
{
public:
    explicit CoreBrowseView(QWidget *parent = nullptr);

protected:
    void contextMenuEvent(QContextMenuEvent *event);
};

#endif // COREBROWSEVIEW_H
