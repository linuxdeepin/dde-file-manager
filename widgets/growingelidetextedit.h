#ifndef GROWINGELIDETEXTEDIT_H
#define GROWINGELIDETEXTEDIT_H

#include <QTextEdit>
#include <QKeyEvent>

class GrowingElideTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit GrowingElideTextEdit(QWidget *parent = 0);
    ~GrowingElideTextEdit();

    void setFullText(const QString &text);
    QString getFullText();
    QString getFullWrapText();
    QString getSimpleWrapText();
    QStringList getTexts();
    void elideText();
    bool isSimpleWrapMode();

signals:
    void heightChanged(int height);
    void renameFinished();
    void menuShowed(QPoint pos);
    void returnEnterPressed();

public slots:
    void showSimpleElideText();
    void showFullWrapText();
    void updateSize();
    void setText(const QString &text);
    void showReadOnly();
    void showEditing();\
    void showText();
    void tryRenamed();

protected:
    void keyPressEvent(QKeyEvent* event);
    void mousePressEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
//    void enterEvent(QEvent *event);

private:
    int m_minHeight = 0;
    int m_maxHeight = 6000;
    QString m_fullText;
    QString m_fullWrapText;
    QString m_simpleWrapText;
    QStringList m_texts;
    bool m_simpleWrapMode = true;
};

#endif // GROWINGELIDETEXTEDIT_H
