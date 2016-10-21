#ifndef SHAREINFOFRAME_H
#define SHAREINFOFRAME_H

#include <QFrame>
#include <QTimer>

#include <dcheckbox.h>
#include <dcombobox.h>
#include <dlineedit.h>

#include "dabstractfileinfo.h"

DWIDGET_USE_NAMESPACE

class ShareInfoFrame : public QFrame
{
    Q_OBJECT
public:
    explicit ShareInfoFrame(const DAbstractFileInfoPointer &info, QWidget *parent = 0);
    ~ShareInfoFrame();

    void initUI();
    void initShareInfo();
    void initConnect();

signals:

public slots:
    void handleCheckBoxChanged(int state);
    void handleShareNameChanged(const QString& name);
    void handlePermissionComboxChanged(int index);
    void handleAnonymityComboxChanged(int index);
    void handShareInfoChanged();
    void doShaeInfoSetting();

private:
    DAbstractFileInfoPointer m_fileinfo;
    DCheckBox* m_sharCheckBox = NULL;
    DLineEdit* m_shareNamelineEdit = NULL;
    DComboBox* m_permissoComBox = NULL;
    DComboBox* m_anonymityCombox = NULL;
    QTimer* m_jobTimer;
};

#endif // SHAREINFOFRAME_H
