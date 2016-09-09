#ifndef EXTENDVIEW_H
#define EXTENDVIEW_H

#include <QFrame>
#include "durl.h"
class QListWidget;
class DDetailView;


class ExtendView : public QFrame
{
    Q_OBJECT
public:
    explicit ExtendView(QWidget *parent = 0);
    ~ExtendView();
    void initUI();
    void initConnect();

signals:

public slots:
    void setStartUrl(const DUrl& url);

private:
    QListWidget* m_extendListView = NULL;
    DDetailView* m_detailView = NULL;
    DUrl m_startUrl;
    DUrl m_currentUrl;
};

#endif // EXTENDVIEW_H
