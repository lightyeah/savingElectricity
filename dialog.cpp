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
    initPort();
    initData();
    initConnections();
}

void Dialog::initPort()
{
    portWrite=new QSerialPort(this);
    portRead=new QSerialPort(this);
    portWrite->setPortName(ui->writePortNumber->text());
    portRead->setPortName(ui->readPortNumber->text());
    portWrite->setBaudRate(QSerialPort::Baud2400);
    portWrite->setDataBits(QSerialPort::Data8);
    portWrite->setParity(QSerialPort::EvenParity);
    portWrite->setStopBits(QSerialPort::OneStop);
    portWrite->setFlowControl(QSerialPort::NoFlowControl);
    portWrite->open(QIODevice::ReadWrite);
    connect(portWrite,&QSerialPort::readyRead,this,&Dialog::getData);

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

void Dialog::initDataToPlot()
{

}

void Dialog::initConnections()
{
    connect(this,&Dialog::voltageDataGot,this,&Dialog::plotVoltage);
    connect(this,&Dialog::currentDataGot,this,&Dialog::plotCurrent);
    connect(this,&Dialog::effectivePowerDataGot,this,&Dialog::plotEffectivePower);
    connect(this,&Dialog::reactivePowerDataGot,this,&Dialog::plotReactivePower);
    connect(this,&Dialog::apparentPowerDataGot,this,&Dialog::plotApparentPower);
    connect(this,&Dialog::powerFactorDataGot,this,&Dialog::plotPowerFactor);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_launch_clicked()
{
    //    QByteArray data=ui->writePortContent->text().toLocal8Bit();
    //    portWrite->write(data.fromHex(data));
    //    getVoltage();
    //    getCurrent();
    //    getEffectivePower();
    //    getReactivePower();
    //    getApparentPower();
    getPowerFactor();
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
            float A=(AH.toHex().toFloat()-33)*10+(AL.toHex().toFloat()-33)*0.1;
            QByteArray BL=m_data.voltage.mid(16,1);
            QByteArray BH=m_data.voltage.mid(17,1);
            float B=(BH.toHex().toFloat()-33)*10+(BL.toHex().toFloat()-33)*0.1;
            QByteArray CL=m_data.voltage.mid(18,1);
            QByteArray CH=m_data.voltage.mid(19,1);
            float C=(CH.toHex().toFloat()-33)*10+(CL.toHex().toFloat()-33)*0.1;
            emit voltageDataGot(A,B,C);
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
            QByteArray AL=m_data.current.mid(14,1);
            QByteArray AM=m_data.current.mid(15,1);
            QByteArray AH=m_data.current.mid(16,1);
            double A=(AH.toHex().toFloat()-33)*10+(AM.toHex().toFloat()-33)*0.1+(AL.toHex().toFloat()-33)*0.001;
            QByteArray BL=m_data.current.mid(17,1);
            QByteArray BM=m_data.current.mid(18,1);
            QByteArray BH=m_data.current.mid(19,1);
            double B=(BH.toHex().toFloat()-33)*10+(BM.toHex().toFloat()-33)*0.1+(BL.toHex().toFloat()-33)*0.001;
            QByteArray CL=m_data.current.mid(20,1);
            QByteArray CM=m_data.current.mid(21,1);
            QByteArray CH=m_data.current.mid(22,1);
            double C=(CH.toHex().toFloat()-33)*10+(CM.toHex().toFloat()-33)*0.1+(CL.toHex().toFloat()-33)*0.001;
            ui->readPortContent->setText(QString("%1").arg(A,0));
            m_data.countCurrent=0;
            m_data.current.clear();
        }
        break;
    case ReadEffectivePower:
        m_data.effectivePower.append(portWrite->readAll());
        ++m_data.countEffectivePower;
        if(m_data.countEffectivePower>=BYTE_NUMBER_EFFECTIVE_POWER)
        {
            QByteArray SL=m_data.effectivePower.mid(14,1);
            QByteArray SM=m_data.effectivePower.mid(15,1);
            QByteArray SH=m_data.effectivePower.mid(16,1);
            double S=(SH.toHex().toFloat()-33)+(SM.toHex().toFloat()-33)*0.01+(SL.toHex().toFloat()-33)*0.0001;
            QByteArray AL=m_data.effectivePower.mid(17,1);
            QByteArray AM=m_data.effectivePower.mid(18,1);
            QByteArray AH=m_data.effectivePower.mid(19,1);
            double A=(AH.toHex().toFloat()-33)+(AM.toHex().toFloat()-33)*0.01+(AL.toHex().toFloat()-33)*0.0001;
            QByteArray BL=m_data.effectivePower.mid(20,1);
            QByteArray BM=m_data.effectivePower.mid(21,1);
            QByteArray BH=m_data.effectivePower.mid(22,1);
            double B=(BH.toHex().toFloat()-33)+(BM.toHex().toFloat()-33)*0.01+(BL.toHex().toFloat()-33)*0.0001;
            QByteArray CL=m_data.effectivePower.mid(23,1);
            QByteArray CM=m_data.effectivePower.mid(24,1);
            QByteArray CH=m_data.effectivePower.mid(25,1);
            double C=(CH.toHex().toFloat()-33)+(CM.toHex().toFloat()-33)*0.01+(CL.toHex().toFloat()-33)*0.0001;
            ui->readPortContent->setText(QString("%1").arg(A,0));
            m_data.countEffectivePower=0;
            m_data.effectivePower.clear();
        }
        break;
    case ReadReactivePower:
        m_data.reactivePower.append(portWrite->readAll());
        ++m_data.countReactivePower;
        if(m_data.countReactivePower>=BYTE_NUMBER_REACTIVE_POWER)
        {
            QByteArray SL=m_data.reactivePower.mid(14,1);
            QByteArray SM=m_data.reactivePower.mid(15,1);
            QByteArray SH=m_data.reactivePower.mid(16,1);
            double S=(SH.toHex().toFloat()-33)+(SM.toHex().toFloat()-33)*0.01+(SL.toHex().toFloat()-33)*0.0001;
            QByteArray AL=m_data.reactivePower.mid(17,1);
            QByteArray AM=m_data.reactivePower.mid(18,1);
            QByteArray AH=m_data.reactivePower.mid(19,1);
            double A=(AH.toHex().toFloat()-33)+(AM.toHex().toFloat()-33)*0.01+(AL.toHex().toFloat()-33)*0.0001;
            QByteArray BL=m_data.reactivePower.mid(20,1);
            QByteArray BM=m_data.reactivePower.mid(21,1);
            QByteArray BH=m_data.reactivePower.mid(22,1);
            double B=(BH.toHex().toFloat()-33)+(BM.toHex().toFloat()-33)*0.01+(BL.toHex().toFloat()-33)*0.0001;
            QByteArray CL=m_data.reactivePower.mid(23,1);
            QByteArray CM=m_data.reactivePower.mid(24,1);
            QByteArray CH=m_data.reactivePower.mid(25,1);
            double C=(CH.toHex().toFloat()-33)+(CM.toHex().toFloat()-33)*0.01+(CL.toHex().toFloat()-33)*0.0001;
            ui->readPortContent->setText(QString("%1").arg(A,0));
            m_data.countReactivePower=0;
            m_data.reactivePower.clear();
        }
        break;
    case ReadApparentPower:
        m_data.apprentPower.append(portWrite->readAll());
        ++m_data.countApparentPower;
        if(m_data.countApparentPower>=BYTE_NUMBER_APPRENT_POWER)
        {
            QByteArray SL=m_data.apprentPower.mid(14,1);
            QByteArray SM=m_data.apprentPower.mid(15,1);
            QByteArray SH=m_data.apprentPower.mid(16,1);
            double S=(SH.toHex().toFloat()-33)+(SM.toHex().toFloat()-33)*0.01+(SL.toHex().toFloat()-33)*0.0001;
            QByteArray AL=m_data.apprentPower.mid(17,1);
            QByteArray AM=m_data.apprentPower.mid(18,1);
            QByteArray AH=m_data.apprentPower.mid(19,1);
            double A=(AH.toHex().toFloat()-33)+(AM.toHex().toFloat()-33)*0.01+(AL.toHex().toFloat()-33)*0.0001;
            QByteArray BL=m_data.apprentPower.mid(20,1);
            QByteArray BM=m_data.apprentPower.mid(21,1);
            QByteArray BH=m_data.apprentPower.mid(22,1);
            double B=(BH.toHex().toFloat()-33)+(BM.toHex().toFloat()-33)*0.01+(BL.toHex().toFloat()-33)*0.0001;
            QByteArray CL=m_data.apprentPower.mid(23,1);
            QByteArray CM=m_data.apprentPower.mid(24,1);
            QByteArray CH=m_data.apprentPower.mid(25,1);
            double C=(CH.toHex().toFloat()-33)+(CM.toHex().toFloat()-33)*0.01+(CL.toHex().toFloat()-33)*0.0001;
            ui->readPortContent->setText(QString("%1").arg(A,0));
            m_data.countApparentPower=0;
            m_data.apprentPower.clear();
        }
        break;
    case ReadPowerFactor:
        m_data.powerFactor.append(portWrite->readAll());
        ++m_data.countPowerFactor;
        if(m_data.countPowerFactor>=BYTE_NUMBER_POWER_FACTOR)
        {
            QByteArray SL=m_data.powerFactor.mid(14,1);
            QByteArray SH=m_data.powerFactor.mid(15,1);
            double S=(SH.toHex().toFloat()-33)*0.1+(SL.toHex().toFloat()-33)*0.001;
            QByteArray AL=m_data.powerFactor.mid(16,1);
            QByteArray AH=m_data.powerFactor.mid(17,1);
            double A=(AH.toHex().toFloat()-33)*0.1+(AL.toHex().toFloat()-33)*0.001;
            QByteArray BL=m_data.powerFactor.mid(18,1);
            QByteArray BH=m_data.powerFactor.mid(19,1);
            double B=(BH.toHex().toFloat()-33)*0.1+(BL.toHex().toFloat()-33)*0.001;
            QByteArray CL=m_data.powerFactor.mid(20,1);
            QByteArray CH=m_data.powerFactor.mid(21,1);
            double C=(CH.toHex().toFloat()-33)*0.1+(CL.toHex().toFloat()-33)*0.001;
            ui->readPortContent->setText(QString("%1").arg(A,0));
            m_data.countPowerFactor=0;
            m_data.powerFactor.clear();
        }
        break;
    }



}

void Dialog::plotVoltage(float A, float B, float C)
{

}

void Dialog::plotCurrent(float A, float B, float C)
{

}

void Dialog::plotEffectivePower(float A, float B, float C, float S)
{

}

void Dialog::plotReactivePower(float A, float B, float C, float S)
{

}

void Dialog::plotApparentPower(float A, float B, float C, float S)
{

}

void Dialog::plotPowerFactor(float A, float B, float C, float S)
{

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
