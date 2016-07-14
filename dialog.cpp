#include "dialog.h"
#include "ui_dialog.h"
#include <QString>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    portWrite=new QSerialPort(this);
    portRead=new QSerialPort(this);
    initPort();
    portWrite->open(QIODevice::ReadWrite);
    portRead->open(QIODevice::ReadOnly);
    connect(portRead,&QSerialPort::readyRead,this,&Dialog::getData);
}

void Dialog::initPort()
{
    portWrite->setPortName(ui->writePortNumber->text());
    portRead->setPortName(ui->readPortNumber->text());
    portWrite->setBaudRate(QSerialPort::Baud9600);
    portWrite->setDataBits(QSerialPort::Data8);
    portWrite->setParity(QSerialPort::NoParity);
    portWrite->setStopBits(QSerialPort::OneStop);
    portWrite->setFlowControl(QSerialPort::NoFlowControl);
    portRead->setBaudRate(QSerialPort::Baud9600);
    portRead->setDataBits(QSerialPort::Data8);
    portRead->setParity(QSerialPort::NoParity);
    portRead->setStopBits(QSerialPort::OneStop);
    portRead->setFlowControl(QSerialPort::NoFlowControl);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_launch_clicked()
{
    //    portWrite->setPortName(ui->writePortNumber->text());
    //    portRead->setPortName(ui->readPortNumber->text());
    QByteArray data=ui->writePortContent->text().toLocal8Bit();
    portWrite->write(data.fromHex(data));
}

void Dialog::getData()
{
    QByteArray data=portRead->readAll();
    ui->readPortContent->setText(QString(data.toHex()));
}
