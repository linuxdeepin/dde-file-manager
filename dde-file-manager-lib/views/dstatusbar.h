#ifndef DSTATUSBAR_H
#define DSTATUSBAR_H

#include <dpicturesequenceview.h>
#include <dslider.h>

#include <QStatusBar>
#include <QHBoxLayout>
#include <QLabel>
#include <QSizeGrip>
#include <QPair>
#include "durl.h"

DWIDGET_USE_NAMESPACE

QT_BEGIN_NAMESPACE
class QPushButton;
class QLineEdit;
class QComboBox;
QT_END_NAMESPACE

class DFMEvent;

class DStatusBar : public QFrame
{
    Q_OBJECT
public:
    enum Mode {
        Normal,
        DialogOpen,
        DialogSave
    };

    DStatusBar(QWidget * parent = 0);

    void initUI();
    void initConnect();

    void setMode(Mode mode);
    void setComBoxItems(const QStringList &filters);

    QSlider* scalingSlider() const;
    QPushButton *acceptButton() const;
    QPushButton *rejectButton() const;
    QLineEdit *lineEdit() const;
    QComboBox *comboBox() const;

    QSize sizeHint() const Q_DECL_OVERRIDE;

    qint64 computerSize(const DUrlList& urllist);
    int computerFolderContains(const DUrlList& urllist);

public slots:
    void itemSelected(const DFMEvent &event, int number);
    void updateStatusMessage();
    void handdleComputerFileSizeFinished();
    void handdleComputerFolderContainsFinished();
    void itemCounted(const DFMEvent &event, int number);
    void setLoadingIncatorVisible(bool visible, const QString &tipText = QString());

protected:
    bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;

private:
    void clearLayoutAndAnchors();

    QString m_OnlyOneItemCounted;
    QString m_counted;
    QString m_OnlyOneItemSelected;
    QString m_selected;

    QString m_selectFolders;
    QString m_selectOnlyOneFolder;
    QString m_selectFiles;
    QString m_selectOnlyOneFile;
    QString m_selectedNetworkOnlyOneFolder;

    int m_fileCount = 0;
    qint64 m_fileSize = 0;
    int m_folderCount = 0;
    int m_folderContains = 0;

    QHBoxLayout * m_layout;
    QLabel * m_label = Q_NULLPTR;
    DPictureSequenceView* m_loadingIndicator;
    QSlider* m_scaleSlider;

    QPushButton *m_acceptButton = Q_NULLPTR;
    QPushButton *m_rejectButton = Q_NULLPTR;
    QLineEdit *m_lineEdit = Q_NULLPTR;
    QComboBox *m_comboBox = Q_NULLPTR;
    QLabel *m_lineEditLabel = Q_NULLPTR;
    QLabel *m_comboBoxLabel = Q_NULLPTR;
};

#endif // DSTATUSBAR_H
