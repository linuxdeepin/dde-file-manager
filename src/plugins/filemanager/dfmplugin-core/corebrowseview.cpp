#include "corebrowseview.h"
#include "dfm-framework/framework.h"
#include "services/common/menu/menuservice.h"

DSC_USE_NAMESPACE

CoreBrowseView::CoreBrowseView(QWidget *parent)
    : BrowseView(parent)
{

}

void CoreBrowseView::contextMenuEvent(QContextMenuEvent *event)
{
    Q_UNUSED(event);

    auto &ctx = dpfInstance.serviceContext();
    MenuService* menuService = ctx.service<MenuService>(MenuService::name());
    if (!menuService) {
        abort();
    }

    qInfo() << "rooturl" << rootUrl();

    QMenu *tempMenu = menuService->createMenu(AbstractFileMenu::Empty,
                                              rootUrl(), {}, {});
    if (tempMenu) {
        tempMenu->exec(QCursor::pos());
        delete tempMenu;
    }
}
