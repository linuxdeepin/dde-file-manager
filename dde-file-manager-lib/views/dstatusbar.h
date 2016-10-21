#ifndef DSTATUSBAR_H
#define DSTATUSBAR_H

#include <dpicturesequenceview.h>
#include <dslider.h>

#include <QStatusBar>
#include <QHBoxLayout>
#include <QLabel>
#include <QSizeGrip>

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

    DSlider* scalingSlider() const;
    QPushButton *acceptButton() const;
    QPushButton *rejectButton() const;
    QLineEdit *lineEdit() const;
    QComboBox *comboBox() const;

public slots:
    void itemSelected(const DFMEvent &event, int number);
    void itemCounted(const DFMEvent &event, int number);
    void setLoadingIncatorVisible(const DFMEvent &event, bool visible);

protected:
    void resizeEvent(QResizeEvent* event);

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

    QHBoxLayout * m_layout;
    QLabel * m_label = Q_NULLPTR;
    DPictureSequenceView* m_loadingIndicator;
    DSlider* m_scaleSlider;

    QPushButton *m_acceptButton = Q_NULLPTR;
    QPushButton *m_rejectButton = Q_NULLPTR;
    QLineEdit *m_lineEdit = Q_NULLPTR;
    QComboBox *m_comboBox = Q_NULLPTR;
};

#endif // DSTATUSBAR_H
