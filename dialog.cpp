#include "dialog.h"
#include "ui_dialog.h"
#include <QString>
#include <QTimer>
#define BYTE_NUMBER_VOLTAGE 22
#define BYTE_NUMBER_CURRENT 25
#define BYTE_NUMBER_EFFECTIVE_POWER 28
#define BYTE_NUMBER_REACTIVE_POWER 28
#define BYTE_NUMBER_APPRENT_POWER 28
#define BYTE_NUMBER_POWER_FACTOR 24
#define DATA_VECTOR_LENGTH 50

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    QTimer * timer=new QTimer(this);
    initPort();
    initData();
    initPlotStyle();
    //    initConnections();
    connect(timer,&QTimer::timeout,this,&Dialog::getAndPlotData);
    timer->start(1000);
}

void Dialog::initPort()
{
    portWrite=new QSerialPort(this);
    portRead=new QSerialPort(this);
    portWrite->setPortName(ui->writePortNumber->text());
    //    portRead->setPortName(ui->readPortNumber->text());
    portWrite->setBaudRate(QSerialPort::Baud2400);
    portWrite->setDataBits(QSerialPort::Data8);
    portWrite->setParity(QSerialPort::EvenParity);
    portWrite->setStopBits(QSerialPort::OneStop);
    portWrite->setFlowControl(QSerialPort::NoFlowControl);
    portWrite->open(QIODevice::ReadWrite);
    connect(portWrite,&QSerialPort::readyRead,this,&Dialog::parseData);

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

void Dialog::initPlotStyle()
{
    //m_voltage.A.resize(50);
    //m_voltage.B.resize(50);
    //m_voltage.C.resize(50);
    //m_current.A.resize(50);
    //m_current.B.resize(50);
    //m_current.C.resize(50);
    //m_effectivePower.A.resize(50);
    //m_effectivePower.B.resize(50);
    //m_effectivePower.C.resize(50);
    //m_effectivePower.S.resize(50);
    //m_reactivePower.A.resize(50);
    //m_reactivePower.B.resize(50);
    //m_reactivePower.C.resize(50);
    //m_reactivePower.S.resize(50);
    //m_apparentPower.A.resize(50);
    //m_apparentPower.B.resize(50);
    //m_apparentPower.C.resize(50);
    //m_apparentPower.S.resize(50);
    //m_powerFactor.A.resize(50);
    //m_powerFactor.B.resize(50);
    //m_powerFactor.C.resize(50);
    //m_powerFactor.S.resize(50);
    ui->PlotA->addGraph();
    ui->PlotA->graph(0)->setPen(QPen(Qt::blue));
    ui->PlotA->xAxis->setRange(0,51);
    ui->PlotA->yAxis->setRange(200,240);
    ui->PlotB->addGraph();
    ui->PlotB->graph(0)->setPen(QPen(Qt::green));
    ui->PlotB->xAxis->setRange(0,51);
    ui->PlotB->yAxis->setRange(-1,270);
    ui->PlotC->addGraph();
    ui->PlotC->graph(0)->setPen(QPen(Qt::red));
    ui->PlotC->xAxis->setRange(0,51);
    ui->PlotC->yAxis->setRange(-1,270);

}

void Dialog::initConnections()
{
    //    connect(this,&Dialog::voltageDataGot,this,&Dialog::plotVoltage);
    //    connect(this,&Dialog::currentDataGot,this,&Dialog::plotCurrent);
    //    connect(this,&Dialog::effectivePowerDataGot,this,&Dialog::plotEffectivePower);
    //    connect(this,&Dialog::reactivePowerDataGot,this,&Dialog::plotReactivePower);
    //    connect(this,&Dialog::apparentPowerDataGot,this,&Dialog::plotApparentPower);
    //    connect(this,&Dialog::powerFactorDataGot,this,&Dialog::plotPowerFactor);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_launch_clicked()
{

    getVoltage();
}

void Dialog::parseData()
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
            datatype A=(AH.toHex().toFloat()-33)*10+(AL.toHex().toFloat()-33)*0.1;
            QByteArray BL=m_data.voltage.mid(16,1);
            QByteArray BH=m_data.voltage.mid(17,1);
            datatype B=(BH.toHex().toFloat()-33)*10+(BL.toHex().toFloat()-33)*0.1;
            QByteArray CL=m_data.voltage.mid(18,1);
            QByteArray CH=m_data.voltage.mid(19,1);
            datatype C=(CH.toHex().toFloat()-33)*10+(CL.toHex().toFloat()-33)*0.1;
            //            emit voltageDataGot(A,B,C);
            ui->readPortContent->setText(QString("%1").arg(A,0));
            insertData(A,m_voltage.A,m_voltage.keys);
            insertData(B,m_voltage.B,m_voltage.keys);
            insertData(C,m_voltage.C,m_voltage.keys);
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
            datatype A=(AH.toHex().toFloat()-33)*10+(AM.toHex().toFloat()-33)*0.1+(AL.toHex().toFloat()-33)*0.001;
            QByteArray BL=m_data.current.mid(17,1);
            QByteArray BM=m_data.current.mid(18,1);
            QByteArray BH=m_data.current.mid(19,1);
            datatype B=(BH.toHex().toFloat()-33)*10+(BM.toHex().toFloat()-33)*0.1+(BL.toHex().toFloat()-33)*0.001;
            QByteArray CL=m_data.current.mid(20,1);
            QByteArray CM=m_data.current.mid(21,1);
            QByteArray CH=m_data.current.mid(22,1);
            datatype C=(CH.toHex().toFloat()-33)*10+(CM.toHex().toFloat()-33)*0.1+(CL.toHex().toFloat()-33)*0.001;
            //            emit currentDataGot(A,B,C);
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
            datatype S=(SH.toHex().toFloat()-33)+(SM.toHex().toFloat()-33)*0.01+(SL.toHex().toFloat()-33)*0.0001;
            QByteArray AL=m_data.effectivePower.mid(17,1);
            QByteArray AM=m_data.effectivePower.mid(18,1);
            QByteArray AH=m_data.effectivePower.mid(19,1);
            datatype A=(AH.toHex().toFloat()-33)+(AM.toHex().toFloat()-33)*0.01+(AL.toHex().toFloat()-33)*0.0001;
            QByteArray BL=m_data.effectivePower.mid(20,1);
            QByteArray BM=m_data.effectivePower.mid(21,1);
            QByteArray BH=m_data.effectivePower.mid(22,1);
            datatype B=(BH.toHex().toFloat()-33)+(BM.toHex().toFloat()-33)*0.01+(BL.toHex().toFloat()-33)*0.0001;
            QByteArray CL=m_data.effectivePower.mid(23,1);
            QByteArray CM=m_data.effectivePower.mid(24,1);
            QByteArray CH=m_data.effectivePower.mid(25,1);
            datatype C=(CH.toHex().toFloat()-33)+(CM.toHex().toFloat()-33)*0.01+(CL.toHex().toFloat()-33)*0.0001;
            //            emit effectivePowerDataGot(A,B,C,S);
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
            datatype S=(SH.toHex().toFloat()-33)+(SM.toHex().toFloat()-33)*0.01+(SL.toHex().toFloat()-33)*0.0001;
            QByteArray AL=m_data.reactivePower.mid(17,1);
            QByteArray AM=m_data.reactivePower.mid(18,1);
            QByteArray AH=m_data.reactivePower.mid(19,1);
            datatype A=(AH.toHex().toFloat()-33)+(AM.toHex().toFloat()-33)*0.01+(AL.toHex().toFloat()-33)*0.0001;
            QByteArray BL=m_data.reactivePower.mid(20,1);
            QByteArray BM=m_data.reactivePower.mid(21,1);
            QByteArray BH=m_data.reactivePower.mid(22,1);
            datatype B=(BH.toHex().toFloat()-33)+(BM.toHex().toFloat()-33)*0.01+(BL.toHex().toFloat()-33)*0.0001;
            QByteArray CL=m_data.reactivePower.mid(23,1);
            QByteArray CM=m_data.reactivePower.mid(24,1);
            QByteArray CH=m_data.reactivePower.mid(25,1);
            datatype C=(CH.toHex().toFloat()-33)+(CM.toHex().toFloat()-33)*0.01+(CL.toHex().toFloat()-33)*0.0001;
            //            emit reactivePowerDataGot(A,B,C,S);
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
            datatype S=(SH.toHex().toFloat()-33)+(SM.toHex().toFloat()-33)*0.01+(SL.toHex().toFloat()-33)*0.0001;
            QByteArray AL=m_data.apprentPower.mid(17,1);
            QByteArray AM=m_data.apprentPower.mid(18,1);
            QByteArray AH=m_data.apprentPower.mid(19,1);
            datatype A=(AH.toHex().toFloat()-33)+(AM.toHex().toFloat()-33)*0.01+(AL.toHex().toFloat()-33)*0.0001;
            QByteArray BL=m_data.apprentPower.mid(20,1);
            QByteArray BM=m_data.apprentPower.mid(21,1);
            QByteArray BH=m_data.apprentPower.mid(22,1);
            datatype B=(BH.toHex().toFloat()-33)+(BM.toHex().toFloat()-33)*0.01+(BL.toHex().toFloat()-33)*0.0001;
            QByteArray CL=m_data.apprentPower.mid(23,1);
            QByteArray CM=m_data.apprentPower.mid(24,1);
            QByteArray CH=m_data.apprentPower.mid(25,1);
            datatype C=(CH.toHex().toFloat()-33)+(CM.toHex().toFloat()-33)*0.01+(CL.toHex().toFloat()-33)*0.0001;
            //            emit apparentPowerDataGot(A,B,C,S);
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
            datatype S=(SH.toHex().toFloat()-33)*0.1+(SL.toHex().toFloat()-33)*0.001;
            QByteArray AL=m_data.powerFactor.mid(16,1);
            QByteArray AH=m_data.powerFactor.mid(17,1);
            datatype A=(AH.toHex().toFloat()-33)*0.1+(AL.toHex().toFloat()-33)*0.001;
            QByteArray BL=m_data.powerFactor.mid(18,1);
            QByteArray BH=m_data.powerFactor.mid(19,1);
            datatype B=(BH.toHex().toFloat()-33)*0.1+(BL.toHex().toFloat()-33)*0.001;
            QByteArray CL=m_data.powerFactor.mid(20,1);
            QByteArray CH=m_data.powerFactor.mid(21,1);
            datatype C=(CH.toHex().toFloat()-33)*0.1+(CL.toHex().toFloat()-33)*0.001;
            //            emit powerFactorDataGot(A,B,C,S);
            ui->readPortContent->setText(QString("%1").arg(A,0));
            m_data.countPowerFactor=0;
            m_data.powerFactor.clear();
        }
        break;
    }



}

void Dialog::getAndPlotData()
{
    getVoltage();
//    getCurrent();
//    getEffectivePower();
//    getReactivePower();
//    getApparentPower();
//    getPowerFactor();
//    if(ui->startPlot->isEnabled())
//    {
        ui->PlotA->graph()->addData(m_voltage.keys,m_voltage.A);
        ui->PlotB->graph()->addData(m_voltage.keys,m_voltage.B);
        ui->PlotC->graph()->addData(m_voltage.keys,m_voltage.C);
        ui->PlotA->replot();
        ui->PlotB->replot();
        ui->PlotC->replot();
//    }
}

//void Dialog::plotVoltage(float A, float B, float C)
//{

//}

//void Dialog::plotCurrent(float A, float B, float C)
//{

//}

//void Dialog::plotEffectivePower(float A, float B, float C, float S)
//{

//}

//void Dialog::plotReactivePower(float A, float B, float C, float S)
//{

//}

//void Dialog::plotApparentPower(float A, float B, float C, float S)
//{

//}

//void Dialog::plotPowerFactor(float A, float B, float C, float S)
//{

//}

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

void Dialog::insertData(datatype data, QVector<datatype> &dataVector, QVector<datatype> &keys)
{
    if(dataVector.size()<DATA_VECTOR_LENGTH)
    {
        dataVector.append(data);
        if(keys.size()<dataVector.size())
            keys.append(dataVector.size());
    }
    else
    {
        dataVector.pop_front();
        dataVector.push_back(data);
    }
}

void Dialog::on_stopPlot_clicked()
{
    ui->stopPlot->setEnabled(false);
    ui->startPlot->setEnabled(true);
}

void Dialog::on_startPlot_clicked()
{
    ui->stopPlot->setEnabled(true);
    ui->startPlot->setEnabled(false);
}
