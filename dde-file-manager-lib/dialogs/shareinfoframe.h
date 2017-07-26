#ifndef SHAREINFOFRAME_H
#define SHAREINFOFRAME_H

#include <QFrame>
#include <QTimer>

#include "dabstractfileinfo.h"

QT_BEGIN_NAMESPACE
class QComboBox;
class QCheckBox;
class QLineEdit;
QT_END_NAMESPACE

class ShareInfoFrame : public QFrame
{
    Q_OBJECT
public:
    explicit ShareInfoFrame(const DAbstractFileInfoPointer &info, QWidget *parent = 0);
    ~ShareInfoFrame();

    void initUI();
    void initConnect();
    void setFileinfo(const DAbstractFileInfoPointer &fileinfo);

signals:
    void folderShared(const QString& filePath);

public slots:
    void handleCheckBoxChanged(const bool &checked);
    void handleShareNameChanged(const QString& name);
    void handlePermissionComboxChanged(const int& index);
    void handleAnonymityComboxChanged(const int& index);
    void handShareInfoChanged();
    void doShareInfoSetting();
    void updateShareInfo(const QString& filePath);
    void activateWidgets();
    void disactivateWidgets();

private:
    DAbstractFileInfoPointer m_fileinfo;
    QCheckBox* m_shareCheckBox = NULL;
    QLineEdit* m_shareNamelineEdit = NULL;
    QComboBox* m_permissoComBox = NULL;
    QComboBox* m_anonymityCombox = NULL;
    QTimer* m_jobTimer;
};

#endif // SHAREINFOFRAME_H
