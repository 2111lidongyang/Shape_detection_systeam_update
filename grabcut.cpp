#include "grabcut.h"
#include<opencv2\highgui\highgui.hpp>
#include<opencv2\imgproc\imgproc.hpp>
#include<iostream>
#include <opencv2\opencv.hpp>
#include <math.h>
using namespace cv;
using namespace std;


grabcut::grabcut()
{

}


void grabcut::showImage()
{
    Mat result, binmask;
        binmask = mask & 1;				//进一步掩膜
        if (init)						//进一步抠出无效区域。鼠标按下，init变为false
        {
            srcImg.copyTo(result, binmask);
        }
        else
        {
            result = srcImg.clone();
        }
        rectangle(result, rect, Scalar(0, 0, 255), 2, 8);
        imshow("源图像", result);

}

void grabcut::runGrabCut()
{
    if (init)//鼠标按下，init变为false
            grabCut(srcImg, mask, rect, bgModel, fgModel, 1);//第二次迭代，用mask初始化grabcut
        else
        {
            grabCut(srcImg, mask, rect, bgModel, fgModel, 1, GC_INIT_WITH_RECT);//用矩形窗初始化GrabCut
            init = true;
        }

}


void grabcut::onMouse(int events, int x, int y, int flag, void *)
{
    if (x < 0 || y < 0 || x > srcImg.cols || y > srcImg.rows)	//无效区域
            return;


        if (events == EVENT_LBUTTONDOWN)
        {
            setMouse = true;
            pt.x = x;
            pt.y = y;
            init = false;
        }
        else if (events == EVENT_MOUSEMOVE)//鼠标只要动，就执行一次
        {
            if (setMouse == true)			//鼠标左键按住，滑动
            {
                Point pt1;
                pt1.x = x;
                pt1.y = y;
                rect = Rect(pt, pt1);//定义矩形区域
                showImage();
                mask.setTo(Scalar::all(GC_BGD));//背景
                mask(rect).setTo(Scalar(GC_PR_FGD));//前景			    //对rect内部设置为可能的前景，外部设置为背景
            }
        }
        else if (events == EVENT_LBUTTONUP)
            setMouse = false;	        	//鼠标左键抬起

}
