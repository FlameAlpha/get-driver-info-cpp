#include <QCoreApplication>
#include <QObject>
#include <QtCore>
#include <downloader.hpp>
#include <dataframe.hpp>
#include <unordered_map>
#include <iostream>
#include <cstdio>
#include <QDateTime>

QString getSpaceStr(int num, char delimeter = ' ') {
    QString temp;
    while (num--) {
        temp.append(delimeter);
    }
    return temp;
}

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    QFile file("URLSTR");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Please enter the correct URL of your mi computer in the file!";
    } else {
        QByteArray line = file.readLine();
        QString URLSTR = QString::fromLocal8Bit(line);
        if(line.size() <= 0) {
            qDebug() << "Please enter the correct URL of your mi computer in the file!";
        } else {
            QUrl url(URLSTR);
            QNetworkAccessManager manager;
            QEventLoop loop;
            QNetworkReply *reply;

            qDebug() << "Reading html code from " << URLSTR;
            reply = manager.get(QNetworkRequest(url));
            QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
            loop.exec();

            QString codeContent = reply->readAll();

            QString str_1 = "main_body\":\\\{\"section\":\\\[\\\{\"body\":\\\{\"section\":\\\[(.*)\\\].*view_type\":\"driver_info";

            QRegExp rx_1(str_1);

            std::unordered_map<int, std::vector<std::vector<user_variant>>> dict;
            std::vector<flame::dataframe < user_variant> * > type_dataframe;

            int max_length_name = 0;
            int pos_1 = 0;
            pos_1 = rx_1.indexIn(codeContent, pos_1);
            std::vector<bool> no_file = {false, false, false, false, false};
            if (pos_1 >= 0) {
                QString str_2 = "\\\{\"items\":\\\[(.*)\\\],\"title\":\"(.*)\"\\\}";
                QRegExp rx_2(str_2);
                rx_2.setMinimal(true);
                int pos_2 = 0;
                int type_num = 0;
                while (pos_2 >= 0) {
                    pos_2 = rx_2.indexIn(rx_1.cap(1), pos_2);
                    if (pos_2 >= 0) {
                        ++pos_2;
                        auto data_save = new flame::dataframe(rx_2.cap(2).toLocal8Bit().toStdString());
                        if (data_save->column_num() == 0) {
                            data_save->column_paste({"name", "size", "download URL"});
                            if (no_file.size() > type_num)
                                no_file[type_num] = true;
                            else no_file.push_back(true);
                        }
                        type_dataframe.push_back(data_save);
                        QString str_3 = "\\\{\"description\":\"(.*)\",\"download_url\":\"(.*)\",\"id\":(.*),\"img_url\":\"(.*)\",\"title\":\"(.*)\"\\\}";
                        QRegExp rx_3(str_3);
                        rx_3.setMinimal(true);
                        int pos_3 = 0;
                        int row_num = 0;
                        while (pos_3 >= 0) {
                            pos_3 = rx_3.indexIn(rx_2.cap(1), pos_3);
                            if (pos_3 >= 0) {
                                ++pos_3;

                                if (int(data_save->row_num()) - 1 < row_num) {
                                    data_save->append({rx_3.cap(5).replace(" ", "").toStdString(),
                                                       rx_3.cap(1).replace(" ", "").toStdString(),
                                                       rx_3.cap(2).replace(" ", "").toStdString()});
                                    if (no_file.size() > type_num)
                                        no_file[type_num] = true;
                                    else no_file.push_back(true);
                                } else if (std::get<std::string>((*data_save)[row_num][2]) !=
                                           rx_3.cap(2).replace(" ", "").toStdString() ||
                                           std::get<std::string>((*data_save)[row_num][1]) !=
                                           rx_3.cap(1).replace(" ", "").toStdString() ||
                                           std::get<std::string>((*data_save)[row_num][0]) !=
                                           rx_3.cap(5).replace(" ", "").toStdString()) {
                                    if (dict.find(type_num) != dict.end()) {
                                        dict[type_num].emplace_back(std::vector<user_variant>{row_num,
                                                                                              rx_3.cap(5).replace(" ",
                                                                                                                  "").toStdString(),
                                                                                              rx_3.cap(1).replace(" ",
                                                                                                                  "").toStdString(),
                                                                                              rx_3.cap(2).replace(" ",
                                                                                                                  "").toStdString()});
                                    } else {
                                        dict.insert(std::pair<int, std::vector<std::vector<user_variant>>>(type_num, {
                                                {row_num, rx_3.cap(5).replace(" ", "").toStdString(),
                                                        rx_3.cap(1).replace(" ", "").toStdString(),
                                                        rx_3.cap(2).replace(" ", "").toStdString()}}));
                                    }
                                }

                                if (max_length_name < rx_3.cap(5).replace(" ", "").toLocal8Bit().size())
                                    max_length_name = rx_3.cap(5).replace(" ", "").toLocal8Bit().size();

                                row_num++;
                            }
                        }
                    }
                    type_num++;
                }

                std::cout << "Infomation of last upgrade packages of drivers : " << std::endl;
                for (auto items = type_dataframe.begin(); items != type_dataframe.end(); items++) {
                    std::cout << (*items)->name() << std::endl;
                    for (int index = 0; index < (*items)->row_num(); index++) {
                        QString DownloadURL = QString::fromStdString(std::get<std::string>((*(*items))[index][2]));
                        qDebug().noquote().nospace() << " |—"
                                                     << QString::fromStdString(std::get<std::string>((*(*items))[index][0]))
                                                     << getSpaceStr(max_length_name - QString::fromStdString(
                                                             std::get<std::string>(
                                                                     (*(*items))[index][0])).toLocal8Bit().size())
                                                     << "\t Size : "
                                                     << QString::fromStdString(std::get<std::string>((*(*items))[index][1]))
                                                     << "\t Time : "
                                                     << (QDateTime::fromString(*(DownloadURL.split('/').end() - 2),
                                                                               "yyyyMMdd").isValid() ? *(
                                                             DownloadURL.split('/').end() - 2) : "xxxxxxxx")
                                                     << "\t Download URL : " << DownloadURL;
                    }
                    std::cout << std::endl;
                }
                if (!dict.empty())
                    std::cout << "Available upgrade installation packages of current drivers : " << std::endl;
                for (auto items = dict.begin(); items != dict.end(); items++) {
                    std::cout << type_dataframe[items->first]->name() << std::endl;
                    for (auto item = items->second.begin(); item != items->second.end(); item++) {
                        QString DownloadURL = QString::fromStdString(std::get<std::string>((*item)[3]));
                        qDebug().noquote().nospace() << " |—"
                                                     << QString::fromStdString(std::get<std::string>((*item)[1]))
                                                     << getSpaceStr(max_length_name - QString::fromStdString(
                                                             std::get<std::string>((*item)[1])).toLocal8Bit().size())
                                                     << "\t Size : "
                                                     << QString::fromStdString(std::get<std::string>((*item)[2]))
                                                     << "\t Time : "
                                                     << (QDateTime::fromString(*(DownloadURL.split('/').end() - 2),
                                                                               "yyyyMMdd").isValid() ? *(
                                                             DownloadURL.split('/').end() - 2) : "xxxxxxxx")
                                                     << "\t Download URL : " << DownloadURL;
                    }
                    std::cout << std::endl;
                }

                if (!dict.empty()) {
                    std::string answer;
                    std::cout << "Update drivers or not [Yes/No] : ";
                    std::cin >> answer;
                    if (answer == "Yes") {
                        std::cout << "Download program installation package automatically or not [Yes/No] : ";
                        answer.clear();
                        std::cin >> answer;

                        QFile *file = nullptr;
                        bool file_ok = true;
                        if (answer == "No") {
                            file = new QFile("ChangeLog.TXT");
                            if (!file->open(QIODevice::WriteOnly | QIODevice::Text)) {
                                std::cout << "Write into info file failed !";
                                file_ok = false;
                            }
                        }

                        for (auto items = dict.begin(); items != dict.end(); items++) {
                            for (auto item = items->second.begin(); item != items->second.end(); item++) {
                                (*type_dataframe[items->first])[std::get<int>((*item)[0])] = {(*item)[1], (*item)[2],
                                                                                              (*item)[3]};
                                if (answer == "Yes") {
                                    Downloader downloader(QString::fromStdString(std::get<std::string>((*item)[3])), &loop);
                                    loop.exec();
                                } else if (file_ok) {
                                    QTextStream out(file);
                                    QString DownloadURL = QString::fromStdString(std::get<std::string>((*item)[3]));
                                    out << QString::fromStdString(std::get<std::string>((*item)[1]))
                                        << getSpaceStr(max_length_name - QString::fromStdString(
                                                std::get<std::string>((*item)[1])).toLocal8Bit().size())
                                        << "\t Size : " << QString::fromStdString(std::get<std::string>((*item)[2]))
                                        << "\t Time : "
                                        << (QDateTime::fromString(*(DownloadURL.split('/').end() - 2), "yyyyMMdd").isValid()
                                            ? *(DownloadURL.split('/').end() - 2) : "xxxxxxxx")
                                        << "\t Download URL : " << DownloadURL << '\n';
                                }
                                type_dataframe[items->first]->to_csv();
                            }
                        }

                        if (answer == "No") {
                            file->close();
                            delete file;
                            if (file_ok) {
                                qDebug() << "Change information has been written to ChangeLog.TXT the file, please update the drivers manually !";
                            }
                        }
                    }
                } else qDebug() << "No upgrade package of driver !";

                type_num = 0;
                for (auto no_file_flag = no_file.begin(); no_file_flag != no_file.end(); no_file_flag++, type_num++) {
                    if (*no_file_flag) {
                        type_dataframe[type_num]->to_csv();
                    }
                }

                for (auto dataframe_item = type_dataframe.begin(); dataframe_item != type_dataframe.end(); dataframe_item++)
                    delete *dataframe_item;
            } else qDebug() << "Information of drivers  is parsed failed, please check your URL !";
        }
    }
    system("pause");
    return 0;
}
