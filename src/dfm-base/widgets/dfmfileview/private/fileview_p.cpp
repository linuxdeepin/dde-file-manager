#include "fileview_p.h"

DFMFileViewPrivate::DFMFileViewPrivate(DFMFileView *qq)
    : q_ptr(qq)
    , m_headview(new DFMHeaderView(Qt::Orientation::Horizontal, qq))
{

}

int DFMFileViewPrivate::iconModeColumnCount(int itemWidth) const
{
    Q_Q(const DFMFileView);

    int horizontalMargin = 0;

    int contentWidth = q->maximumViewportSize().width();

    return qMax((contentWidth - horizontalMargin - 1) / itemWidth, 1);
}

void DFMFileViewPrivate::updateViewDelegate(const QListView::ViewMode &mode){

    if (QListView::ListMode == mode) {
        m_headview->show();
        if (!m_listDelegate)
            m_listDelegate = new DFMListItemDelegate(q_ptr);
        q_ptr->setItemDelegate(m_listDelegate);
        q_ptr->setIconSize(QSize(GlobalPrivate::LIST_VIEW_ICON_SIZE, GlobalPrivate::LIST_VIEW_ICON_SIZE));
    } else {
        m_headview->hide();
        if (!m_iconDelegate)
            m_iconDelegate = new DFMIconItemDelegate(q_ptr);
        q_ptr->setItemDelegate(m_iconDelegate);
        q_ptr->setIconSize(m_iconDelegate->iconSizeByIconSizeLevel());
    }
    //向上传递，否则setViewMode内部会执行循环导致崩溃
    q_ptr->QListView::setViewMode(mode);
}
