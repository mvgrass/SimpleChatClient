#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDateTime>
#include <QSound>
#include <vector>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->scrollArea->setWidget(new QWidget());
    ui->scrollArea->widget()->setLayout(new QVBoxLayout);
    ui->scrollArea->widget()->layout()->setAlignment(Qt::AlignTop|Qt::AlignHCenter);

    QObject::connect(ui->lineEdit_4, SIGNAL(returnPressed()), this, SLOT(on_pushButton_2_clicked()));

    ui->lineEdit_3->setValidator(new QIntValidator(1, 65535, this));
    QString ipRange = "(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])";
    QRegExp ipRegex ("^" + ipRange
                     + "\\." + ipRange
                     + "\\." + ipRange
                     + "\\." + ipRange + "$");
    QRegExpValidator *ipValidator = new QRegExpValidator(ipRegex, this);
    ui->lineEdit_2->setValidator(ipValidator);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_3_clicked()
{
    ui->textBrowser->setText("");
}

void MainWindow::on_pushButton_clicked()
{
    if(ui->lineEdit->text()==""||ui->lineEdit_2->text()==""||ui->lineEdit_3->text()==""){
        QString str = "Заполните поля: ";
        if(ui->lineEdit->text()=="")
            str.append("Имя ");
        if(ui->lineEdit_2->text()=="")
            str.append("Сервер ");
        if(ui->lineEdit_3->text()=="")
            str.append("Порт ");

        ui->label_4->setText(str);

        return;

    }

    Socket = new QTcpSocket();

    QString ip = ui->lineEdit_2->text();
    quint16 port = ui->lineEdit_3->text().toInt();

    Socket->connectToHost(ip, port, QTcpSocket::ReadWrite, QTcpSocket::IPv4Protocol);

    if(!Socket->waitForConnected()){
        ui->label_4->setText("Can't connect to server "+Socket->errorString());
        delete Socket;
        Socket = nullptr;
        return;
    }

    Socket->write(ui->lineEdit->text().toUtf8());
    Socket->waitForReadyRead();
    QString ans = QString::fromUtf8(Socket->readAll().toStdString().c_str());
    if(ans == "n"){
        ui->label_4->setText("Данный никнейм уже занят");
        return;
    }

    Socket->write("ok");

    Socket->waitForReadyRead();
    std::vector<QString> Users;
    QString User;
    do{
        User = QString::fromUtf8(Socket->readAll().toStdString().c_str());
        if(User!="\n"){
            Socket->write("ok");
            Users.push_back(User);
            Socket->waitForReadyRead();
        }
    }while(User!="\n");

    QObject::connect(Socket, SIGNAL(readyRead()), this, SLOT(ReadData()));
    QObject::connect(Socket, SIGNAL(error(QAbstractSocket::SocketError)),
                     this,         SLOT(Error(QAbstractSocket::SocketError)));

    ui->label_4->setText("Connected");

    for(auto User: Users){
        QLabel* new_user_l = new QLabel;
        new_user_l->setText(User);
        users[User] = new_user_l;
        ui->scrollArea->widget()->layout()->addWidget(new_user_l);
    }

    ui->pushButton_4->setEnabled(true);
    ui->pushButton->setEnabled(false);
}

void MainWindow::on_pushButton_4_clicked()
{
    Socket->disconnectFromHost();

    Socket->waitForDisconnected();

    delete Socket;
    Socket = nullptr;

    while(!users.empty()){
        delete users.begin()->second;
        users.erase(users.begin());
    }

    ui->label_4->setText("");
    ui->pushButton->setEnabled(true);
    ui->pushButton_4->setEnabled(false);
}

void MainWindow::on_pushButton_2_clicked()
{
    if(Socket!=nullptr&&Socket->isOpen()&&ui->lineEdit_4->text()!=""){
        Socket->write(ui->lineEdit_4->text().append("\n").toUtf8());
    }

    ui->lineEdit_4->setText("");
}

void MainWindow::ReadData(){
    QString str = QString::fromUtf8(Socket->readAll().toStdString().c_str());

    QString finalMessage = "[";
    finalMessage.append(QDateTime::currentDateTime().time().toString());
    finalMessage.append("] ");

    int length = str.length();

    if(str[0]=='\n'){
        QLabel* n_user = new QLabel();
        users[str.mid(1,length-2)] = n_user;
        n_user->setText(str.mid(1,length-2));
        ui->scrollArea->widget()->layout()->addWidget(n_user);
        finalMessage.append("Подключился: ").append(str.right(length-1));
    }else if(str[0]=='\r'){
        auto name = users.find(str.mid(1,length-2));
        delete name->second;
        users.erase(name->first);
        finalMessage.append("Отключился: ").append(str.right(length-1));
    }else
        finalMessage.append(str);

    ui->textBrowser->append(finalMessage);

    if(!this->isActiveWindow()){
        QSound::play("tuturu.wav");
    }

}

void MainWindow::Error(QAbstractSocket::SocketError err)
{
    QString strError =
           "Error: " + (err == QAbstractSocket::HostNotFoundError ?
                        "The host was not found." :
                        err == QAbstractSocket::RemoteHostClosedError ?
                        "The remote host is closed." :
                        err == QAbstractSocket::ConnectionRefusedError ?
                        "The connection was refused." :
                        QString(Socket->errorString())
                       );

    ui->label_4->setText(strError);
}

void MainWindow::closeEvent(QCloseEvent *event){
    if(Socket!=nullptr&&Socket->isOpen())
        on_pushButton_4_clicked();

    QMainWindow::closeEvent(event);
}
