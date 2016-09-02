#ifndef DSTATUSBAR_H
#define DSTATUSBAR_H

#include <QStatusBar>
#include <QHBoxLayout>
#include <QLabel>
#include <QSizeGrip>
#include "dpicturesequenceview.h"


DWIDGET_USE_NAMESPACE

class FMEvent;

class DStatusBar : public QFrame
{
    Q_OBJECT
public:
    DStatusBar(QWidget * parent = 0);

    void initUI();
    void initConnect();

public slots:
    void itemSelected(const FMEvent &event, int number);
    void itemCounted(const FMEvent &event, int number);
    void setLoadingIncatorVisible(const FMEvent &event, bool visible);

protected:
    void resizeEvent(QResizeEvent* event);

private:
    QString m_OnlyOneItemCounted;
    QString m_counted;
    QString m_OnlyOneItemSelected;
    QString m_selected;

    QString m_selectFolders;
    QString m_selectOnlyOneFolder;
    QString m_selectFiles;
    QString m_selectOnlyOneFile;

    QHBoxLayout * m_layout;
    QLabel * m_label;
    DPictureSequenceView* m_loadingIndicator;
};

#endif // DSTATUSBAR_H
