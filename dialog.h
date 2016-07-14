#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QtSerialPort/QSerialPort>
namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    void initPort();
    ~Dialog();

private slots:
    void on_launch_clicked();
    void getData();

private:
    Ui::Dialog *ui;
    QSerialPort * portWrite;
    QSerialPort * portRead;
};

#endif // DIALOG_H
