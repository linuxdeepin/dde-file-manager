#ifndef TOOLBARFRAME_H
#define TOOLBARFRAME_H

#include <QFrame>
#include <QMediaPlayer>


class QPushButton;
class QSlider;
class QLabel;
class QMediaPlayer;
class QTimer;
class ToolBarFrame : public QFrame
{
    Q_OBJECT
public:
    explicit ToolBarFrame(const QString& uri, QWidget *parent = 0);

    void initUI();
    void initConnections();

signals:

public slots:
    void onPlayStateChanged(const QMediaPlayer::State& state);
    void onPlayStatusChanged(const QMediaPlayer::MediaStatus& status);
    void onPlayControlButtonClicked();
    void updateProgress();
    void seekPosition(const int& pos);

private:
    QMediaPlayer* m_player;
    QPushButton* m_playControlButton;
    QSlider* m_progressSlider;
    QLabel* m_durationLabel;
    QTimer* m_updateProgressTimer;
};

#endif // TOOLBARFRAME_H
