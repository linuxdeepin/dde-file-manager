#ifndef DSEARCHBAR_H
#define DSEARCHBAR_H

#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QListView>
#include <QMenu>
#include <QHBoxLayout>
#include <QCompleter>
#include <QStringListModel>
#include <QStringList>
#include <QDirModel>


class DSearchBar : public QLineEdit
{
    Q_OBJECT
public:
    explicit DSearchBar(QWidget *parent = 0);
    ~DSearchBar();
    void setPopup(QListWidget * popup);
    QListWidget * getPopupList();
    QAction * setClearAction();
    QAction * removeClearAction();
    bool isActive();
    void setActive(bool active);
    void keyPressEvent(QKeyEvent *e);
    QAction * getClearAction();
    bool hasScheme();
    bool isSearchFile();
    bool isBookmarkFile();
    bool isComputerFile();
    bool isLocalFile();
    bool isTrashFile();
    bool isPath();
private:
    void initData();
    void initUI();
    void keyUpDown(int key);
    void recomended();
    void complete(const QString & str);
    QStringList splitPath(const QString &path);
    QListWidget * m_list;
    QCompleter * m_completer;
    QCompleter * m_historyCompleter;
    QAction * m_clearAction;
    QStringListModel * m_stringListMode;
    QStringList m_historyList;
    QDirModel * m_dirModel;
    bool m_isActive;
    void initConnections();
    QString m_text;
    bool m_disableCompletion = false;
public slots:
    void doTextChanged(QString text);
    void searchHistoryLoaded(const QStringList &list);
    void historySaved();
    void setCompleter(const QString &text);
    void completeText(QListWidgetItem * item);
protected:
    void focusInEvent(QFocusEvent *e);
    void focusOutEvent(QFocusEvent *e);
    bool event(QEvent *e);
    bool eventFilter(QObject *obj, QEvent *e);
    void resizeEvent(QResizeEvent *e);
    void moveEvent(QMoveEvent *e);
signals:
    void searchBarFocused();
    void focusedOut();
};

#endif // DSEARCHBAR_H
