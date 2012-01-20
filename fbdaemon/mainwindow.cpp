#include "fbdialog.h"
#include "mainwindow.h"

#include <QtGui>

CMainWindow::CMainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QWidget *w = new QWidget;
    setCentralWidget(w);
    QVBoxLayout *vbox = new QVBoxLayout(w);

    QHBoxLayout *hbox = new QHBoxLayout;
    vbox->addLayout(hbox);

    hbox->addWidget(devEdit = new QLineEdit("/dev/fb1"));

    hbox->addWidget(openDevButton = new QPushButton("Open"));
    connect(openDevButton, SIGNAL(clicked()), SLOT(openDevice()));

    vbox->addWidget(imageLabel = new QLabel);

    imageUpdateTimer = new QTimer(this);
    connect(imageUpdateTimer, SIGNAL(timeout()), SLOT(updateImage()));
    imageUpdateTimer->setInterval(1000/30);
}

void CMainWindow::openDevice()
{
    if (devFile.isOpen())
    {
        devFile.close();
        openDevButton->setText("Open");
        imageUpdateTimer->stop();
    }
    else
    {
        devFile.setFileName(devEdit->text());
        if (!devFile.open(QFile::ReadOnly))
        {
            QMessageBox::critical(this, "fb error", "Failed to open fb device!");
            return;
        }

        openDevButton->setText("Close");
        devMap = devFile.map(0, 130 * 130 * 2);
        imageUpdateTimer->start(1000/30);

        //    for (int i=0; i<200; ++i)
        //        qDebug() << QString("[%1] = %2").arg(i).arg((int)mappedData[i]);
    }
}

void CMainWindow::updateImage()
{
#if 0
    QImage img(130, 130, QImage::Format_RGB16);
    img.fill(qRgb(0, 0, 0));

    QPainter painter(&img);
    for (int x=0; x<130; ++x)
    {
        for (int y=0; y<130; ++y)
        {
            const uint16_t px = *(uint16_t*)(devMap + x * 2 + y * 130 * 2);
            const uint16_t r = ((px >> 11) & 0x1F) << 3;
            const uint16_t g = ((px >> 5) & 0x3F) << 2;
            const uint16_t b = (px & 0x1F) << 3;

            painter.setPen(QColor(r, g, b));
            painter.drawPoint(x, y);
        }
    }
#else
    QImage img(devMap, 130, 130, QImage::Format_RGB16);
#endif

    imageLabel->setPixmap(QPixmap::fromImage(img));
}
