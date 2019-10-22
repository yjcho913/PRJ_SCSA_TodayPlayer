#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QTimer>
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private slots:
    void on_playButton_clicked();

    void on_stopButton_clicked();

    void on_upButton_clicked();

    void on_downButton_clicked();

    void on_addButton_clicked();

    void on_deleteButton_clicked();

    void on_rewindButton_clicked();

    void on_fastforwardButton_clicked();

    void on_offButton_clicked();

    void on_timer_count();

private:
    Ui::MainWindow *ui;
    int fd_pipe[2];
    int fd_pipe_info[2];
    QTimer* timer;
};

#endif // MAINWINDOW_H
