#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QtSerialPort/QSerialPort>
#include <QVector>
#include <qcustomplot.h>
typedef double datatype;
namespace Ui {
class Dialog;
}

struct dataToRead
{
//    int countVoltage;
//    int countCurrent;
//    int countEffectivePower;
//    int countReactivePower;
//    int countApparentPower;
//    int countPowerFactor;
    int count;
//    QByteArray voltage;
//    QByteArray current;
//    QByteArray effectivePower;
//    QByteArray reactivePower;
//    QByteArray apparentPower;
//    QByteArray powerFactor;
    QByteArray buffer;
};
struct dataToPlot
{
    QVector<datatype> A;
    QVector<datatype> B;
    QVector<datatype> C;
    QVector<datatype> S;
    QVector<datatype> keys;
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
    void getVoltage();
    void getCurrent();
    void getEffectivePower();
    void getReactivePower();
    void getApparentPower();
    void getPowerFactor();
    void whichToPlot();
    void insertData(datatype data, QVector<datatype>& dataVector,QVector<datatype>& keys);
    ~Dialog();

private slots:
    void on_launch_clicked();
    void parseData();
    void getAndPlotData();
    void on_stopPlot_clicked();
    void on_startPlot_clicked();

signals:
    void voltageDataGot(datatype A,datatype B,datatype C);
    void currentDataGot(datatype A,datatype B,datatype C);
    void effectivePowerDataGot(datatype A,datatype B,datatype C,datatype S);
    void reactivePowerDataGot(datatype A,datatype B,datatype C,datatype S);
    void apparentPowerDataGot(datatype A,datatype B,datatype C,datatype S);
    void powerFactorDataGot(datatype A,datatype B,datatype C,datatype S);


private:
    Ui::Dialog *ui;
    QSerialPort * portWrite;
    QSerialPort * portRead;
    dataToRead m_data;
    readType m_readType;
    dataToPlot m_voltage;
    dataToPlot m_current;
    dataToPlot m_effectivePower;
    dataToPlot m_reactivePower;
    dataToPlot m_apparentPower;
    dataToPlot m_powerFactor;





};

#endif // DIALOG_H
