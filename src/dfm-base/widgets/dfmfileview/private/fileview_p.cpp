#include "fileview_p.h"

DFMBASE_BEGIN_NAMESPACE
FileViewPrivate::FileViewPrivate(FileView *qq)
    : q_ptr(qq)
    , m_headview(new HeaderView(Qt::Orientation::Horizontal, qq))
{

}

int FileViewPrivate::iconModeColumnCount(int itemWidth) const
{
    Q_Q(const FileView);

    int horizontalMargin = 0;

    int contentWidth = q->maximumViewportSize().width();

    return qMax((contentWidth - horizontalMargin - 1) / itemWidth, 1);
}

void FileViewPrivate::updateViewDelegate(const QListView::ViewMode &mode){

    if (QListView::ListMode == mode) {
        m_headview->show();
        if (!m_listDelegate)
            m_listDelegate = new ListItemDelegate(q_ptr);
        q_ptr->setItemDelegate(m_listDelegate);
        q_ptr->setIconSize(QSize(GlobalPrivate::LIST_VIEW_ICON_SIZE, GlobalPrivate::LIST_VIEW_ICON_SIZE));
    } else {
        m_headview->hide();
        if (!m_iconDelegate)
            m_iconDelegate = new IconItemDelegate(q_ptr);
        q_ptr->setItemDelegate(m_iconDelegate);
        q_ptr->setIconSize(m_iconDelegate->iconSizeByIconSizeLevel());
    }
    //向上传递，否则setViewMode内部会执行循环导致崩溃
    q_ptr->QListView::setViewMode(mode);
}

DFMBASE_END_NAMESPACE
