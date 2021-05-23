#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QUrl>
#include <QMainWindow>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QProcess>
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void startSearch();
    void specifyUrlPressed();
    void clearGroupPressed();
    void replyFinished(QNetworkReply *reply);
    void searchReplyFinished(QNetworkReply *reply);
    void itemSelected(const QModelIndex &index);
    void pythonFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QMessageBox *mMessageBox;
    QDialog *idleDialog;
    QProcess *mProcess;
    QNetworkAccessManager *mManager;
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
