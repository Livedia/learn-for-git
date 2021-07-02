#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMenu>
#include <QToolBar>
#include <QDebug>
#include <QSerialPort>          // 提供访问串口的功能
#include <QSerialPortInfo>      // 提供系统中存在的串口信息
#include<QFile>                 //文件读写
#include<QFileDialog>
#include<QDir>
#include<QTextStream>
#include <QtConcurrent/QtConcurrent>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    t = new threadSon();//子线程
    //创建主线程发送轨迹点信息子线程接收处理的连接
    connect(this,&MainWindow::PathArray,t,&threadSon::getPath);

    //创建主线程发送当前机械臂位置信息与子线程接收处理的连接
    connect(this,&MainWindow::arrmNow,t,&threadSon::getArm);

    //子线程发送的信号和主线程处理的连接
    connect(t,&threadSon::toSignal,this,&MainWindow::GetSign);

    //连接滑动条信号和槽
    QObject::connect(&serial, &QSerialPort::readyRead, this, &MainWindow::serialPort_readyRead);   //串口接收函数
    //关联所有的滑动条到处理函数
    connect(ui->Joint_2,SIGNAL(valueChanged(int)),this,SLOT(on_Joint_1_valueChanged(int)));
    connect(ui->Joint_3,SIGNAL(valueChanged(int)),this,SLOT(on_Joint_1_valueChanged(int)));//函数有形参的时候要把数据类型写上
    connect(ui->Joint_4,SIGNAL(valueChanged(int)),this,SLOT(on_Joint_1_valueChanged(int)));
    connect(ui->Joint_5,SIGNAL(valueChanged(int)),this,SLOT(on_Joint_1_valueChanged(int)));
    connect(ui->Joint_6,SIGNAL(valueChanged(int)),this,SLOT(on_Joint_1_valueChanged(int)));
    connect(ui->Graper,SIGNAL(valueChanged(int)),this,SLOT(on_Joint_1_valueChanged(int)));

    //遍历电脑串口信息
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        ui->portBox->addItem(info.portName());
    }
     //设置初始显示的下拉条数值
    ui->portBox->setCurrentIndex(1);
    ui->boteBox->setCurrentIndex(4);


    //菜单栏设置

    //保存
    connect(ui->actionSave,&QAction::triggered,[=]()
    {
        QString fileName = QFileDialog::getOpenFileName(this,tr("请选择要操作的文件"),"F:",tr("本本文件(*txt)"));
        //"图片文件(*png *jpg);;"
        qDebug()<<"filename : "<<fileName;

        QFile wfile(fileName);//创建文件变量
        if(!wfile.open(QIODevice::WriteOnly))
        {
            qDebug()<<"打开文件失败";
            return;
        }
        int pj;
        for (auto it = pathName.begin();it!=pathName.end();++it  )
        {
            wfile.write(it->toLatin1());//将QString转换为Qbetry
            wfile.write(" ");
            //在path变量中遍历
            for (auto itt = path.begin();itt !=path.end() ;itt++ )//迭代器指针不能超出范围了！！！
            {
                if(*it ==itt->name )
                {
                    for(pj=0;pj<7;pj++)
                    {
                        wfile.write(QString::number(itt->armValue[pj]).toLatin1());
                        wfile.write(" ");
                    }
                }
            }
            wfile.write("\n");
        }
        wfile.close();
        qDebug()<<"路径保存成功！";
    });

    //读取
    connect(ui->actionLoad,&QAction::triggered,[=]()
    {
        QString fileName = QFileDialog::getOpenFileName(this,tr("请选择要操作的文件"),"F:",tr("本本文件(*txt)"));
        //"图片文件(*png *jpg);;"
        qDebug()<<"filename : "<<fileName;

        QFile file(fileName);//创建文件变量
        if(file.open(QFile::ReadOnly | QIODevice::Text))//以只读的方式打开文件
        {
            path.clear();//清空原有路径

            while (!file.atEnd())
            {
                QByteArray line = file.readLine();//    定义文件读取的缓冲区,读一行
                qDebug()<<line<<line.size();
                QString str[8];
                for (int i=0,j=0,k=0;line[i]!='\n' ;i++ )
                {
                    str[j][k]=line[i];
                    k++;
                    if(line[i]==' ')
                    {
                        j++;
                        k=0;
                    }
                }
               qDebug()<<str[0];     //str[0]是名称。str[1]-str[7]分别对应各电机位置

               pathName.push_back(str[0]);//将保存名字读入pathName
               point.name=str[0];
                for (int i=1; i<8;i++ )
                {
                    point.armValue.push_back(str[i].toInt());   //将每一行中存储的数据存入
                }
                path.push_back(point);
                point.armValue.clear();//清空一下向量
            }
            file.close();
        }

        //显示一下读取的信息
        qDebug()<<"pathName: "<<pathName;
        for (auto itt = path.begin();itt !=path.end() ;itt++ )
        {
            qDebug()<<"name: "<<itt->name;
            qDebug()<<"armValue: "<<itt->armValue;
            //上拉栏显示更新
            ui->singlePoint->addItem(itt->name);  //显示新加入的位置
            ui->pathBox->addItem(itt->name);
        }
        //轨迹显示更新
        QString pathshow;
        for (int i=0;i<pathName.size() ;i++ )
        {
            pathshow.append(pathName[i]);
            pathshow.append("-->");
        }
        ui->pathtext->setText(pathshow);
        qDebug()<<"导入轨迹成功！";


    });


}

MainWindow::~MainWindow()
{
    delete ui;
}


//串口连接
void MainWindow::on_ConnectArm_clicked()
{
    if(ui->ConnectArm->text()==QString("连接"))
    {
        //设置串口名
        serial.setPortName(ui->portBox->currentText());
        //设置波特率
        serial.setBaudRate(ui->boteBox->currentText().toInt());
        //设置数据位数
        switch(ui->dataBox->currentIndex())
        {
        case 0: serial.setDataBits(QSerialPort::Data8); break;
        case 1: serial.setDataBits(QSerialPort::Data7); break;
        case 2: serial.setDataBits(QSerialPort::Data6); break;
        case 3: serial.setDataBits(QSerialPort::Data5); break;
        default: break;
        }
        //设置奇偶校验
        switch(ui->checkBox->currentIndex())
        {
        case 0: serial.setParity(QSerialPort::NoParity); break;
        case 1: serial.setParity(QSerialPort::OddParity); break;
        case 2: serial.setParity(QSerialPort::EvenParity); break;
        default: break;
        }
        //设置停止位
        switch(ui->stopBox->currentIndex())
        {
        case 0: serial.setStopBits(QSerialPort::OneStop); break;
        case 2: serial.setStopBits(QSerialPort::TwoStop); break;
        default: break;
        }
        //设置流控制
        serial.setFlowControl(QSerialPort::NoFlowControl);
        //打开串口
        ui->stateEdit->append("打开串口成功\n");
        qDebug() << "打开串口成功\n";
        if(!serial.open(QIODevice::ReadWrite))
        {
            ui->stateEdit->append("打开串口失败\n");
            qDebug() << "打开串口失败\n";
            return;
        }
        //下拉菜单控件失能
        ui->portBox->setEnabled(false);
        ui->boteBox->setEnabled(false);
        ui->dataBox->setEnabled(false);
        ui->checkBox->setEnabled(false);
        ui->stopBox->setEnabled(false);
        ui->ConnectArm->setText(QString("断开"));
    }
    else
    {
        //关闭串口
        serial.close();
        ui->stateEdit->append("成功关闭串口\n");
        //下拉菜单控件使能
        ui->portBox->setEnabled(true);
        ui->boteBox->setEnabled(true);
        ui->dataBox->setEnabled(true);
        ui->checkBox->setEnabled(true);
        ui->stopBox->setEnabled(true);
        ui->ConnectArm->setText(QString("连接"));
    }
}

//串口接收函数
void MainWindow::serialPort_readyRead()
{
    //从接收缓冲区中读取数据
    buffer_r = serial.readAll();
    qDebug() << "接收到数据大小:"<<buffer_r.size();
    if(0==buffer.size())
    {
        qDebug() << "接收到的数据不足，仅为："<<buffer.size();
        buffer = buffer_r;
    }
    else
    {
        buffer = buffer.left(31);
        buffer = buffer + " " + buffer_r;

        QByteArray buffersend =buffer;           //复制一个收到的信息以备子线程发送
        qDebug() << "接收到数据:"<<buffer;
        ui->stateEdit->append( "接收到数据:"+buffer+"\n");

        //将收到的字符串根据空格分割 存入arm[]数组中      截取收到信息中的位置信息
        int t;
        int n = buffer.count();
        for(int i =0;i<6;i++)
        {
            t = buffer.indexOf(" ");           //第一个空格空格位置值给t
            arm[i] = buffer.left(t).toInt();   //将t位置左边数字存储到arm[i]内
            n=n-t-1;
            buffer = buffer.right(n);          //将第一个 空格后面内容再赋给buffer  从右边数n-t-1个内容

            t = buffer.indexOf(" ");           //第一个空格空格位置值给t
            arm[i] = buffer.left(t).toInt();   //将t位置左边数字存储到arm[i]内
            n=n-t-1;
            buffer = buffer.right(n);          //将第一个 空格后面内容再赋给buffer

            t = buffer.indexOf(" ");           //第一个空格空格位置值给t
            arm[i] = buffer.left(t).toInt();   //将t位置左边数字存储到arm[i]内
            n=n-t-1;
            buffer = buffer.right(n);          //将第一个 空格后面内容再赋给buffer
        }

        qDebug() << "分割后buffer为："<<buffer;
        buffer=buffer.right(5);
        qDebug() << "分割后buffer为："<<buffer;
        buffer=buffer.left(3);
        qDebug() << "分割后buffer为："<<buffer;
        arm[6]=buffer.toInt();     //接收夹爪位置信息


        buffer.clear();        //清空buffer
        for(int i =0;i<7;i++)
        {
            qDebug() << "分割后arm["<<i<<"]为："<<arm[i];
        }

        //如果信号标志位为真，要向子线程发送信号
        if(emitsign)
        {
            emit arrmNow(buffersend);  //向子线程发送当前机械臂位置
            qDebug()<<"向子线程发送当前机械臂位置"<<buffersend;
        }
    }
}

void MainWindow::serialPort_write(QByteArray data)    //串口输出函数
{
    QByteArray send;
    QVector<QByteArray> part;
    int t,n=data.count();

    for(int i =0;i<13;i++)
    {
        t = data.indexOf(" ");           //第一个空格空格位置值给t
        part.push_back(data.left(t));   //将t位置左边数字存储到part内
        n=n-t-1;
        data = data.right(n);           //将第一个 空格后面内容再赋给data  从右边数n-t-1个内容
    }
    part.push_back(data);

    qDebug()<<"存入向量的值为"<<part;

    //要处理一下角度值的位数

    for(int i = 7;i<14;i++)
    {
        if(1==part[i].count())
            part[i]="00"+part[i];
        if(2==part[i].count())
            part[i]="0"+part[i];
    }
   //编码         0 0 180 1 1 045 2 0 180 3 0 090 4 0 045 5 1 090 s 090   电机编号+空格+正反转+空格+角度
    send = "0 "+part[0]+" "+part[7]+" 1 "+part[1]+" "+part[8]
                                   +" 2 "+part[2]+" "+part[9]
                                   +" 3 "+part[3]+" "+part[10]
                                   +" 4 "+part[4]+" "+part[11]
                                   +" 5 "+part[5]+" "+part[12]
                                   +" s "+part[13];

    qDebug()<<"send"<<send;
    serial.write(send);

}



//滑动条值发送改变时候的处理函数
void MainWindow::on_Joint_1_valueChanged(int value)
{
    //获取滑动条数值
    slider[0] = ui->Joint_1->value();
    slider[1] = ui->Joint_2->value();
    slider[2] = ui->Joint_3->value();
    slider[3] = ui->Joint_4->value();
    slider[4] = ui->Joint_5->value();
    slider[5] = ui->Joint_6->value();
    slider[6] = ui->Graper->value();

    //显示滑动条的数值显示
    ui->spinBox_1->setValue(value);
    ui->spinBox_2->setValue(ui->Joint_2->value());
    ui->spinBox_3->setValue(ui->Joint_3->value());
    ui->spinBox_4->setValue(ui->Joint_4->value());
    ui->spinBox_5->setValue(ui->Joint_5->value());
    ui->spinBox_6->setValue(ui->Joint_6->value());
    ui->spinBox_7->setValue(ui->Graper->value());

    for (int i=0;i<7 ;i++ )
       {
           cmd[0][i] =slider[i]>arm[i];             //设置正反转方向，与机械臂真实位置进行对比，判断要正转还是反转
           cmd[1][i] = qAbs(slider[i]-arm[i]);      //设置要转动的角度
       }
    //合成要发送的字符串
    QString send = QString("%1").arg(cmd[0][0])+" "+QString("%1").arg(cmd[0][1])+" "+QString("%1").arg(cmd[0][2])+" "+QString("%1").arg(cmd[0][3])
              +" "+QString("%1").arg(cmd[0][4])+" "+QString("%1").arg(cmd[0][5])+" "+QString("%1").arg(cmd[0][6])
              +" "+QString("%1").arg(cmd[1][0])+" "+QString("%1").arg(cmd[1][1])+" "+QString("%1").arg(cmd[1][2])+" "+QString("%1").arg(cmd[1][3])
              +" "+QString("%1").arg(cmd[1][4])+" "+QString("%1").arg(cmd[1][5])+" "+QString("%1").arg(cmd[1][6]);

    qDebug()<<send;
    ui->stateEdit->append( "发送数据:"+send+"\n");
    QByteArray data = send.toUtf8();
    //qDebug()<<data;
    //serial.write(data);
    serialPort_write(data);

}


//保存点的位置
void MainWindow::on_pushButton_2_clicked()
{

    ui->singlePoint->addItem(ui->PointName->text());  //显示新加入的位置
    ui->pathBox->addItem(ui->PointName->text());
//结构体存储
    point.name = ui->PointName->text();     //存储名称
    point.armValue.clear();                 //清空之前内容
    for (int cc=0;cc<7 ;cc++ )               //存储位置信息
    {
       point.armValue.push_back(arm[cc]);
    }
    path.push_back(point);       //将点存入路径

    for (auto it=path.begin();it!=path.end();it++ )
    {
        qDebug()<<"path内容：";
        qDebug()<<it->name<<it->armValue;
    }
}


//到达某个指定点
void MainWindow::on_excute_clicked()
{
  gopoint(ui->singlePoint->currentText());
}


//定义一个去某点的函数
void MainWindow::gopoint(QString name)
{
//结构体情况下
      for (auto it=path.begin();it!=path.end();it++ )
      {
          if (it->name==name)
             {
              int i = 0;
              for (auto itt=it->armValue.begin();itt!=it->armValue.end() ;itt++ )
                 {
                     cmd[0][i] =*itt>arm[i];//设置正反转方向
                     qDebug()<<"cmd[0][i] "<<cmd[0][i] ;
                     cmd[1][i] = qAbs(*itt-arm[i]);  //设置要转动的角度
                     qDebug()<<"cmd[1][i]"<<cmd[1][i] ;
                     ++i;
                 }
             }
      }

      //合成要发送的字符串
       QString send2 = QString("%1").arg(cmd[0][0])+" "+QString("%1").arg(cmd[0][1])+" "+QString("%1").arg(cmd[0][2])+" "+QString("%1").arg(cmd[0][3])
                 +" "+QString("%1").arg(cmd[0][4])+" "+QString("%1").arg(cmd[0][5])+" "+QString("%1").arg(cmd[0][6])
                 +" "+QString("%1").arg(cmd[1][0])+" "+QString("%1").arg(cmd[1][1])+" "+QString("%1").arg(cmd[1][2])+" "+QString("%1").arg(cmd[1][3])
                 +" "+QString("%1").arg(cmd[1][4])+" "+QString("%1").arg(cmd[1][5])+" "+QString("%1").arg(cmd[1][6]);

      QByteArray data2 = send2.toUtf8();
      qDebug()<<"发送单片机原来版本数据："<<data2;
     // serial.write(data2);
      serialPort_write(data2);

}


//加入点到轨迹
void MainWindow::on_pushin_clicked()
{
    pathName.append(ui->pathBox->currentText());
    QString pathshow;
    for (int i=0;i<pathName.size() ;i++ )
    {
        pathshow.append(pathName[i]);
        pathshow.append("-->");
    }
    ui->pathtext->setText(pathshow);
}


//删除轨迹列表中最后一个元素
void MainWindow::on_pushout_clicked()
{
    pathName.pop_back();
    QString pathshow;
    for (int i=0;i<pathName.size() ;i++ )
    {
        pathshow.append(pathName[i]);
        pathshow.append("-->");
    }
    ui->pathtext->setText(pathshow);
}


//将记录的轨迹转换为一个一维数组给子线程友元变量
void MainWindow::on_GoButton_clicked()
{
    //开启子线程
    t->start();

   //int pathmsg[1000];
   int pi=0;
   int pj=0;
   //利用迭代器遍历给友元数组赋值
   for (auto itt = pathName.begin();itt != pathName.end() ;++itt )
   {
       for (auto it = path.begin();it !=path.end() ;it++ )
       {
           if (it->name == *itt)
           {
               for(pj=0;pj<7;pj++)
               {
                t->pathmsg[pi]= it->armValue[pj];
                pi++;
               }
           }
       }
   }
   t->pathmsg[pi]= 999;//设置结束标志位

   //emit PathArray(pathmsg);  //将数组信息发送给子线程
   emitsign=true;
   qDebug()<<"emitsign=true";

}


//从子线程得到发送序号处理函数
void MainWindow::GetSign(int xuhao)
{
    qDebug()<<"接收序号"<<xuhao;

    if(xuhao==999)
    {
        t->terminate();//结束线程
        emitsign=false;
        qDebug()<<"emitsign=false结束线程";
    }
    else
    {
        gopoint(pathName[xuhao]);
    }
}


//快速保存
void MainWindow::on_save_clicked()
{
    qDebug()<<"准备保存路径信息！";
    QString dirPath = QCoreApplication::applicationDirPath();//得到当前exe所在文件夹

    QFile wfile(dirPath+"/data/saveFile.txt");//没有文件就新建一个

    if(!wfile.open(QIODevice::WriteOnly))
    {
        qDebug()<<"打开文件失败";
        return;
    }
    int pj;
    for (auto it = pathName.begin();it!=pathName.end();++it  )
    {
        wfile.write(it->toLatin1());//将QString转换为Qbetry
        wfile.write(" ");
        //在path变量中遍历
        for (auto itt = path.begin();itt !=path.end() ;itt++ )//迭代器指针不能超出范围了！！！
        {
            if(*it ==itt->name )
            {
                for(pj=0;pj<7;pj++)
                {
                    wfile.write(QString::number(itt->armValue[pj]).toLatin1());
                    wfile.write(" ");
                }
            }
        }
        wfile.write("\n");
    }
    wfile.close();
    qDebug()<<"快速保存成功！";
}


//快速导入
void MainWindow::on_load_clicked()
{
    qDebug()<<"快速导入路径";
    QString dirPath = QCoreApplication::applicationDirPath();//得到当前exe所在文件夹
    qDebug()<<dirPath;
    QFile file(dirPath+"/data/saveFile.txt");//定义QFile对象
    if(file.open(QFile::ReadOnly | QIODevice::Text))//以只读的方式打开文件
    {
        path.clear();//清空原有路径

        while (!file.atEnd())
        {
            QByteArray line = file.readLine();//    定义文件读取的缓冲区,读一行
            qDebug()<<line<<line.size();
            QString str[8];
            for (int i=0,j=0,k=0;line[i]!='\n' ;i++ )
            {
                str[j][k]=line[i];
                k++;
                if(line[i]==' ')
                {
                    j++;
                    k=0;
                }
            }
           qDebug()<<str[0];     //str[0]是名称。str[1]-str[7]分别对应各电机位置

           pathName.push_back(str[0]);//将保存名字读入pathName
           point.name=str[0];
            for (int i=1; i<8;i++ )
            {
                point.armValue.push_back(str[i].toInt());   //将每一行中存储的数据存入
            }
            path.push_back(point);
            point.armValue.clear();//清空一下向量
        }
        file.close();
    }

    //显示一下读取的信息
    qDebug()<<"pathName: "<<pathName;
    for (auto itt = path.begin();itt !=path.end() ;itt++ )
    {
        qDebug()<<"name: "<<itt->name;
        qDebug()<<"armValue: "<<itt->armValue;
        //上拉栏显示更新
        ui->singlePoint->addItem(itt->name);  //显示新加入的位置
        ui->pathBox->addItem(itt->name);
    }
    //轨迹显示更新
    QString pathshow;
    for (int i=0;i<pathName.size() ;i++ )
    {
        pathshow.append(pathName[i]);
        pathshow.append("-->");
    }
    ui->pathtext->setText(pathshow);
    qDebug()<<"导入轨迹成功！";
}
