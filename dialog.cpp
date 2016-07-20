#include "dialog.h"
#include "ui_dialog.h"
#include <QString>
#include <QTimer>
#include <QThread>
#include <algorithm>
#include <vector>
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
    timeout=new QTimer(this);
    connect(timer,&QTimer::timeout,this,&Dialog::getVoltage);
    connect(timeout,&QTimer::timeout,this,&Dialog::handleTimeout);
    count=0;
    initPort();
    initData();
    initConnections();
    initPlotStyle();
    timer->start(1000);
}

void Dialog::initPort()
{
    portWrite=new QSerialPort(this);
    foreach (const QSerialPortInfo &info,QSerialPortInfo::availablePorts())
    {
        QString portDescription=info.description();
        if(portDescription.contains(QString("USB")))
        {
            portWrite->setPort(info);
            ui->writePortNumber->setText(info.portName());
        }

    }

    portWrite->setBaudRate(QSerialPort::Baud9600);
    portWrite->setDataBits(QSerialPort::Data8);
    portWrite->setParity(QSerialPort::EvenParity);
    portWrite->setStopBits(QSerialPort::OneStop);
    portWrite->setFlowControl(QSerialPort::NoFlowControl);
    portWrite->open(QIODevice::ReadWrite);
    connect(portWrite,&QSerialPort::readyRead,this,&Dialog::parseData);
    connect(portWrite,&QSerialPort::bytesWritten,this,&Dialog::startTimeoutTimer);

}

void Dialog::initData()
{
    m_readType=NoneType;
    m_instruction.voltageInstruction=QString("68aaaaaaaaaaaa68110433323435af16").toLocal8Bit();
    m_instruction.currentInstruction=QString("68aaaaaaaaaaaa68110433323535b016").toLocal8Bit();
    m_instruction.effectivePowerInstruction=QString("68aaaaaaaaaaaa68110433323635b116").toLocal8Bit();
    m_instruction.reactivePowerInstruction=QString("68aaaaaaaaaaaa68110433323735b216").toLocal8Bit();
    m_instruction.apparentPowerInstruction=QString("68aaaaaaaaaaaa68110433323835b316").toLocal8Bit();
    m_instruction.powerFactorInstruction=QString("68aaaaaaaaaaaa68110433323935b416").toLocal8Bit();
    m_instruction.voltageInstruction=QByteArray::fromHex(m_instruction.voltageInstruction);
    m_instruction.currentInstruction=QByteArray::fromHex(m_instruction.currentInstruction);
    m_instruction.effectivePowerInstruction=QByteArray::fromHex(m_instruction.effectivePowerInstruction);
    m_instruction.reactivePowerInstruction=QByteArray::fromHex( m_instruction.reactivePowerInstruction);
    m_instruction.apparentPowerInstruction=QByteArray::fromHex(m_instruction.apparentPowerInstruction);
    m_instruction.powerFactorInstruction=QByteArray::fromHex(m_instruction.powerFactorInstruction);

}

void Dialog::initPlotStyle()
{

    ui->PlotA->addGraph();
//    ui->PlotA->legend->setVisible(true);
    ui->PlotA->legend->autoMargins();
    ui->PlotA->legend->setFont(QFont("Helvetica",9));
    ui->PlotA->graph(0)->setPen(QPen(Qt::blue));
//    ui->PlotA->graph(0)->setName("Phase A");
//    ui->PlotA->xAxis->setLabel("time");
    ui->PlotA->xAxis->setRange(0,51);
    ui->PlotA->yAxis->setRange(0,240);
    ui->PlotB->addGraph();
//    ui->PlotB->legend->setVisible(true);
    ui->PlotB->legend->autoMargins();
    ui->PlotB->legend->setFont(QFont("Helvetica",9));
    ui->PlotB->graph(0)->setPen(QPen(Qt::blue));
//    ui->PlotB->graph(0)->setName("Phase B");
    ui->PlotB->graph(0)->setPen(QPen(Qt::green));
    ui->PlotB->xAxis->setRange(0,51);
    ui->PlotB->yAxis->setRange(220,270);
    ui->PlotC->addGraph();
//    ui->PlotC->legend->setVisible(true);-
    ui->PlotC->legend->autoMargins();
    ui->PlotC->legend->setFont(QFont("Helvetica",9));
    ui->PlotC->graph(0)->setPen(QPen(Qt::blue));
//    ui->PlotC->graph(0)->setName("Phase C");
    ui->PlotC->graph(0)->setPen(QPen(Qt::red));
    ui->PlotC->xAxis->setRange(0,51);
    ui->PlotC->yAxis->setRange(220,270);
    ui->PlotA->plotLayout()->insertRow(0);
    ui->PlotA->plotLayout()->addElement(0, 0, new QCPPlotTitle(ui->PlotA, "A相"));
    ui->PlotB->plotLayout()->insertRow(0);
    ui->PlotB->plotLayout()->addElement(0, 0, new QCPPlotTitle(ui->PlotB, "B相"));
    ui->PlotC->plotLayout()->insertRow(0);
    ui->PlotC->plotLayout()->addElement(0, 0, new QCPPlotTitle(ui->PlotC, "总"));

}

void Dialog::initConnections()
{
    connect(this,&Dialog::voltageDataGot,this,&Dialog::getCurrent);
    connect(this,&Dialog::currentDataGot,this,&Dialog::getEffectivePower);
    connect(this,&Dialog::effectivePowerDataGot,this,&Dialog::getReactivePower);
    connect(this,&Dialog::reactivePowerDataGot,this,&Dialog::getApparentPower);
    connect(this,&Dialog::apparentPowerDataGot,this,&Dialog::getPowerFactor);
    connect(this,&Dialog::powerFactorDataGot,this,&Dialog::whichToPlot);
    connect(ui->radioButtonVoltage,&QRadioButton::clicked,this,&Dialog::updatePlotStyle);
    connect(ui->radioButtonCurrent,&QRadioButton::clicked,this,&Dialog::updatePlotStyle);
    connect(ui->radioButtonEffective,&QRadioButton::clicked,this,&Dialog::updatePlotStyle);
    connect(ui->radioButtonReactive,&QRadioButton::clicked,this,&Dialog::updatePlotStyle);
    connect(ui->radioButtonApparent,&QRadioButton::clicked,this,&Dialog::updatePlotStyle);
    connect(ui->radioButtonPowerFactor,&QRadioButton::clicked,this,&Dialog::updatePlotStyle);
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
    //    qDebug()<<"begin read and parse data "<<endl;
    //    qDebug()<<"read type is "<<m_readType<<endl;

    switch(m_readType)
    {
    case ReadVoltage:
        buffer.append(portWrite->readAll());
        if(buffer.size()==BYTE_NUMBER_VOLTAGE)
        {
            qDebug()<<"read type is "<<m_readType<<endl;
            qDebug()<<"the number of bytes that have been read is "<<buffer.size()<<endl;
            timeout->stop();
            QByteArray AL=buffer.mid(14,1);
            QByteArray AH=buffer.mid(15,1);
            datatype A=(minus33(AH.toHex()))*10+(minus33(AL.toHex()))*0.1;
            QByteArray BL=buffer.mid(16,1);
            QByteArray BH=buffer.mid(17,1);
            datatype B=(minus33(BH.toHex()))*10+(minus33(BL.toHex()))*0.1;
            QByteArray CL=buffer.mid(18,1);
            QByteArray CH=buffer.mid(19,1);
            datatype C=(minus33(CH.toHex()))*10+minus33(CL.toHex())*0.1;
            qDebug()<<"CL is "<<CL.toHex()<<endl;
            qDebug()<<"CH is "<<CH.toHex()<<endl;
            ui->readPortContent->setText(QString("%1").arg(A,0));
            qDebug()<<"get voltage data A "<<A<<endl;
            qDebug()<<"get voltage data C "<<C<<endl;
            qDebug()<<"the ByteArray is "<<buffer.toHex()<<endl;
            qDebug()<<"the buffer size is "<<buffer.size()<<endl;
            insertData(A,m_voltage.A,m_voltage.keys);
            insertData(B,m_voltage.B,m_voltage.keys);
            insertData(C,m_voltage.C,m_voltage.keys);
            qDebug()<<"size of keys "<<m_voltage.keys.size()<<" size of value "<<m_voltage.A.size()<<endl;
            buffer.clear();
            emit voltageDataGot();
            qDebug()<<"the total is "<<count<<endl;
        }
        else if(buffer.size()>BYTE_NUMBER_VOLTAGE)
        {
            timeout->stop();
            buffer.clear();
            getVoltage();
        }
        break;
    case ReadCurrent:
        buffer.append(portWrite->readAll());
        if(buffer.size()==BYTE_NUMBER_CURRENT)
        {
            timeout->stop();
            QByteArray AL=buffer.mid(14,1);
            QByteArray AM=buffer.mid(15,1);
            QByteArray AH=buffer.mid(16,1);
            datatype A=(minus33(AH.toHex()))*10+(minus33(AM.toHex()))*0.1+(minus33(AL.toHex()))*0.001;
            QByteArray BL=buffer.mid(17,1);
            QByteArray BM=buffer.mid(18,1);
            QByteArray BH=buffer.mid(19,1);
            datatype B=(minus33(BH.toHex()))*10+(minus33(BM.toHex()))*0.1+(minus33(BL.toHex()))*0.001;
            QByteArray CL=buffer.mid(20,1);
            QByteArray CM=buffer.mid(21,1);
            QByteArray CH=buffer.mid(22,1);
            datatype C=(minus33(CH.toHex()))*10+(minus33(CM.toHex()))*0.1+(minus33(CL.toHex()))*0.001;
            ui->readPortContent->setText(QString("%1").arg(A,0));
            qDebug()<<"get current data A "<<A<<endl;
            insertData(A,m_current.A,m_current.keys);
            insertData(B,m_current.B,m_current.keys);
            insertData(C,m_current.C,m_current.keys);
            buffer.clear();
            emit currentDataGot();
        }
        else if(buffer.size()>BYTE_NUMBER_CURRENT)
        {
            timeout->stop();
            buffer.clear();
            getCurrent();
        }
        break;
    case ReadEffectivePower:
        buffer.append(portWrite->readAll());
        if(buffer.size()==BYTE_NUMBER_EFFECTIVE_POWER)
        {
            timeout->stop();
            QByteArray SL=buffer.mid(14,1);
            QByteArray SM=buffer.mid(15,1);
            QByteArray SH=buffer.mid(16,1);
            datatype S=(minus33(SH.toHex()))+(minus33(SM.toHex()))*0.01+(minus33(SL.toHex()))*0.0001;
            QByteArray AL=buffer.mid(17,1);
            QByteArray AM=buffer.mid(18,1);
            QByteArray AH=buffer.mid(19,1);
            datatype A=(minus33(AH.toHex()))+(minus33(AM.toHex()))*0.01+(minus33(AL.toHex()))*0.0001;
            QByteArray BL=buffer.mid(20,1);
            QByteArray BM=buffer.mid(21,1);
            QByteArray BH=buffer.mid(22,1);
            datatype B=(minus33(BH.toHex()))+(minus33(BM.toHex()))*0.01+(minus33(BL.toHex()))*0.0001;
            QByteArray CL=buffer.mid(23,1);
            QByteArray CM=buffer.mid(24,1);
            QByteArray CH=buffer.mid(25,1);
            datatype C=(minus33(CH.toHex()))+(minus33(CM.toHex()))*0.01+(minus33(CL.toHex()))*0.0001;
            ui->readPortContent->setText(QString("%1").arg(A,0));
            qDebug()<<"get effectivePower data A "<<A<<endl;
            insertData(A,m_effectivePower.A,m_effectivePower.keys);
            insertData(B,m_effectivePower.B,m_effectivePower.keys);
            insertData(C,m_effectivePower.C,m_effectivePower.keys);
            insertData(S,m_effectivePower.S,m_effectivePower.keys);
            buffer.clear();
            emit effectivePowerDataGot();
        }
        else if(buffer.size()>BYTE_NUMBER_EFFECTIVE_POWER)
        {
            timeout->stop();
            buffer.clear();
            getEffectivePower();
        }
        break;
    case ReadReactivePower:
        buffer.append(portWrite->readAll());
        if(buffer.size()==BYTE_NUMBER_REACTIVE_POWER)
        {
            timeout->stop();
            QByteArray SL=buffer.mid(14,1);
            QByteArray SM=buffer.mid(15,1);
            QByteArray SH=buffer.mid(16,1);
            datatype S=(minus33(SH.toHex()))+(minus33(SM.toHex()))*0.01+(minus33(SL.toHex()))*0.0001;
            QByteArray AL=buffer.mid(17,1);
            QByteArray AM=buffer.mid(18,1);
            QByteArray AH=buffer.mid(19,1);
            datatype A=(minus33(AH.toHex()))+(minus33(AM.toHex()))*0.01+(minus33(AL.toHex()))*0.0001;
            QByteArray BL=buffer.mid(20,1);
            QByteArray BM=buffer.mid(21,1);
            QByteArray BH=buffer.mid(22,1);
            datatype B=(minus33(BH.toHex()))+(minus33(BM.toHex()))*0.01+(minus33(BL.toHex()))*0.0001;
            QByteArray CL=buffer.mid(23,1);
            QByteArray CM=buffer.mid(24,1);
            QByteArray CH=buffer.mid(25,1);
            datatype C=(minus33(CH.toHex()))+(minus33(CM.toHex()))*0.01+(minus33(CL.toHex()))*0.0001;
            ui->readPortContent->setText(QString("%1").arg(A,0));
            qDebug()<<"get reactivePower data A "<<A<<endl;
            insertData(A,m_reactivePower.A,m_reactivePower.keys);
            insertData(B,m_reactivePower.B,m_reactivePower.keys);
            insertData(C,m_reactivePower.C,m_reactivePower.keys);
            insertData(S,m_reactivePower.S,m_reactivePower.keys);
            buffer.clear();
            emit reactivePowerDataGot();
        }
        else if(buffer.size()>BYTE_NUMBER_REACTIVE_POWER)
        {
            timeout->stop();
            buffer.clear();
            getReactivePower();
        }
        break;
    case ReadApparentPower:
        buffer.append(portWrite->readAll());
        if(buffer.size()==BYTE_NUMBER_APPRENT_POWER)
        {
            timeout->stop();
            QByteArray SL=buffer.mid(14,1);
            QByteArray SM=buffer.mid(15,1);
            QByteArray SH=buffer.mid(16,1);
            datatype S=(minus33(SH.toHex()))+(minus33(SM.toHex()))*0.01+(minus33(SL.toHex()))*0.0001;
            QByteArray AL=buffer.mid(17,1);
            QByteArray AM=buffer.mid(18,1);
            QByteArray AH=buffer.mid(19,1);
            datatype A=(minus33(AH.toHex()))+(minus33(AM.toHex()))*0.01+(minus33(AL.toHex()))*0.0001;
            QByteArray BL=buffer.mid(20,1);
            QByteArray BM=buffer.mid(21,1);
            QByteArray BH=buffer.mid(22,1);
            datatype B=(minus33(BH.toHex()))+(minus33(BM.toHex()))*0.01+(minus33(BL.toHex()))*0.0001;
            QByteArray CL=buffer.mid(23,1);
            QByteArray CM=buffer.mid(24,1);
            QByteArray CH=buffer.mid(25,1);
            datatype C=(minus33(CH.toHex()))+(minus33(CM.toHex()))*0.01+(minus33(CL.toHex()))*0.0001;
            ui->readPortContent->setText(QString("%1").arg(A,0));
            qDebug()<<"get apparentPower data A "<<A<<endl;
            insertData(A,m_apparentPower.A,m_apparentPower.keys);
            insertData(B,m_apparentPower.B,m_apparentPower.keys);
            insertData(C,m_apparentPower.C,m_apparentPower.keys);
            insertData(S,m_apparentPower.S,m_apparentPower.keys);
            buffer.clear();
            emit apparentPowerDataGot();
        }
        else if(buffer.size()>BYTE_NUMBER_APPRENT_POWER)
        {
            timeout->stop();
            buffer.clear();
            getApparentPower();
        }
        break;
    case ReadPowerFactor:
        buffer.append(portWrite->readAll());
        if(buffer.size()==BYTE_NUMBER_POWER_FACTOR)
        {
            timeout->stop();
            QByteArray SL=buffer.mid(14,1);
            QByteArray SH=buffer.mid(15,1);
            datatype S=(minus33(SH.toHex()))*0.1+(minus33(SL.toHex()))*0.001;
            QByteArray AL=buffer.mid(16,1);
            QByteArray AH=buffer.mid(17,1);
            datatype A=(minus33(AH.toHex()))*0.1+(minus33(AL.toHex()))*0.001;
            QByteArray BL=buffer.mid(18,1);
            QByteArray BH=buffer.mid(19,1);
            datatype B=(minus33(BH.toHex()))*0.1+(minus33(BL.toHex()))*0.001;
            QByteArray CL=buffer.mid(20,1);
            QByteArray CH=buffer.mid(21,1);
            datatype C=(minus33(CH.toHex()))*0.1+(minus33(CL.toHex()))*0.001;
            ui->readPortContent->setText(QString("%1").arg(A,0));
            qDebug()<<"get powerFactor data A "<<A<<endl;
            insertData(A,m_powerFactor.A,m_powerFactor.keys);
            insertData(B,m_powerFactor.B,m_powerFactor.keys);
            insertData(C,m_powerFactor.C,m_powerFactor.keys);
            insertData(S,m_powerFactor.S,m_powerFactor.keys);
            buffer.clear();
            emit powerFactorDataGot();
        }
        else if(buffer.size()>BYTE_NUMBER_POWER_FACTOR)
        {
            timeout->stop();
            buffer.clear();
            getPowerFactor();
        }
        break;
    default:
        qDebug()<<"the default type is "<<m_readType<<endl;
        break;
    }
}


void Dialog::getVoltage()
{
    qDebug()<<"before read voltage"<<endl;
    timer->stop();
    m_readType=ReadVoltage;
    portWrite->write(m_instruction.voltageInstruction);
    qDebug()<<"fetch voltage"<<endl;
    qDebug()<<"write "<<m_instruction.voltageInstruction;

}

void Dialog::getCurrent()
{
    qDebug()<<"before read current"<<endl;
    m_readType=ReadCurrent;
    portWrite->write(m_instruction.currentInstruction);
    qDebug()<<"fetch current"<<endl;
}

void Dialog::getEffectivePower()
{
    qDebug()<<"before read effectivePower"<<endl;
    m_readType=ReadEffectivePower;
    portWrite->write(m_instruction.effectivePowerInstruction);
    qDebug()<<"fetch effective Power"<<endl;

}

void Dialog::getReactivePower()
{
    qDebug()<<"before read reactivePower"<<endl;
    m_readType=ReadReactivePower;
    portWrite->write(m_instruction.reactivePowerInstruction);
    qDebug()<<"fetch reactive Power"<<endl;

}

void Dialog::getApparentPower()
{
    qDebug()<<"before read apparentPower"<<endl;
    m_readType=ReadApparentPower;
    portWrite->write(m_instruction.apparentPowerInstruction);
    qDebug()<<"fetch Apparent Power"<<endl;

}

void Dialog::getPowerFactor()
{
    qDebug()<<"before read powerFactor"<<endl;
    m_readType=ReadPowerFactor;
    portWrite->write(m_instruction.powerFactorInstruction);
    qDebug()<<"fetch power factor"<<endl;

}

void Dialog::whichToPlot()
{
    qDebug()<<"excute wichToPlot"<<endl;
    if(!(ui->startPlot->isEnabled()))
    {
        if (ui->radioButtonVoltage->isChecked())
        {
            //            qDebug()<<"size of keys "<<m_voltage.keys<<" size of value "<<m_voltage.A<<endl;
            updateGraph(m_voltage);
            qDebug()<<"voltage"<<endl;
        }
        else if(ui->radioButtonCurrent->isChecked())
        {
            updateGraph(m_current);
            qDebug()<<"current"<<endl;
        }
        else if(ui->radioButtonEffective->isChecked())
        {
            updateGraph(m_effectivePower);
            qDebug()<<"effectivePower"<<endl;
        }
        else if(ui->radioButtonReactive->isChecked())
        {
            updateGraph(m_reactivePower);
            qDebug()<<"reactivePower"<<endl;
        }
        else if(ui->radioButtonApparent->isChecked())
        {
            updateGraph(m_apparentPower);
            qDebug()<<"apparentPower"<<endl;
        }
        else if(ui->radioButtonPowerFactor->isChecked())
        {
            updateGraph(m_powerFactor);
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

void Dialog::handleTimeout()
{
    if(m_readType==ReadVoltage)
    {
        timeout->stop();
        buffer.clear();
        emit voltageDataGot();
        qDebug()<<"lost voltage"<<endl;
    }
    else if(m_readType==ReadCurrent)
    {
        timeout->stop();
        buffer.clear();
        emit currentDataGot();
        qDebug()<<"lost current"<<endl;
    }
    else if(m_readType==ReadEffectivePower)
    {
        timeout->stop();
        buffer.clear();
        emit effectivePowerDataGot();
        qDebug()<<"lost effectivePower"<<endl;
    }
    else if(m_readType==ReadReactivePower)
    {
        timeout->stop();
        buffer.clear();
        emit reactivePowerDataGot();
        qDebug()<<"lost reactivePower"<<endl;
    }
    else if(m_readType==ReadApparentPower)
    {
        timeout->stop();
        buffer.clear();
        emit apparentPowerDataGot();
        qDebug()<<"lost apparentPower"<<endl;
    }
    else if(m_readType==ReadPowerFactor)
    {
        timeout->stop();
        buffer.clear();
        emit powerFactorDataGot();
        qDebug()<<"lost PowerFactor"<<endl;
    }
}

void Dialog::startTimeoutTimer()
{
    timeout->start(1000);
}

void Dialog::updatePlotStyle()
{
    if(ui->radioButtonVoltage->isChecked())
    {

        ui->PlotA->yAxis->setLabel("电压/V");
        //        ui->PlotA->yAxis->setRange(220,240);
        ui->PlotB->yAxis->setLabel("电压/V");
        //        ui->PlotB->yAxis->setRange(220,240);
        ui->PlotC->yAxis->setLabel("电压/V");
        //        ui->PlotC->yAxis->setRange(220,240);

    }
    else if(ui->radioButtonCurrent->isChecked())
    {
        //        std::max
        ui->PlotA->yAxis->setLabel("电流/A");
        ui->PlotB->yAxis->setLabel("电流/A");
        ui->PlotC->yAxis->setLabel("电流/A");
    }
    else if(ui->radioButtonEffective->isChecked())
    {
        ui->PlotA->yAxis->setLabel("有功功率/KW");
        ui->PlotB->yAxis->setLabel("有功功率/KW");
        ui->PlotC->yAxis->setLabel("有功功率/KW");
    }
    else if(ui->radioButtonReactive->isChecked())
    {
        ui->PlotA->yAxis->setLabel("无功功率/KW");
        ui->PlotB->yAxis->setLabel("无功功率/KW");
        ui->PlotC->yAxis->setLabel("无功功率/KW");
    }
    else if(ui->radioButtonApparent->isChecked())
    {
        ui->PlotA->yAxis->setLabel("视在功率/KW");
        ui->PlotB->yAxis->setLabel("视在功率/KW");
        ui->PlotC->yAxis->setLabel("视在功率/KW");
    }
    else if(ui->radioButtonPowerFactor->isChecked())
    {
        ui->PlotA->yAxis->setLabel("功率因素");
        ui->PlotB->yAxis->setLabel("功率因素");
        ui->PlotC->yAxis->setLabel("功率因素");
    }
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

void Dialog::updateGraph(dataToPlot &data)
{
    clearPlotData();
    ui->PlotA->graph()->addData(data.keys,data.A);
    ui->PlotB->graph()->addData(data.keys,data.B);
    if(ui->radioButtonApparent->isChecked()||ui->radioButtonReactive->isChecked()||ui->radioButtonPowerFactor->isChecked()||ui->radioButtonEffective->isChecked())
    {
        ui->PlotC->graph()->addData(data.keys,data.S);
    }
    else
    {
    ui->PlotC->graph()->addData(data.keys,data.C);
    }
    ui->PlotA->yAxis->rescale();
    ui->PlotB->yAxis->rescale();
    ui->PlotC->yAxis->rescale();
    ui->PlotA->replot();
    ui->PlotB->replot();
    ui->PlotC->replot();
}

double Dialog::minus33(QByteArray data)
{
    double result=0;
    QByteArray temp=data.mid(0,1);
    bool ok;
//    if(temp==Qbyt)
    result+=10*(temp.toLong(&ok,16)-3);
    temp=data.mid(1,1);
    result+=temp.toLong(&ok,16)-3;
    return result;

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
