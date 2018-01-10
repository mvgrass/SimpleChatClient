#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QTcpSocket>
#include <QLabel>
#include <map>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent*);

private slots:
    void on_pushButton_3_clicked();

    void on_pushButton_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_2_clicked();

    void ReadData();

    void Error(QAbstractSocket::SocketError);


private:
    Ui::MainWindow *ui;
    QTcpSocket* Socket = nullptr;
    std::map<QString, QLabel*> users;
    bool CheckFields();
};

#endif // MAINWINDOW_H
