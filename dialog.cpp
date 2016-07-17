#include "dialog.h"
#include "ui_dialog.h"
#include <QString>
#include <QTimer>
#include <QThread>
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
    timer=new QTimer(this);
    connect(timer,&QTimer::timeout,this,&Dialog::getVoltage);
    initPort();
    initData();
    initConnections();
    initPlotStyle();
    timer->start();
}

void Dialog::initPort()
{
    portWrite=new QSerialPort(this);
    foreach (const QSerialPortInfo &info,QSerialPortInfo::availablePorts())
    {
        QString portDescription=info.description();
        if(portDescription.contains(QString("CH341A")))
        {
            portWrite->setPort(info);
            ui->writePortNumber->setText(info.portName());
        }

    }

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
    m_data.count=0;
    m_readType=NoneType;
}

void Dialog::initPlotStyle()
{
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
    connect(this,&Dialog::voltageDataGot,this,&Dialog::getCurrent);
    connect(this,&Dialog::currentDataGot,this,&Dialog::getEffectivePower);
    connect(this,&Dialog::effectivePowerDataGot,this,&Dialog::getReactivePower);
    connect(this,&Dialog::reactivePowerDataGot,this,&Dialog::getApparentPower);
    connect(this,&Dialog::apparentPowerDataGot,this,&Dialog::getPowerFactor);
    connect(this,&Dialog::powerFactorDataGot,this,&Dialog::whichToPlot);
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
        m_data.buffer.append(portWrite->readAll());
        ++m_data.count;
        if(m_data.count>=BYTE_NUMBER_VOLTAGE)
        {
            QByteArray AL=m_data.buffer.mid(14,1);
            QByteArray AH=m_data.buffer.mid(15,1);
            datatype A=(AH.toHex().toFloat()-33)*10+(AL.toHex().toFloat()-33)*0.1;
            QByteArray BL=m_data.buffer.mid(16,1);
            QByteArray BH=m_data.buffer.mid(17,1);
            datatype B=(BH.toHex().toFloat()-33)*10+(BL.toHex().toFloat()-33)*0.1;
            QByteArray CL=m_data.buffer.mid(18,1);
            QByteArray CH=m_data.buffer.mid(19,1);
            datatype C=(CH.toHex().toFloat()-33)*10+(CL.toHex().toFloat()-33)*0.1;
            ui->readPortContent->setText(QString("%1").arg(A,0));
            qDebug()<<"get voltage data A "<<A<<endl;
            insertData(A,m_voltage.A,m_voltage.keys);
            insertData(B,m_voltage.B,m_voltage.keys);
            insertData(C,m_voltage.C,m_voltage.keys);
            m_data.count=0;
            m_data.buffer.clear();
            emit voltageDataGot();
        }
        break;
    case ReadCurrent:
        m_data.buffer.append(portWrite->readAll());
        ++m_data.count;
        if(m_data.count>=BYTE_NUMBER_CURRENT)
        {
            QByteArray AL=m_data.buffer.mid(14,1);
            QByteArray AM=m_data.buffer.mid(15,1);
            QByteArray AH=m_data.buffer.mid(16,1);
            datatype A=(AH.toHex().toFloat()-33)*10+(AM.toHex().toFloat()-33)*0.1+(AL.toHex().toFloat()-33)*0.001;
            QByteArray BL=m_data.buffer.mid(17,1);
            QByteArray BM=m_data.buffer.mid(18,1);
            QByteArray BH=m_data.buffer.mid(19,1);
            datatype B=(BH.toHex().toFloat()-33)*10+(BM.toHex().toFloat()-33)*0.1+(BL.toHex().toFloat()-33)*0.001;
            QByteArray CL=m_data.buffer.mid(20,1);
            QByteArray CM=m_data.buffer.mid(21,1);
            QByteArray CH=m_data.buffer.mid(22,1);
            datatype C=(CH.toHex().toFloat()-33)*10+(CM.toHex().toFloat()-33)*0.1+(CL.toHex().toFloat()-33)*0.001;
            ui->readPortContent->setText(QString("%1").arg(A,0));
            qDebug()<<"get current data A "<<A<<endl;
            insertData(A,m_current.A,m_current.keys);
            insertData(B,m_current.B,m_current.keys);
            insertData(C,m_current.C,m_current.keys);
            m_data.count=0;
            m_data.buffer.clear();
            emit currentDataGot();
        }
        break;
    case ReadEffectivePower:
        m_data.buffer.append(portWrite->readAll());
        ++m_data.count;
        if(m_data.count>=BYTE_NUMBER_EFFECTIVE_POWER)
        {
            QByteArray SL=m_data.buffer.mid(14,1);
            QByteArray SM=m_data.buffer.mid(15,1);
            QByteArray SH=m_data.buffer.mid(16,1);
            datatype S=(SH.toHex().toFloat()-33)+(SM.toHex().toFloat()-33)*0.01+(SL.toHex().toFloat()-33)*0.0001;
            QByteArray AL=m_data.buffer.mid(17,1);
            QByteArray AM=m_data.buffer.mid(18,1);
            QByteArray AH=m_data.buffer.mid(19,1);
            datatype A=(AH.toHex().toFloat()-33)+(AM.toHex().toFloat()-33)*0.01+(AL.toHex().toFloat()-33)*0.0001;
            QByteArray BL=m_data.buffer.mid(20,1);
            QByteArray BM=m_data.buffer.mid(21,1);
            QByteArray BH=m_data.buffer.mid(22,1);
            datatype B=(BH.toHex().toFloat()-33)+(BM.toHex().toFloat()-33)*0.01+(BL.toHex().toFloat()-33)*0.0001;
            QByteArray CL=m_data.buffer.mid(23,1);
            QByteArray CM=m_data.buffer.mid(24,1);
            QByteArray CH=m_data.buffer.mid(25,1);
            datatype C=(CH.toHex().toFloat()-33)+(CM.toHex().toFloat()-33)*0.01+(CL.toHex().toFloat()-33)*0.0001;
            ui->readPortContent->setText(QString("%1").arg(A,0));
            qDebug()<<"get effectivePower data A "<<A<<endl;
            insertData(A,m_effectivePower.A,m_effectivePower.keys);
            insertData(B,m_effectivePower.B,m_effectivePower.keys);
            insertData(C,m_effectivePower.C,m_effectivePower.keys);
            insertData(S,m_effectivePower.S,m_effectivePower.keys);
            m_data.count=0;
            m_data.buffer.clear();
            emit effectivePowerDataGot();
        }
        break;
    case ReadReactivePower:
        m_data.buffer.append(portWrite->readAll());
        ++m_data.count;
        if(m_data.count>=BYTE_NUMBER_REACTIVE_POWER)
        {
            QByteArray SL=m_data.buffer.mid(14,1);
            QByteArray SM=m_data.buffer.mid(15,1);
            QByteArray SH=m_data.buffer.mid(16,1);
            datatype S=(SH.toHex().toFloat()-33)+(SM.toHex().toFloat()-33)*0.01+(SL.toHex().toFloat()-33)*0.0001;
            QByteArray AL=m_data.buffer.mid(17,1);
            QByteArray AM=m_data.buffer.mid(18,1);
            QByteArray AH=m_data.buffer.mid(19,1);
            datatype A=(AH.toHex().toFloat()-33)+(AM.toHex().toFloat()-33)*0.01+(AL.toHex().toFloat()-33)*0.0001;
            QByteArray BL=m_data.buffer.mid(20,1);
            QByteArray BM=m_data.buffer.mid(21,1);
            QByteArray BH=m_data.buffer.mid(22,1);
            datatype B=(BH.toHex().toFloat()-33)+(BM.toHex().toFloat()-33)*0.01+(BL.toHex().toFloat()-33)*0.0001;
            QByteArray CL=m_data.buffer.mid(23,1);
            QByteArray CM=m_data.buffer.mid(24,1);
            QByteArray CH=m_data.buffer.mid(25,1);
            datatype C=(CH.toHex().toFloat()-33)+(CM.toHex().toFloat()-33)*0.01+(CL.toHex().toFloat()-33)*0.0001;
            ui->readPortContent->setText(QString("%1").arg(A,0));
            qDebug()<<"get reactivePower data A "<<A<<endl;
            insertData(A,m_reactivePower.A,m_reactivePower.keys);
            insertData(B,m_reactivePower.B,m_reactivePower.keys);
            insertData(C,m_reactivePower.C,m_reactivePower.keys);
            insertData(S,m_reactivePower.S,m_reactivePower.keys);
            m_data.count=0;
            m_data.buffer.clear();
            emit reactivePowerDataGot();
        }
        break;
    case ReadApparentPower:
        m_data.buffer.append(portWrite->readAll());
        ++m_data.count;
        if(m_data.count>=BYTE_NUMBER_APPRENT_POWER)
        {
            QByteArray SL=m_data.buffer.mid(14,1);
            QByteArray SM=m_data.buffer.mid(15,1);
            QByteArray SH=m_data.buffer.mid(16,1);
            datatype S=(SH.toHex().toFloat()-33)+(SM.toHex().toFloat()-33)*0.01+(SL.toHex().toFloat()-33)*0.0001;
            QByteArray AL=m_data.buffer.mid(17,1);
            QByteArray AM=m_data.buffer.mid(18,1);
            QByteArray AH=m_data.buffer.mid(19,1);
            datatype A=(AH.toHex().toFloat()-33)+(AM.toHex().toFloat()-33)*0.01+(AL.toHex().toFloat()-33)*0.0001;
            QByteArray BL=m_data.buffer.mid(20,1);
            QByteArray BM=m_data.buffer.mid(21,1);
            QByteArray BH=m_data.buffer.mid(22,1);
            datatype B=(BH.toHex().toFloat()-33)+(BM.toHex().toFloat()-33)*0.01+(BL.toHex().toFloat()-33)*0.0001;
            QByteArray CL=m_data.buffer.mid(23,1);
            QByteArray CM=m_data.buffer.mid(24,1);
            QByteArray CH=m_data.buffer.mid(25,1);
            datatype C=(CH.toHex().toFloat()-33)+(CM.toHex().toFloat()-33)*0.01+(CL.toHex().toFloat()-33)*0.0001;
            ui->readPortContent->setText(QString("%1").arg(A,0));
            qDebug()<<"get apparentPower data A "<<A<<endl;
            insertData(A,m_apparentPower.A,m_apparentPower.keys);
            insertData(B,m_apparentPower.B,m_apparentPower.keys);
            insertData(C,m_apparentPower.C,m_apparentPower.keys);
            insertData(S,m_apparentPower.S,m_apparentPower.keys);
            m_data.count=0;
            m_data.buffer.clear();
            emit apparentPowerDataGot();
        }
        break;
    case ReadPowerFactor:
        m_data.buffer.append(portWrite->readAll());
        ++m_data.count;
        if(m_data.count>=BYTE_NUMBER_POWER_FACTOR)
        {
            QByteArray SL=m_data.buffer.mid(14,1);
            QByteArray SH=m_data.buffer.mid(15,1);
            datatype S=(SH.toHex().toFloat()-33)*0.1+(SL.toHex().toFloat()-33)*0.001;
            QByteArray AL=m_data.buffer.mid(16,1);
            QByteArray AH=m_data.buffer.mid(17,1);
            datatype A=(AH.toHex().toFloat()-33)*0.1+(AL.toHex().toFloat()-33)*0.001;
            QByteArray BL=m_data.buffer.mid(18,1);
            QByteArray BH=m_data.buffer.mid(19,1);
            datatype B=(BH.toHex().toFloat()-33)*0.1+(BL.toHex().toFloat()-33)*0.001;
            QByteArray CL=m_data.buffer.mid(20,1);
            QByteArray CH=m_data.buffer.mid(21,1);
            datatype C=(CH.toHex().toFloat()-33)*0.1+(CL.toHex().toFloat()-33)*0.001;
            ui->readPortContent->setText(QString("%1").arg(A,0));
            qDebug()<<"get powerFactor data A "<<A<<endl;
            insertData(A,m_powerFactor.A,m_powerFactor.keys);
            insertData(B,m_powerFactor.B,m_powerFactor.keys);
            insertData(C,m_powerFactor.C,m_powerFactor.keys);
            insertData(S,m_powerFactor.S,m_powerFactor.keys);
            m_data.count=0;
            m_data.buffer.clear();
            emit powerFactorDataGot();
        }
        break;
    default:
        break;
    }
}


void Dialog::getVoltage()
{
    timer->stop();
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

void Dialog::whichToPlot()
{
    if(!(ui->startPlot->isEnabled()))
    {
        if (ui->radioButtonVoltage->isChecked())
        {
            clearPlotData();
            ui->PlotA->graph()->addData(m_voltage.keys,m_voltage.A);
            ui->PlotB->graph()->addData(m_voltage.keys,m_voltage.B);
            ui->PlotC->graph()->addData(m_voltage.keys,m_voltage.C);
            ui->PlotA->replot();
            ui->PlotB->replot();
            ui->PlotC->replot();
            qDebug()<<"voltage"<<endl;
        }
        else if(ui->radioButtonCurrent->isChecked())
        {
            clearPlotData();
            ui->PlotA->graph()->addData(m_current.keys,m_current.A);
            ui->PlotB->graph()->addData(m_current.keys,m_current.B);
            ui->PlotC->graph()->addData(m_current.keys,m_current.C);
            ui->PlotA->replot();
            ui->PlotB->replot();
            ui->PlotC->replot();
            qDebug()<<"current"<<endl;
        }
        else if(ui->radioButtonEffective->isChecked())
        {
            clearPlotData();
            ui->PlotA->graph()->addData(m_effectivePower.keys,m_effectivePower.A);
            ui->PlotB->graph()->addData(m_effectivePower.keys,m_effectivePower.B);
            ui->PlotC->graph()->addData(m_effectivePower.keys,m_effectivePower.C);
            ui->PlotA->replot();
            ui->PlotB->replot();
            ui->PlotC->replot();
            qDebug()<<"effectivePower"<<endl;
        }
        else if(ui->radioButtonReactive->isChecked())
        {
            clearPlotData();
            ui->PlotA->graph()->addData(m_reactivePower.keys,m_reactivePower.A);
            ui->PlotB->graph()->addData(m_reactivePower.keys,m_reactivePower.B);
            ui->PlotC->graph()->addData(m_reactivePower.keys,m_reactivePower.C);
            ui->PlotA->replot();
            ui->PlotB->replot();
            ui->PlotC->replot();
            qDebug()<<"reactivePower"<<endl;
        }
        else if(ui->radioButtonApparent->isChecked())
        {
            clearPlotData();
            ui->PlotA->graph()->addData(m_apparentPower.keys,m_apparentPower.A);
            ui->PlotB->graph()->addData(m_apparentPower.keys,m_apparentPower.B);
            ui->PlotC->graph()->addData(m_apparentPower.keys,m_apparentPower.C);
            ui->PlotA->replot();
            ui->PlotB->replot();
            ui->PlotC->replot();
            qDebug()<<"apparentPower"<<endl;
        }
        else if(ui->radioButtonPowerFactor->isChecked())
        {
            clearPlotData();
            ui->PlotA->graph()->addData(m_powerFactor.keys,m_powerFactor.A);
            ui->PlotB->graph()->addData(m_powerFactor.keys,m_powerFactor.B);
            ui->PlotC->graph()->addData(m_powerFactor.keys,m_powerFactor.C);
            ui->PlotA->replot();
            ui->PlotB->replot();
            ui->PlotC->replot();

            qDebug()<<"powerFactor"<<endl;
        }
    }
    timer->start();
}

void Dialog::clearPlotData()
{
    ui->PlotA->graph()->clearData();
    ui->PlotB->graph()->clearData();
    ui->PlotC->graph()->clearData();
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
