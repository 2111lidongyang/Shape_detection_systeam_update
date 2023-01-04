#include "mainwindow.h"

#include<QtCharts>
QT_CHARTS_USE_NAMESPACE
#include "ui_mainwindow.h"
#include<QPixmap>
#include <QScreen>
#include <opencv2/opencv.hpp>
#include <QDebug>
#include<QChartView>

using namespace cv;
using namespace std;


#define single_or_loop 0

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_start_clicked()
{

}

void MainWindow::on_pushButton_open_clicked()
{

    QBarSeries* series = new QBarSeries();// QBarSeries 类将一系列数据显示为按类别分组的垂直条。
    QBarCategoryAxis* axisX = new QBarCategoryAxis(); // QBarCategoryAxis类向图表的轴添加类别。
    QValueAxis* axisY = new QValueAxis();// 创建Y轴

     String pattern = "C:\\Users\\86195\\Pictures\\testimg\\*.jpg";
     vector<string> img_path;
     vector<cv::String> fn;
     glob(pattern, fn, false);
     size_t count = fn.size(); //number of png files in images folder
     for (size_t i = 0; i < count; i++)
     {
        img_path.push_back(fn[i]);
     }
     Mat img,hsv,cloneImg;
     for (int i = 0; i <fn.size(); i++)
     {
       int num_star = 0;int num_white = 0; int num_black = 0;int num_rect= 0;
       img = imread(img_path[i]);
       if (img.data == NULL)
       {
          break;
       }

       cloneImg =img.clone();
       cv::resize(img,img,Size(480,640));
       cv::resize(cloneImg,cloneImg,Size(480,640));

       cvtColor(img,hsv,COLOR_BGR2HSV);
       vector<Mat> channels;
       split(hsv, channels);//分离HSV
       Canny(channels[2],channels[2],3000,600,5);

       vector<vector<Point>> contours;
       vector<Vec4i> hierachy;
       //vector<RotatedRect> box(contours.size());
       findContours(channels[2], contours, hierachy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(-1,-1));
       vector<RotatedRect> box(contours.size());
               for (size_t i = 0;i<contours.size();i++)
               {
                   double area_rect = contourArea(contours[i]);
                   if(area_rect > 2000 && area_rect<20000)
                   {
                       box[i] = minAreaRect(Mat(contours[i]));
                       double area_minrect = box[i].size.area();
                       double result =  area_rect/area_minrect;
                      if( result>0.95)
                      {
                          num_rect++;
                      }
                      if(result<0.95 && result>0.75)
                      {
                          int x = box[i].center.x;
                          int y = box[i].center.y;
                          Vec3b intensity =img.at<Vec3b>(y, x);//读一个RGB像素点的像素值
                          int blue = intensity.val[0]; /*通道0*/
                          if (blue < 150)
                          {
                              num_black++;
                          }
                          else
                          {
                              num_white++;
                          }
                      }
                      if(result>0.3 && result<0.75)
                      {
                          num_star++;
                      }
                      drawContours(cloneImg, contours, i, Scalar(0, 255, 0), 3, 8, hierachy);
                    }
               }

               qDebug() <<" 白色棋子个数：" <<num_white <<"\n" <<"黑色棋子个数：" <<num_black <<"\n"<<"矩形个数："<<num_rect<<"\n"<<"五角星个数："<<num_star;
               qDebug() <<"                              "<<"\n";


          //将结果显示在label上(图片在label中显示要转一下格式)
          ui->label_result->setPixmap(QPixmap::fromImage(QImage(cloneImg.data,cloneImg.cols,cloneImg.rows,QImage::Format_BGR888)));
          ui->label_result->setScaledContents(true);
          ui->label_result->setAlignment(Qt::AlignCenter);
          ui->label_src->setPixmap(QPixmap::fromImage(QImage(img.data,img.cols,img.rows,QImage::Format_BGR888)));
          ui->label_src->setScaledContents(true);
          ui->label_src->setAlignment(Qt::AlignCenter);

        //将结果在textEdit显示
        QString rect= QString::number(num_rect);
        ui->textEdit_rect->clear();
        ui->textEdit_rect->append(rect);
        QString star= QString::number(num_star);
        ui->textEdit_star->clear();
        ui->textEdit_star->append(star);
        QString black= QString::number(num_black);
        ui->textEdit_black->clear();
        ui->textEdit_black->append(black);
        QString white= QString::number(num_white);
        ui->textEdit_white->clear();
        ui->textEdit_white->append(white);


     //条形图
    QBarSet* set0 = new QBarSet("检测结果"); // QBarSet 类表示条形图中的一组条形
    *set0 <<num_black <<num_white  << num_rect << num_star; // 为每一个条形添加数据

    series->clear();
    series->append(set0);

    QChart *chart = ui->chartView->chart(); //获取一个QChart对象，QChart类似于容器，可以加载不同的图表（series），并通过QChartView显示

    chart->addSeries(series);
    chart->setTitle("数据统计");
    chart->setAnimationOptions(QChart::SeriesAnimations);  // 设置图表变化时的动画效果
    QStringList categories;                                // X轴分类，一般与QBarSet中添加的数据个数相同，如果少了则会显示不全,多了不影响，但是不能重复
    categories << "黑子" << "白子"  << "矩形"<< "星";

    axisX->setTitleText("种类");                // 设置X轴标题
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);                // 将X轴放在图表的底部
    series->attachAxis(axisX);                             // 将axis指定的轴附着到series。

    axisY->setRange(0, 10);                   // 设置Y轴范围
    axisY->setTitleText("个数");  // 设置Y轴标题
    chart->addAxis(axisY, Qt::AlignLeft);     // Y轴左对齐
    series->attachAxis(axisY);
    chart->legend()->setVisible(true);             // 设置图例是否可见
    chart->legend()->setAlignment(Qt::AlignRight); // 设置图例显示的位置
    ui->chartView->setRenderHint(QPainter::Antialiasing);  // 设置抗锯齿

    Delay_MSec(1000);



//    //将数据保存到.txt
//    QFile file;
//        file.setFileName("C:\\Users\\86195\\Pictures\\1.txt");
//        if(file.open(QIODevice::ReadWrite | QIODevice::Append)) //Append：接着上文的内容写，不会覆盖原文
//        {
//            QTextStream stream(&file);
//            QString num_black = ui->textEdit_black->toPlainText();
//            QString num_white = ui->textEdit_white->toPlainText();
//            QString num_star = ui->textEdit_star->toPlainText();
//            QString num_rect = ui->textEdit_rect->toPlainText();
//            stream<< "Black Chess:"<<num_black <<"\n";
//            stream<< "White Chess:"<<num_white <<"\n";
//            stream<< "star:"<<num_star<< "\n";
//            stream<< "rect:"<<num_rect<< "\n";
//            stream<<"\n";
//            file.close();
//        }



}
}



void MainWindow::on_pushButton_back_clicked()
{
    QWidget::close();
    //    Mat src,dst;
    //    String pattern = "C:\\Users\\86195\\Pictures\\testimg\\*.jpg";
    //    vector<string> img_path;
    //    vector<cv::String> fn;
    //    glob(pattern, fn, false);

    //    QBarSeries* series = new QBarSeries();
    //    QBarCategoryAxis* axisX = new QBarCategoryAxis();
    //    QValueAxis* axisY = new QValueAxis();
    //    size_t count = fn.size(); //number of png files in images folder
    //    for (size_t i = 0; i < count; i++)
    //    {
    //       img_path.push_back(fn[i]);
    //    }
    //    Mat img;
    //    for (int i = 0; i <3; i++)
    //    {
    //      img = imread(img_path[i]);
    //          //qDebug() << i;
    //      if (img.data == NULL)
    //      {
    //         break;
    //      }

    //   int num_star = 0;int num_white = 0; int num_black = 0;int num_rect= 0;
    //   //Mat img = imread("C:\\Users\\86195\\Pictures\\testimg\\6.jpg");


    //   Mat dstImg =img.clone();
    //   cv::resize(img,img,Size(480,640));
    //   cv::resize(dstImg,dstImg,Size(480,640));
    //   cvtColor(img,dst,COLOR_BGR2HSV);//转灰度图
    //   vector<Mat> channels;
    //   split(dst, channels);//通道分离HSV
    //   //        imshow("V2GRAY", channels[0]);
    //   //        imshow("S2GRAY", channels[1]);
    //   //        imshow("H2GRAY", channels[2]);
    //   threshold(channels[2],channels[2],55,255,THRESH_BINARY);//阈值处理
    //   //medianBlur(channels[1],channels[1],7);//中值波处理
    //   //imshow("medianBlur",channels[1]);

    //   vector<vector<Point>> contours_1;
    //   vector<Vec4i> hierachy_1;
    //   findContours(channels[1], contours_1, hierachy_1, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(-1, -1));//轮廓检测
    //   //cout<<"num="<<contours_1.size()<<endl;
    //   vector<Rect> boundRect_1(contours_1.size());
    //   vector<RotatedRect> box_1(contours_1.size());
    //   Point2f rect_1[4];
    //           for (size_t i = 0;i<contours_1.size();i++)
    //           {
    //               double area_0 = contourArea(contours_1[i]);//轮廓面积
    //               //qDebug() <<"面积："<<area_0;
    //               if(area_0 > 3000 && area_0<20000)//判断面积，面积过小和过大都排除
    //               {
    //                   box_1[i] = minAreaRect(Mat(contours_1[i]));//获取最小外接矩形
    //                   boundRect_1[i] = boundingRect(Mat(contours_1[i]));//外界矩形
    ////                    cout<<"kuan:"<<box_1[i].size.width<<endl;
    ////                    cout<<"chang:"<<box_1[i].size.height<<endl;
    ////                    cout<<" "<<box_1[i].size.area()<<endl;
    //                   double area_1 = box_1[i].size.area();//获取最小外接矩形的面积
    //                   double result =  area_0/area_1;  //得到轮廓面积和最小外接矩形面积的比值
    //                  if( result>0.9)   //将比值大于0.9的判断为矩形
    //                  {
    //                      drawContours(dstImg, contours_1, i, Scalar(0, 255, 0), 3, 8, hierachy_1);
    //                      num_rect++;
    //                  }
    //                  if(result<0.9 && result>0.7) //将比值介于0.7到0.9之间的判断为白棋子
    //                  {
    //                      drawContours(dstImg, contours_1, i, Scalar(0, 255, 0), 3, 8, hierachy_1);
    //                      num_white++;
    //                  }
    //                }
    //           }

    //   cvtColor(img,img,COLOR_BGR2GRAY);//转灰度图
    //   threshold(img,img,100,255,THRESH_BINARY);//阈值处理
    //   //imshow("threshold",img);
    //   medianBlur(img,img,7);//中值波处理
    //   //imshow("medianBlur",img);
    //   vector<vector<Point>> contours_0;
    //   vector<Vec4i> hierachy_0;
    //   findContours(img, contours_0, hierachy_0, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(-1, -1));//轮廓检测
    //   //cout<<"num="<<contours_0.size()<<endl;
    //   vector<Rect> boundRect(contours_0.size());
    //   vector<RotatedRect> box(contours_0.size());
    //   Point2f rect[4];
    //           for (size_t i = 0;i<contours_0.size();i++)
    //           {
    //               double area_0 = contourArea(contours_0[i]);//轮廓面积
    //               //qDebug() <<"面积："<<area_0;

    //               if(area_0 > 3000 && area_0<20000)//判断面积，面积过小和过大都排除
    //               {
    //                   box[i] = minAreaRect(Mat(contours_0[i]));//获取最小外接矩形
    //                   boundRect[i] = boundingRect(Mat(contours_0[i]));//外界矩形
    ////                    cout<<"kuan:"<<box[i].size.width<<endl;
    ////                    cout<<"chang:"<<box[i].size.height<<endl;
    ////                    cout<<" "<<box[i].size.area()<<endl;
    //                   double area_1 = box[i].size.area();//获取最小外接矩形的面积
    //                   double result =  area_0/area_1;//得到轮廓面积和最小外接矩形面积的比值
    //                  if( result>0.9) //将比值大于0.9的判断为矩形
    //                  {
    //                      drawContours(dstImg, contours_0, i, Scalar(0, 255, 0), 3, 8, hierachy_0);
    //                      num_rect++;
    //                  }
    //                  if(result<0.9 && result>0.7)//将比值介于0.7到0.9之间的判断为黑棋子
    //                  {
    //                      drawContours(dstImg, contours_0, i, Scalar(0, 255, 0), 3, 8, hierachy_0);
    //                      num_black++;
    //                  }
    //                  if(result>0.5 && result<0.7)//将比值介于0.5到0.7之间的判断为五角星
    //                  {
    //                      drawContours(dstImg, contours_0, i, Scalar(0, 255, 0), 3, 8, hierachy_0);
    //                      num_star++;
    //                  }


    //                }
    //           }

}

void MainWindow::Delay_MSec(unsigned int msec)
{
    QTime _Timer = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < _Timer )
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}
