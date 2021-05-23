#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <libxml2/libxml/HTMLparser.h>
#include <libxml2/libxml/parser.h>
#include <libxml2/libxml/tree.h>

#include <QInputDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDesktopServices>
#include <QUrl>

#define BORDER 100

//https://realpython.com/python-sleep/
//Graphical User Interfaces
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , mMessageBox(new QMessageBox(this))
    , idleDialog(new QDialog(this))
    , mManager(new QNetworkAccessManager(this))
    , mProcess(new QProcess(this))
{
    ui->setupUi(this);

    mMessageBox->setModal(true);
    mMessageBox->setWindowTitle("Wait please!");
    mMessageBox->setText("      Idle...     ");
    mMessageBox->setStandardButtons(0);
    mMessageBox->setMinimumHeight(300);
    mMessageBox->setMinimumHeight(400);

    connect(mManager, &QNetworkAccessManager::finished, this, &MainWindow::replyFinished);

    connect(mProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &MainWindow::pythonFinished);

    connect(ui->specifyUrlPushButton, &QPushButton::pressed, this, &MainWindow::specifyUrlPressed);
    connect(ui->lineEdit, &QLineEdit::returnPressed, this, &MainWindow::startSearch);
    connect(ui->resultListWidget, &QListWidget::clicked, this, &MainWindow::itemSelected);
    ui->resultListWidget->setSpacing(4);

    ui->resultListWidget->setSpacing(4);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::startSearch()
{
    QUrl serviceUrl = QUrl("http://localhost:9200/_search?pretty");
    auto requestText = ui->lineEdit->text();
    QByteArray postData;
    postData.append("{\
                    \"_source\": [\
                      \"title\",\
                      \"content\"\
                    ],\
                    \"query\": {\
                      \"match\": {\
                        \"content\": \"" + requestText + "\"\
                      }\
                    }\
                  }");
    // Call the webservice
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);

    QNetworkRequest request(serviceUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(QString("text/xml")));

   connect(manager, &QNetworkAccessManager::finished, this, &MainWindow::searchReplyFinished);
   manager->post(request, postData);
}

void MainWindow::specifyUrlPressed()
{
    bool ok;
    QString text = QInputDialog::getText(0, "Input dialog",
                                         "Specify Url:", QLineEdit::Normal,
                                         "", &ok);
    if (ok && !text.isEmpty())
    {
        qDebug() << "specified URL " << text;
        QStringList arguments { "/home/max/InformationGatheringTool/sendDataScript.py", QString("-url=%1").arg(text) };
        mProcess->start("python3", arguments);
        mMessageBox->show();
        mProcess->waitForFinished();
    }
}

void MainWindow::clearGroupPressed()
{

}

void MainWindow::replyFinished(QNetworkReply *reply)
{
    QByteArray buffer = reply->readAll();

    qDebug()<<"Reply received! "<< buffer;
}

void MainWindow::searchReplyFinished(QNetworkReply *reply)
{
    QByteArray buffer = reply->readAll();
    ui->resultListWidget->clear();
    if (buffer.size())
    {
        QString title;
        QString content;
        double score= 0.0;

        QJsonDocument document = QJsonDocument::fromJson(buffer);

        QJsonObject root = document.object();

        QJsonValue hitsExternalVal = root.value("hits");
        if (hitsExternalVal.isObject())
        {
            QJsonObject hitsExternalObj = hitsExternalVal.toObject();
            QJsonValue hitsVal = hitsExternalObj.value("hits");
            if(hitsVal.isArray()){
                QJsonArray hitsArray = hitsVal.toArray();

                for(int i = 0; i < hitsArray.count(); i++){
                    QJsonObject subtree = hitsArray.at(i).toObject();
                    score = subtree.value("_score").toDouble();
                    QJsonValue sourceVal = subtree.value("_source");
                    if (sourceVal.isObject())
                    {
                        QJsonObject sourceObj = sourceVal.toObject();
                        title = sourceObj.value("title").toString();
                        content = sourceObj.value("content").toString();
                        auto simplifiedString = content.simplified();
                        int index = simplifiedString.indexOf(ui->lineEdit->text());
                        QString stringToShow = simplifiedString.left(BORDER);
                        if (index > 0)
                        {
                            int count = index + BORDER < simplifiedString.size() ? BORDER : simplifiedString.size() - index;
                            stringToShow = simplifiedString.mid(index, count);
                        }
                        ui->resultListWidget->addItem(QString("%1 - score = %2\n%3").arg(title).arg(score).arg(stringToShow));
                    }
                }
            }
        }
    }

    qDebug()<<"Reply received! "<< buffer;
}

void MainWindow::itemSelected(const QModelIndex &index)
{
    QString str = index.data().toString();
    QString url = str.split(" ").at(0);
    url.replace("website", "");
    QDesktopServices::openUrl(QUrl(url));
    qDebug()<< url;
}

void MainWindow::pythonFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug()<< "Python script finished with code "<< exitCode;
    mMessageBox->hide();
}

