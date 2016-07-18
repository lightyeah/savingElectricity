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
    connect(portWrite,&QSerialPort::bytesWritten,this,&Dialog::startTimeoutTimer);

}

void Dialog::initData()
{
    m_data.count=0;
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
    ui->PlotA->graph(0)->setPen(QPen(Qt::blue));
    ui->PlotA->graph(0)->setName("Phase A");
    ui->PlotA->xAxis->setLabel("time");
    ui->PlotA->xAxis->setRange(0,51);
    ui->PlotA->yAxis->setRange(0,240);
    ui->PlotB->addGraph();
    ui->PlotB->graph(0)->setPen(QPen(Qt::green));
    ui->PlotB->xAxis->setRange(0,51);
    ui->PlotB->yAxis->setRange(220,270);
    ui->PlotC->addGraph();
    ui->PlotC->graph(0)->setPen(QPen(Qt::red));
    ui->PlotC->xAxis->setRange(0,51);
    ui->PlotC->yAxis->setRange(220,270);
    ui->PlotA->plotLayout()->insertRow(0);
    ui->PlotA->plotLayout()->addElement(0, 0, new QCPPlotTitle(ui->PlotA, "A相"));
    ui->PlotB->plotLayout()->insertRow(0);
    ui->PlotB->plotLayout()->addElement(0, 0, new QCPPlotTitle(ui->PlotB, "B相"));
    ui->PlotC->plotLayout()->insertRow(0);
    ui->PlotC->plotLayout()->addElement(0, 0, new QCPPlotTitle(ui->PlotC, "C相"));

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
    //    qDebug()<<"begin read and parse data "<<endl;
    //    qDebug()<<"read type is "<<m_readType<<endl;
    //    qDebug()<<"the number of bytes that have been read is "<<m_data.count<<endl;

    switch(m_readType)
    {
    case ReadVoltage:
        m_data.buffer.append(portWrite->readAll());
//        ++m_data.count;
        if(m_data.buffer.size()==BYTE_NUMBER_VOLTAGE)
        {
            qDebug()<<"read type is "<<m_readType<<endl;
            qDebug()<<"the number of bytes that have been read is "<<m_data.count<<endl;
            timeout->stop();
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
            qDebug()<<"the ByteArray is "<<m_data.buffer.toHex()<<endl;
            qDebug()<<"the buffer size is "<<m_data.buffer.size()<<endl;
            insertData(A,m_voltage.A,m_voltage.keys);
            insertData(B,m_voltage.B,m_voltage.keys);
            insertData(C,m_voltage.C,m_voltage.keys);
            qDebug()<<"size of keys "<<m_voltage.keys.size()<<" size of value "<<m_voltage.A.size()<<endl;
            m_data.count=0;
            m_data.buffer.clear();
            emit voltageDataGot();
            count++;
            qDebug()<<"the total is "<<count<<endl;
        }
        else if(m_data.buffer.size()>BYTE_NUMBER_VOLTAGE)
        {
            timeout->stop();
            m_data.count=0;
            m_data.buffer.clear();
            getVoltage();
        }
        break;
    case ReadCurrent:
        m_data.buffer.append(portWrite->readAll());
        ++m_data.count;
        if(m_data.buffer.size()==BYTE_NUMBER_CURRENT)
        {
            timeout->stop();
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
//            qDebug()<<"the ByteArray is "<<m_data.buffer.toHex()<<endl;
            insertData(A,m_current.A,m_current.keys);
            insertData(B,m_current.B,m_current.keys);
            insertData(C,m_current.C,m_current.keys);
            m_data.count=0;
            m_data.buffer.clear();
            emit currentDataGot();
        }
        else if(m_data.buffer.size()>BYTE_NUMBER_CURRENT)
        {
            timeout->stop();
            m_data.count=0;
            m_data.buffer.clear();
            getCurrent();
        }
        break;
    case ReadEffectivePower:
        m_data.buffer.append(portWrite->readAll());
        ++m_data.count;
        if(m_data.buffer.size()==BYTE_NUMBER_EFFECTIVE_POWER)
        {
            timeout->stop();
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
        else if(m_data.buffer.size()>BYTE_NUMBER_EFFECTIVE_POWER)
        {
            timeout->stop();
            m_data.count=0;
            m_data.buffer.clear();
            getEffectivePower();
        }
        break;
    case ReadReactivePower:
        m_data.buffer.append(portWrite->readAll());
        ++m_data.count;
        if(m_data.buffer.size()==BYTE_NUMBER_REACTIVE_POWER)
        {
            timeout->stop();
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
        else if(m_data.buffer.size()>BYTE_NUMBER_REACTIVE_POWER)
        {
            timeout->stop();
            m_data.count=0;
            m_data.buffer.clear();
            getReactivePower();
        }
        break;
    case ReadApparentPower:
        m_data.buffer.append(portWrite->readAll());
        ++m_data.count;
        if(m_data.buffer.size()==BYTE_NUMBER_APPRENT_POWER)
        {
            timeout->stop();
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
        else if(m_data.buffer.size()>BYTE_NUMBER_APPRENT_POWER)
        {
            timeout->stop();
            m_data.count=0;
            m_data.buffer.clear();
            getApparentPower();
        }
        break;
    case ReadPowerFactor:
        m_data.buffer.append(portWrite->readAll());
        ++m_data.count;
        if(m_data.buffer.size()==BYTE_NUMBER_POWER_FACTOR)
        {
            timeout->stop();
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
        else if(m_data.buffer.size()>BYTE_NUMBER_POWER_FACTOR)
        {
            timeout->stop();
            m_data.count=0;
            m_data.buffer.clear();
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

}

void Dialog::getCurrent()
{
    //    static QByteArray data(QString("68aaaaaaaaaaaa68110433323535b016").toLocal8Bit());
    qDebug()<<"before read current"<<endl;
    m_readType=ReadCurrent;
    portWrite->write(m_instruction.currentInstruction);
    qDebug()<<"fetch current"<<endl;


}

void Dialog::getEffectivePower()
{
    //    static QByteArray data(QString("68aaaaaaaaaaaa68110433323635b116").toLocal8Bit());
    qDebug()<<"before read effectivePower"<<endl;
    m_readType=ReadEffectivePower;
    portWrite->write(m_instruction.effectivePowerInstruction);
    qDebug()<<"fetch effective Power"<<endl;

}

void Dialog::getReactivePower()
{
    //    static QByteArray data(QString("68aaaaaaaaaaaa68110433323735b216").toLocal8Bit());
    qDebug()<<"before read reactivePower"<<endl;
    m_readType=ReadReactivePower;
    portWrite->write(m_instruction.reactivePowerInstruction);
    qDebug()<<"fetch reactive Power"<<endl;

}

void Dialog::getApparentPower()
{
    //    static QByteArray data(QString("68aaaaaaaaaaaa68110433323835b316").toLocal8Bit());
    qDebug()<<"before read apparentPower"<<endl;
    m_readType=ReadApparentPower;
    portWrite->write(m_instruction.apparentPowerInstruction);
    qDebug()<<"fetch Apparent Power"<<endl;

}

void Dialog::getPowerFactor()
{
    //    static QByteArray data(QString("68aaaaaaaaaaaa68110433323935b416").toLocal8Bit());
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
            clearPlotData();
            //            ui->PlotA->graph()->rescaleAxes();
            qDebug()<<"size of keys "<<m_voltage.keys<<" size of value "<<m_voltage.A<<endl;

            ui->PlotA->graph()->addData(m_voltage.keys,m_voltage.A);
            ui->PlotB->graph()->addData(m_voltage.keys,m_voltage.B);
            ui->PlotC->graph()->addData(m_voltage.keys,m_voltage.C);
            ui->PlotA->yAxis->rescale();
            ui->PlotB->yAxis->rescale();
            ui->PlotC->yAxis->rescale();

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
    timer->start(1000);
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
        m_data.buffer.clear();
        emit voltageDataGot();
        qDebug()<<"lost voltage"<<endl;
    }
    else if(m_readType==ReadCurrent)
    {
        timeout->stop();
        m_data.buffer.clear();
        emit currentDataGot();
        qDebug()<<"lost current"<<endl;
    }
    else if(m_readType==ReadEffectivePower)
    {
        timeout->stop();
        m_data.buffer.clear();
        emit effectivePowerDataGot();
        qDebug()<<"lost effectivePower"<<endl;
    }
    else if(m_readType==ReadReactivePower)
    {
        timeout->stop();
        m_data.buffer.clear();
        emit reactivePowerDataGot();
        qDebug()<<"lost reactivePower"<<endl;
    }
    else if(m_readType==ReadApparentPower)
    {
        timeout->stop();
        m_data.buffer.clear();
        emit apparentPowerDataGot();
        qDebug()<<"lost apparentPower"<<endl;
    }
    else if(m_readType==ReadPowerFactor)
    {
        timeout->stop();
        m_data.buffer.clear();
        emit powerFactorDataGot();
        qDebug()<<"lost PowerFactor"<<endl;
    }
}

void Dialog::startTimeoutTimer()
{
    timeout->start(1000);
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
