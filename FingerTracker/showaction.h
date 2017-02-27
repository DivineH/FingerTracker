/*
 *    ���ڼ����Լ�������������ƴ����豸�������ʵ��
 *
 *    Show the action of the finger
 *
 *    By Hc
 */
#pragma once

#include "LeastSquareMethod.h"
#include "cv_common.h"
#include "common.h"
#include "modes.h"

extern Blob BlobFinder;
bool newAction = false;
double slope = 0;
std::vector<cv::Point2f>  PointVec;

MoveMode ShowAction(PowerVideoCapture & MyCam,int id)
{
	cvNamedWindow("SrcFingerTrace");
	cvNamedWindow("DstFingerTrace");

	IplImage * src=cvCreateImage(cvSize(IMG_WIDTH,IMG_HEIGHT), IPL_DEPTH_8U, 3);
	IplImage * dst=cvCreateImage(cvSize(IMG_WIDTH,IMG_HEIGHT), IPL_DEPTH_8U, 1);

	while(1)
	{
		if(MyCam.isFrameNew(id))       
		{
			MyCam.getPixels(id, (unsigned char *)src->imageData, false, true);   
			
			
			cvCvtColor(src,dst,CV_BGR2GRAY);
			
			cvSmooth(dst,dst,CV_GAUSSIAN);
			
			cvThreshold(dst,dst,100,255,CV_THRESH_BINARY);
			
			IplConvKernel * pKernel = cvCreateStructuringElementEx(15,15,8,8,CV_SHAPE_ELLIPSE,NULL);
			cvErode(dst,dst,pKernel,1);
			cvDilate(dst,dst,pKernel,1);

			char c=cvWaitKey(1);
			if(c==27) 
				break;

			cvShowImage("DstFingerTrace",dst);
			cvShowImage("SrcFingerTrace",src);
			
			if(BlobFinder.GetPointVec(dst,PointVec) == 0)    
			{
				if(newAction)    
				{
					
					LSM(PointVec);
					//����slope�ж��ƶ�����     ��Ҫ��ROI.xml�ļ�
					
					PointVec.clear();
					newAction = false;
				}
				else
					continue;
			}
			else    //ͼƬ���й��
			{
				if(newAction)
					continue;
				newAction = true;
			}
		}
	}
	
	cvDestroyAllWindows();
	return MoveDown;
}