#ifndef DOWNLOADER_HPP
#define DOWNLOADER_HPP

#include <iostream>
#include <QtNetwork>

class Downloader : public QObject{
    Q_OBJECT

    QNetworkAccessManager *manager;
    QNetworkReply *reply;
    QUrl url;
    QFile *file;
    QEventLoop * loop;
public:
    Downloader(QString urlstr, QEventLoop * loop_){
        url = urlstr;
        loop = loop_;
        manager = new QNetworkAccessManager(this);
        reply = manager->get(QNetworkRequest(url));

        QFileInfo info(url.path());
        QString fileName(info.fileName());

        if (fileName.isEmpty()) fileName = "index.html";

        file = new QFile(fileName);

        if(!file->open(QIODevice::WriteOnly))
        {
            qDebug() << "file open error";
            delete file;
            file = 0;
            return;
        }else qDebug() << fileName<< " is downloading ...";

        connect(reply,SIGNAL(finished()),this,SLOT(httpFinished()));
        connect(reply,SIGNAL(readyRead()),this,SLOT(httpReadyRead()));
    }

private slots:
    void httpReadyRead() {
        if (file) file->write(reply->readAll());  //如果文件存在，则写入文件
    }

    void httpFinished() {
        file->flush();
        file->close();
        reply->deleteLater();
        reply = 0;
        delete file;
        file = 0;
        loop->quit();
    }
};

#endif // DOWNLOADER_HPP
