#include "dialog.h"
#include "ui_dialog.h"
#include <QString>
#define BYTE_NUMBER_VOLTAGE 22
#define BYTE_NUMBER_CURRENT 25
#define BYTE_NUMBER_EFFECTIVE_POWER 28
#define BYTE_NUMBER_REACTIVE_POWER 28
#define BYTE_NUMBER_APPRENT_POWER 28
#define BYTE_NUMBER_POWER_FACTOR 24

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    portWrite=new QSerialPort(this);
    portRead=new QSerialPort(this);
    initPort();
    initData();
    portWrite->open(QIODevice::ReadWrite);
//    portRead->open(QIODevice::ReadOnly);
    connect(portWrite,&QSerialPort::readyRead,this,&Dialog::getData);
}

void Dialog::initPort()
{
    portWrite->setPortName(ui->writePortNumber->text());
    portRead->setPortName(ui->readPortNumber->text());
    portWrite->setBaudRate(QSerialPort::Baud2400);
    portWrite->setDataBits(QSerialPort::Data8);
    portWrite->setParity(QSerialPort::EvenParity);
    portWrite->setStopBits(QSerialPort::OneStop);
    portWrite->setFlowControl(QSerialPort::NoFlowControl);
//    portRead->setBaudRate(QSerialPort::Baud9600);
//    portRead->setDataBits(QSerialPort::Data8);
//    portRead->setParity(QSerialPort::NoParity);
//    portRead->setStopBits(QSerialPort::OneStop);
//    portRead->setFlowControl(QSerialPort::NoFlowControl);
}

void Dialog::initData()
{
    m_data.countApparentPower=0;
    m_data.countCurrent=0;
    m_data.countEffectivePower=0;
    m_data.countPowerFactor=0;
    m_data.countReactivePower=0;
    m_data.countVoltage=0;
    m_readType=NoneType;
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_launch_clicked()
{
    //    QByteArray data=ui->writePortContent->text().toLocal8Bit();
    //    portWrite->write(data.fromHex(data));
    getVoltage();
//    getCurrent();
//    getEffectivePower();
//    getReactivePower();
//    getApparentPower();
//    getPowerFactor();
}

void Dialog::getData()
{
    switch(m_readType)
    {
    case ReadVoltage:
        m_data.voltage.append(portWrite->readAll());
        ++m_data.countVoltage;
        if(m_data.countVoltage>=BYTE_NUMBER_VOLTAGE)
        {
            QByteArray AL=m_data.voltage.mid(14,1);
            QByteArray AH=m_data.voltage.mid(15,1);
            double A=(AH.toHex().toFloat()-33)*10+(AL.toHex().toFloat()-33)*0.1;
            QByteArray BL=m_data.voltage.mid(16,1);
            QByteArray BH=m_data.voltage.mid(17,1);
            double B=(BH.toHex().toFloat()-33)*10+(BL.toHex().toFloat()-33)*0.1;
            QByteArray CL=m_data.voltage.mid(18,1);
            QByteArray CH=m_data.voltage.mid(19,1);
            double C=(CH.toHex().toFloat()-33)*10+(CL.toHex().toFloat()-33)*0.1;


            ui->readPortContent->setText(QString("%1").arg(C,0));
            m_data.countVoltage=0;
            m_data.voltage.clear();
        }
        break;
    case ReadCurrent:
        m_data.current.append(portWrite->readAll());
        ++m_data.countCurrent;
        if(m_data.countCurrent>=BYTE_NUMBER_CURRENT)
        {
            ui->readPortContent->setText(QString(m_data.current.toHex()));
            m_data.countCurrent=0;
            m_data.current.clear();
        }
        break;
    case ReadEffectivePower:
        m_data.effectivePower.append(portWrite->readAll());
        ++m_data.countEffectivePower;
        if(m_data.countEffectivePower>=BYTE_NUMBER_EFFECTIVE_POWER)
        {
            ui->readPortContent->setText(QString(m_data.effectivePower.toHex()));
            m_data.countEffectivePower=0;
            m_data.effectivePower.clear();
        }
        break;
    case ReadReactivePower:
        m_data.reactivePower.append(portWrite->readAll());
        ++m_data.countReactivePower;
        if(m_data.countReactivePower>=BYTE_NUMBER_REACTIVE_POWER)
        {
            ui->readPortContent->setText(QString(m_data.reactivePower.toHex()));
            m_data.countReactivePower=0;
            m_data.reactivePower.clear();
        }
        break;
    case ReadApparentPower:
        m_data.apprentPower.append(portWrite->readAll());
        ++m_data.countApparentPower;
        if(m_data.countApparentPower>=BYTE_NUMBER_APPRENT_POWER)
        {
            ui->readPortContent->setText(QString(m_data.apprentPower.toHex()));
            m_data.countApparentPower=0;
            m_data.apprentPower.clear();
        }
        break;
    case ReadPowerFactor:
        m_data.powerFactor.append(portWrite->readAll());
        ++m_data.countPowerFactor;
        if(m_data.countPowerFactor>=BYTE_NUMBER_POWER_FACTOR)
        {
            ui->readPortContent->setText(QString(m_data.powerFactor.toHex()));
            m_data.countPowerFactor=0;
            m_data.powerFactor.clear();
        }
        break;
    }



}

void Dialog::getVoltage()
{
    QByteArray data(QString("68aaaaaaaaaaaa68110433323435af16").toLocal8Bit());
    portWrite->write(data.fromHex(data));
    m_readType=ReadVoltage;
}

void Dialog::getCurrent()
{
    QByteArray data(QString("68aaaaaaaaaaaa68110433323535b016").toLocal8Bit());
    portWrite->write(data.fromHex(data));
    m_readType=ReadCurrent;

}

void Dialog::getEffectivePower()
{
    QByteArray data(QString("68aaaaaaaaaaaa68110433323635b116").toLocal8Bit());
    portWrite->write(data.fromHex(data));
    m_readType=ReadEffectivePower;
}

void Dialog::getReactivePower()
{
    QByteArray data(QString("68aaaaaaaaaaaa68110433323735b216").toLocal8Bit());
    portWrite->write(data.fromHex(data));
    m_readType=ReadReactivePower;
}

void Dialog::getApparentPower()
{
    QByteArray data(QString("68aaaaaaaaaaaa68110433323835b316").toLocal8Bit());
    portWrite->write(data.fromHex(data));
    m_readType=ReadApparentPower;
}

void Dialog::getPowerFactor()
{
    QByteArray data(QString("68aaaaaaaaaaaa68110433323935b416").toLocal8Bit());
    portWrite->write(data.fromHex(data));
    m_readType=ReadPowerFactor;
}
