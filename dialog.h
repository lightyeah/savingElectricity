#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QVector>
#include <qcustomplot.h>
#include <QTimer>
typedef double datatype;
namespace Ui {
class Dialog;
}

struct dataToPlot
{
    QVector<datatype> A;
    QVector<datatype> B;
    QVector<datatype> C;
    QVector<datatype> S;
    QVector<datatype> keys;
};

struct instruction
{
    QByteArray voltageInstruction;
    QByteArray currentInstruction;
    QByteArray effectivePowerInstruction;
    QByteArray reactivePowerInstruction;
    QByteArray apparentPowerInstruction;
    QByteArray powerFactorInstruction;
};

enum readType
{
    NoneType=0,
    ReadVoltage,
    ReadCurrent,
    ReadEffectivePower,
    ReadReactivePower,
    ReadApparentPower,
    ReadPowerFactor
};

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    void initPort();
    void initData();
    void initPlotStyle();
    void initConnections();
    void insertData(datatype data, QVector<datatype>& dataVector,QVector<datatype>& keys);
    void updateGraph(dataToPlot &data);
    ~Dialog();

private slots:
    void on_launch_clicked();
    void parseData();
    void on_stopPlot_clicked();
    void on_startPlot_clicked();
    void getVoltage();
    void getCurrent();
    void getEffectivePower();
    void getReactivePower();
    void getApparentPower();
    void getPowerFactor();
    void whichToPlot();
    void clearPlotData();
    void handleTimeout();
    void startTimeoutTimer();
    void updatePlotStyle();


signals:
    void voltageDataGot();
    void currentDataGot();
    void effectivePowerDataGot();
    void reactivePowerDataGot();
    void apparentPowerDataGot();
    void powerFactorDataGot();


private:
    Ui::Dialog *ui;
    QSerialPort * portWrite;
    readType m_readType;
    dataToPlot m_voltage;
    dataToPlot m_current;
    dataToPlot m_effectivePower;
    dataToPlot m_reactivePower;
    dataToPlot m_apparentPower;
    dataToPlot m_powerFactor;
    QTimer * timer;
    QTimer * timeout;
    instruction m_instruction;
    QByteArray buffer;
    int count;





};

#endif // DIALOG_H
