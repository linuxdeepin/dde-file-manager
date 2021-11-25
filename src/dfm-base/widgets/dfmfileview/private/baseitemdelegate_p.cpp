#include "baseitemdelegate_p.h"
#include "widgets/dfmfileview/baseitemdelegate.h"
#include "widgets/dfmfileview/fileview.h"

DFMBASE_BEGIN_NAMESPACE

BaseItemDelegatePrivate::BaseItemDelegatePrivate(BaseItemDelegate *qq)
    : q_ptr(qq)
{
}

BaseItemDelegatePrivate::~BaseItemDelegatePrivate()
{
}

DFMBASE_END_NAMESPACE
