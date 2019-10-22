#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <stdio.h>
#include <signal.h>
#include <QFileDialog>
#include <QTextStream>
#include <QFile>
#include <QDataStream>

#define MAX_BUF (2000)

volatile int play_flag;
volatile int pause_flag;
volatile int existed_idx;
int vol = 30;
char strvol[4];
const char *ans_meta = "ANS_META_";
char max_buf[MAX_BUF];
volatile float end_time;

MainWindow::MainWindow(QWidget *parent) :
QMainWindow(parent),
ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    timer = new QTimer;
    connect(timer, SIGNAL(timeout()), this, SLOT(on_timer_count()));
    timer->setInterval(1000);

    this->setStyleSheet("background-color: white;");

    play_flag = 0;
    pause_flag = 1;
    if (pipe(fd_pipe) == -1) close();
    if (pipe(fd_pipe_info) == -1) close();

    // 시작 화면
    QStringList list = QFileDialog::getOpenFileNames(this,
        tr("Select Files"), "/mnt/nfs/musics",
        tr("MP3 Files (*.mp3 *.wav)"));
    if (list.isEmpty()) return;
    foreach(QString x, list)
    {
        ui->playList->addItem(x.split("/")[4]);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_playButton_clicked()
{
    pid_t pid_temp;

    if (!ui->playList->count()) return;
    if (!pause_flag && !play_flag) return;
    if (ui->playList->currentRow() == -1) ui->playList->setCurrentRow(0);

    if (play_flag && existed_idx != ui->playList->currentRow())
    {
        write(fd_pipe[1], "quit\n", 5);
        play_flag = 0;
        pause_flag = 1;
    }

    // 재생 상태
    if (play_flag && !pause_flag)
    {
        pause_flag = 1;
        ui->playButton->setStyleSheet("border-image: url(:/images/images/play.png);");
        write(fd_pipe[1], "pause\n", 6);
        return;
    }

    // 일시정지 상태
    if (play_flag && pause_flag)
    {
        pause_flag = 0;
        ui->playButton->setStyleSheet("border-image: url(:/images/images/pause.png);");
        write(fd_pipe[1], "pause\n", 6);
        return;
    }

    // 기존음악 없는 상태
    ui->startLabel->setText(QString("00.0"));
    pid_temp = fork();
    if (pid_temp == -1) exit(-1);
    else if (pid_temp == 0){
        char filepath[256] = "/mnt/nfs/musics/";
        strcat(filepath, ui->playList->currentItem()->text().toStdString().c_str());
        ::close(0);
        dup(fd_pipe[0]);
        ::close(fd_pipe[0]);
        ::close(fd_pipe[1]);

        ::close(1);
        dup(fd_pipe_info[1]);
        ::close(fd_pipe_info[0]);
        ::close(fd_pipe_info[1]);

        sprintf(strvol, "%d", vol);
        execlp("/mnt/nfs/mplayer", "mplayer", "-slave", "-quiet", "-volume", strvol, "-srate", "44100", filepath, NULL);
    }
    else {
        char *tmp;

        ui->playButton->setStyleSheet("border-image: url(:/images/images/pause.png);");

        // 앨범사진
        QString musicpath = ui->playList->currentItem()->text();
        QString dir = QString("border-image: url(:/musics/musics/%1.png);").arg(musicpath.split(".")[0]);
        ui->albumImage->setStyleSheet(dir);

        // 가사
        dir = QString("/mnt/nfs/musics/text/%1.txt").arg(musicpath.split(".")[0]);
        QFile textFile(dir);
        QString line;
        ui->lyricText->clear();
        if (textFile.open(QIODevice::ReadOnly | QIODevice::Text)){
            QTextStream stream(&textFile);
            while (!stream.atEnd()){
                line = stream.readLine();
                ui->lyricText->setText(ui->lyricText->toPlainText()+line+"\n");
            }
        }
        textFile.close();

        // 제목
        write(fd_pipe[1], "get_meta_title\n", 15);
        usleep(500000);
        read(fd_pipe_info[0], max_buf, MAX_BUF-1);
        tmp = strstr(max_buf,ans_meta);
        strchr(tmp, '\n')[0] = 0;
        ui->titleLabel->setText(&tmp[9]);

        // 가수
        write(fd_pipe[1], "get_meta_artist\n", 16);
        usleep(500000);
        read(fd_pipe_info[0], max_buf, MAX_BUF-1);
        tmp = strstr(max_buf,ans_meta);
        strchr(tmp, '\n')[0] = 0;
        ui->artistLabel->setText(&tmp[9]);

        // 연도
        write(fd_pipe[1], "get_meta_year\n", 14);
        usleep(300000);
        read(fd_pipe_info[0], max_buf, MAX_BUF-1);
        tmp = strstr(max_buf,ans_meta);
        strchr(tmp, '\n')[0] = 0;
        ui->yearLabel->setText(&tmp[9]);

        // 전체시간
        write(fd_pipe[1], "get_time_length\n", 16);
        usleep(300000);
        read(fd_pipe_info[0], max_buf, MAX_BUF-1);
        tmp = strstr(max_buf,"ANS_LENGTH=");
        strchr(tmp, '\n')[0] = 0;
        ui->endLabel->setText(&tmp[11]);
        end_time = atof(&tmp[11]);
        ui->slider->setRange(0, (int)end_time);
        play_flag = 1;
        pause_flag = 0;
        existed_idx = ui->playList->currentRow();
        timer->start();
        return;
    }
}

void MainWindow::on_stopButton_clicked()
{
    if (play_flag == 0) return;
    write(fd_pipe[1], "quit\n", 5);
    pause_flag = 1;
    play_flag = 0;
    ui->playButton->setStyleSheet("border-image: url(:/images/images/play.png);");
}

void MainWindow::on_upButton_clicked()
{
    char cmd[15];

    if(pause_flag) return;
    vol += 5;
    if (vol > 100) vol = 100;
    sprintf(cmd, "volume %3d 1\n", vol);
    write(fd_pipe[1], cmd, 13);
}

void MainWindow::on_downButton_clicked()
{
    char cmd[15];

    if(pause_flag) return;
    vol -= 5;
    if (vol < 0) vol = 0;
    sprintf(cmd, "volume %3d 1\n", vol);
    write(fd_pipe[1], cmd, 13);
}

void MainWindow::on_addButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Music"), "/mnt/nfs/musics", tr("Music Files (*.mp3 *.wav)"));
    if(fileName.isEmpty()) return;
    ui->playList->addItem(fileName.split("/")[4]);
}

void MainWindow::on_deleteButton_clicked()
{
    delete ui->playList->takeItem(ui->playList->currentRow());
}

void MainWindow::on_rewindButton_clicked()
{
    ui->playButton->setStyleSheet("border-image: url(:/images/images/play.png);");
    if (existed_idx < 1) return;
    ui->playList->setCurrentRow(existed_idx - 1);
    on_playButton_clicked();
}

void MainWindow::on_fastforwardButton_clicked()
{
    ui->playButton->setStyleSheet("border-image: url(:/images/images/play.png);");
    if (existed_idx >= ui->playList->count() - 1) return;
    ui->playList->setCurrentRow(existed_idx + 1);
    on_playButton_clicked();
}

void MainWindow::on_offButton_clicked()
{
    if(pause_flag == 0)
    {
        write(fd_pipe[1], "quit\n", 5);
        sleep(1);
    }
    close();
}

void MainWindow::on_timer_count()
{
    static char *tmp;

    if(pause_flag) return;

    write(fd_pipe[1], "get_time_pos\n", 13);
    usleep(300000);
    read(fd_pipe_info[0], max_buf, MAX_BUF-1);
    tmp = strstr(max_buf,"ANS_TIME_POSITION=");
    strchr(tmp, '\n')[0] = 0;
    ui->startLabel->setText(&tmp[18]);
    ui->slider->setValue(atoi(&tmp[18]));
    if(end_time - atof(&tmp[18]) < 1.0)
    {
        timer->stop();
        on_fastforwardButton_clicked();
    }
}
