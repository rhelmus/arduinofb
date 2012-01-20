#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFile>
#include <QtGui/QMainWindow>

class QLabel;
class QLineEdit;
class QPushButton;
class QTimer;

class CMainWindow : public QMainWindow
{
    Q_OBJECT

    QLineEdit *devEdit;
    QPushButton *openDevButton;
    QLabel *imageLabel;
    QTimer *imageUpdateTimer;
    QFile devFile;
    uchar *devMap;

private slots:
    void openDevice(void);
    void updateImage(void);

public:
    CMainWindow(QWidget *parent = 0);
};

#endif // MAINWINDOW_H
