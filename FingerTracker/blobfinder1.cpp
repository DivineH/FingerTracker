/*
 *    基于激光以及红外摄像的手势触控设备的设计与实现
 *
 *    Find blob
 *
 *    By Hc
 */
#include <conio.h>
#include <math.h>
#include "cv_common.h"
#include "blobfinder.h"
#include "event_injector.h"
#include "dev_conf.h"

#define CVCONTOUR_APPROX_LEVEL  1   // Approx.threshold - the bigger it is, the simpler is the boundary
extern Blob BlobFinder;

int Blob::GetPointVec(IplImage * src)
{
	if(!m_newAction)
		PointVec.clear();
	int count;
	CvMemStorage*	mem_storage	= NULL;
	CvMoments myMoments;

	if( mem_storage==NULL ) 
		mem_storage = cvCreateMemStorage(0);
	else
		cvClearMemStorage(mem_storage);

	CvSeq* contour_list = 0;
	
	count = cvFindContours(src,mem_storage,&contour_list, sizeof(CvContour),
			CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE);

	if(count == 0)
		return count;

	for (CvSeq* d = contour_list; d != NULL; d=d->h_next)
	{
		bool isHole = false;
		CvSeq* c = d;
		while (c != NULL)
		{                     
			double area = fabs(cvContourArea( c ));
			if( area >= BLOB_MIN_SIZE && area <= BLOB_MAX_SIZE)
			{
				CvSeq* approx;

			    approx = cvApproxPoly(c,sizeof(CvContour),mem_storage,CV_POLY_APPROX_DP, CVCONTOUR_APPROX_LEVEL,0);

				float area = (float)cvContourArea( approx, CV_WHOLE_SEQ );
				CvRect box = cvBoundingRect(approx);
				cvMoments( approx, &myMoments );

                Blob obj( box, cvPoint(0,0), fabs(area));

				if (myMoments.m10 > -DBL_EPSILON && myMoments.m10 < DBL_EPSILON)
				{
					obj.m_center.x = obj.m_box.x + obj.m_box.width/2;
					obj.m_center.y = obj.m_box.y + obj.m_box.height/2;
				}
				else
				{
					obj.m_center.x = myMoments.m10 / myMoments.m00;
					obj.m_center.y = myMoments.m01 / myMoments.m00;
				}
				PointVec.push_back(obj.m_center);
			}//END if( area >= minArea)

			if (isHole)
				c = c->h_next;//one_hole->h_next is another_hole
			else
				c = c->v_next;//one_contour->h_next is one_hole
			isHole = true;
		}//END while (c != NULL)
	}//END for
	cvReleaseMemStorage(&mem_storage);
	return count;
}

void Blob::ShowAction(PowerVideoCapture & MyCam,int id)
{
	int tempfingerCount = 0;
	long long count = 0;
	std::vector<INPUT> EventVec;
	INPUT input[4];
	CvMat * Mat = (CvMat *)cvLoad("ROI.xml");
	CvMat * _cam_map_x = (CvMat *)cvLoad("cam_map_x.xml");
    CvMat * _cam_map_y = (CvMat *)cvLoad("cam_map_y.xml");

	DefSlope = (double)(CV_MAT_ELEM(*Mat,short,1,1) - CV_MAT_ELEM(*Mat,short,0,1))/
		       (double)(CV_MAT_ELEM(*Mat,short,1,0) - CV_MAT_ELEM(*Mat,short,0,0));

	input[0].type = INPUT_KEYBOARD;
	input[0].ki.wVk = VK_DOWN;
	input[0].ki.dwFlags = 0;
	input[1].type = INPUT_KEYBOARD;
	input[1].ki.wVk = VK_DOWN;
	input[1].ki.dwFlags = KEYEVENTF_KEYUP;
	input[2].type = INPUT_KEYBOARD;
	input[2].ki.wVk = 0xBB;    
	input[2].ki.dwFlags = KEYEVENTF_KEYUP;
	input[3].type = INPUT_KEYBOARD;
	input[3].ki.wVk = 0x11;    //ctrl
	input[3].ki.dwFlags = KEYEVENTF_KEYUP;

	cvNamedWindow("SrcFingerTrace");
	cvNamedWindow("DstFingerTrace");

	IplImage * src=cvCreateImage(cvSize(IMG_WIDTH,IMG_HEIGHT), IPL_DEPTH_8U, 3);
	IplImage * dst=cvCreateImage(cvSize(IMG_WIDTH,IMG_HEIGHT), IPL_DEPTH_8U, 1);

	while(1)
	{
		if(MyCam.isFrameNew(id))       
		{
			MyCam.getPixels(id, (unsigned char *)src->imageData, false, true); 
			cvFlip(src,NULL,-1); 

			cvCvtColor(src,dst,CV_BGR2GRAY);
			cvSmooth(dst,dst,CV_GAUSSIAN);
			cvThreshold(dst,dst,200,255,CV_THRESH_BINARY);

			IplConvKernel * pKernel = cvCreateStructuringElementEx(15,15,8,8,CV_SHAPE_ELLIPSE,NULL);
			cvErode(dst,dst,pKernel,1);
			cvDilate(dst,dst,pKernel,1);

			char c=cvWaitKey(1);
			if(c==27) 
				break;

			cvSetImageROI(src,cvRect(CV_MAT_ELEM(*Mat,short,0,0),CV_MAT_ELEM(*Mat,short,0,1),
				                     CV_MAT_ELEM(*Mat,short,1,0),CV_MAT_ELEM(*Mat,short,1,1)));

			cvSetImageROI(dst,cvRect(CV_MAT_ELEM(*Mat,short,0,0),CV_MAT_ELEM(*Mat,short,0,1),
				                     CV_MAT_ELEM(*Mat,short,1,0),CV_MAT_ELEM(*Mat,short,1,1)));

			IplImage * clone=cvCloneImage(src);
			cvRemap(src, clone, _cam_map_x, _cam_map_y);

			cvShowImage("SrcFingerTrace",src);
			cvShowImage("DstFingerTrace",clone);		

			if((tempfingerCount = BlobFinder.GetPointVec(dst)) == 0)    
			{
				if(m_newAction)    
				{
					GetTime();
					switch(LSM())
					{
					case MInvalid:
						break;
					case MoveDown:
						input[0].type = INPUT_KEYBOARD;
						input[0].ki.wVk = VK_DOWN;
						EventVec.push_back(input[0]);
						input[1].type = INPUT_KEYBOARD;
						input[1].ki.wVk = VK_DOWN;
						EventVec.push_back(input[1]);
						break;
					case MoveUp:
						input[0].type = INPUT_KEYBOARD;
						input[0].ki.wVk = VK_UP;
						EventVec.push_back(input[0]);
						input[1].type = INPUT_KEYBOARD;
						input[1].ki.wVk = VK_UP;
						EventVec.push_back(input[1]);
						break;
					case MoveLeft:
						input[0].type = INPUT_KEYBOARD;
						input[0].ki.wVk = VK_LEFT;
						EventVec.push_back(input[0]);	
						input[1].type = INPUT_KEYBOARD;
						input[1].ki.wVk = VK_LEFT;
						EventVec.push_back(input[1]);
						break;
					case MoveRight:
						input[0].type = INPUT_KEYBOARD;
						input[0].ki.wVk = VK_RIGHT;
						EventVec.push_back(input[0]);	
						input[1].type = INPUT_KEYBOARD;
						input[1].ki.wVk = VK_RIGHT;
						EventVec.push_back(input[1]);
						break;
					case ClickLeft:
						input[0].type = INPUT_MOUSE;						
						input[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
						EventVec.push_back(input[0]);
						input[1].type = INPUT_MOUSE;						
						input[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
						EventVec.push_back(input[1]);
						break;
					case ClickRight:
						input[0].type = INPUT_MOUSE;						
						input[0].mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
						EventVec.push_back(input[0]);
						input[1].type = INPUT_MOUSE;						
						input[1].mi.dwFlags = MOUSEEVENTF_RIGHTUP;
						EventVec.push_back(input[1]);
						break;
					case ZoomIn:
						input[0].type = INPUT_KEYBOARD;
						input[0].ki.wVk = 0x11;  //ctrl
						EventVec.push_back(input[0]);
						input[1].type = INPUT_KEYBOARD;
						input[1].ki.wVk = 0xBB;  //‘+’
						input[1].ki.dwFlags = 0;
						EventVec.push_back(input[1]);
						input[2].ki.wVk = 0xBB;
						EventVec.push_back(input[2]);
						input[3].ki.wVk = 0xBB;
						EventVec.push_back(input[3]);
						input[1].ki.dwFlags = KEYEVENTF_KEYUP;
						break;
					case ZoomOut:
						input[0].type = INPUT_KEYBOARD;
						input[0].ki.wVk = 0x11;  //ctrl
						EventVec.push_back(input[0]);
						input[1].type = INPUT_KEYBOARD;
						input[1].ki.wVk = 0xBD;  //‘-’
						input[1].ki.dwFlags = 0;
						EventVec.push_back(input[1]);
						input[2].ki.wVk = 0xBD;
						EventVec.push_back(input[2]);
						input[3].ki.wVk = 0xBD;
						EventVec.push_back(input[3]);
						input[1].ki.dwFlags = KEYEVENTF_KEYUP;
						break;
					}			
					Injector::EventInject(EventVec);
					PointVec.clear();
					m_newAction = false;
				}
				else
					continue;
			}
			else          //图片中有光斑
			{
				count++;
				if(count == 1)
				{
					if(tempfingerCount <= BLOB_MAX_COUNT)
						max_fingerCount = tempfingerCount;
				}
				else
				{
					if(max_fingerCount != tempfingerCount)
					{
						PointVec.clear();
						count = 0;
						max_fingerCount = 0;
					}
				}
				if(m_newAction)
					continue;
				GetTime();
				m_newAction = true;
			}
		}
	}
	
	cvReleaseMat(&Mat);
	cvReleaseImage(&src);
	cvReleaseImage(&dst);
	cvDestroyAllWindows();
}

MoveMode Blob::LSM()
{
	std::cout<<"max_fingerCount="<<max_fingerCount<<std::endl;
	if(max_fingerCount == 1)
	{
		double sumX = 0,sumY = 0,Sxx = 0,Sxy = 0,MeanX = 0,MeanY = 0,Corrcoef = 0,aveX,aveY;
		int n = 0;
		int xMove = 0,yMove = 0;    //坐标x，y是增加(1)还是减小(-1)

		std::vector<cv::Point2f>::iterator Point;
	
		for(Point = PointVec.begin();Point != PointVec.end();++Point)
		{
			std::cout<<Point->x<<std::ends<<Point->y<<std::endl;
		}
		for(Point = PointVec.begin();Point != PointVec.end();++Point)
		{
			n++;
			sumX += Point->x;
			sumY += Point->y;

			if(Point + 1 != PointVec.end())
			{
				if(Point->x > (Point+1)->x)
					xMove--;
				else
					xMove++;
				if(Point->y > (Point+1)->y)
					yMove--;
				else
					yMove++;
			}
		}
		aveX = sumX/n;
		aveY = sumY/n;

		for(Point = PointVec.begin();Point != PointVec.end();++Point)
		{
			Sxx += pow(Point->x - aveX,2);
			MeanY += pow(Point->y - aveY,2);
			Sxy += (Point->x - aveX) * (Point->y - aveY);
		}
		Corrcoef = Sxy/(sqrt(Sxx)*sqrt(MeanY));
		m_slope = Sxy/Sxx;
		MeanX = Sxx/n;
		MeanY /= n;

		std::cout<<MeanX<<std::ends<<MeanY<<std::ends<<Corrcoef<<std::endl;
		if(MeanX < BLOB_SHIFT && MeanY < BLOB_SHIFT)
		{
			std::cout<<Rear_time - Fro_time<<std::endl;
			if(Rear_time - Fro_time)
				return ClickRight;
			return ClickLeft;
		}
		if(abs(Corrcoef) < COR_MIN)
			return MInvalid;
	
		////按照slope判定移动反向     
		//std::cout<<m_slope<<std::endl;
		//
		//if(abs(m_slope) > DefSlope)    //上或下
		//{
		//	std::cout<<"m_slope > defSLOPE\n";
		//	if(yMove > 0)
		//	{
		//		std::cout<<"下\n";
		//		return MoveDown;
		//	}
		//	std::cout<<"上\n";
		//	return MoveUp;
		//}
		//else                //左或右
		//{
		//	std::cout<<"m_slope < defSLOPE\n";
		//	if(xMove > 0)
		//	{
		//		std::cout<<"右\n";
		//		return MoveRight;
		//	}
		//	std::cout<<"左\n";
		//	return MoveLeft;
		//}
		//return MInvalid;
	
		//按x、y的方差判断手指移动方向
		if(MeanX > MeanY)     //左或右
		{
			if(xMove > 0)
			{
				std::cout<<"右\n";
				return MoveRight;	
			}
			std::cout<<"左\n";
			return MoveLeft;
		}
		else                    //上或下
		{
			if(yMove > 0)
			{
				std::cout<<"下\n";
				return MoveDown;
			}
			std::cout<<"上\n";
			return MoveUp;
		}
	}
	else if(max_fingerCount == 2)
	{
		double dis1 = 0,dis2;
		int zoom = 0;
		std::vector<cv::Point2f>::iterator Point,temp;
	
		for(Point = PointVec.begin();Point != PointVec.end();++Point)
		{
			temp = Point + 1;
			dis2 = (Point->x - temp->x)*(Point->x - temp->x) + (Point->y - temp->y)*(Point->y - temp->y);

			if(dis2 >= dis1)
				zoom++;
			else
				zoom--;

			dis1 = dis2;
			Point = temp;
		}
		if(zoom > 0)
			return ZoomIn;
		return ZoomOut;
	}
}

void Blob::GetTime()
{
	if(!m_newAction)
		Fro_time = time(NULL);
	else
		Rear_time = time(NULL);
}

void Blob::SetROI(PowerVideoCapture & MyCam,int id)
{
	PointVec.clear();
	Mat = cvCreateMat(2,2,CV_16SC1);
	int i = 0,j = 0,count = 0;

	cvNamedWindow("SetROI");

	IplImage * src=cvCreateImage(cvSize(IMG_WIDTH,IMG_HEIGHT), IPL_DEPTH_8U, 3);
	IplImage * dst=cvCreateImage(cvSize(IMG_WIDTH,IMG_HEIGHT), IPL_DEPTH_8U, 1);
	IplImage * img=cvCreateImage(cvSize(IMG_WIDTH,IMG_HEIGHT),IPL_DEPTH_8U,3);

	while(1)
	{
		
			MyCam.getPixels(id, (unsigned char *)src->imageData, false, true);   
			
			cvFlip(src,NULL,-1); 
			cvCvtColor(src,dst,CV_BGR2GRAY);
			
			cvSmooth(dst,dst,CV_GAUSSIAN);
			
			cvThreshold(dst,dst,100,255,CV_THRESH_BINARY);
			
			IplConvKernel * pKernel = cvCreateStructuringElementEx(15,15,8,8,CV_SHAPE_ELLIPSE,NULL);
			cvErode(dst,dst,pKernel,1);
			cvDilate(dst,dst,pKernel,1);

			char c=cvWaitKey(1);
			if(c==27) 
				break;

			cvShowImage("SetROI",dst);
			
			if(BlobFinder.GetPointVec(dst) == 0)     
				continue;
			else
			{
				count++;
				std::vector<cv::Point2f>::iterator Point = PointVec.begin();

				if(count == 2)
				{
					if(pow((Point->x - CV_MAT_ELEM(*Mat,short,0,0)),2) + pow((Point->y - CV_MAT_ELEM(*Mat,short,0,1)),2)
						< ROI_AREA_MIN)
					{
						std::cout<<pow((Point->x - CV_MAT_ELEM(*Mat,short,0,0)),2) + pow((Point->y - CV_MAT_ELEM(*Mat,short,0,1)),2)<<std::endl;
						count--;
						continue;
					}
				}
				cvCircle(img,cvPoint((int)Point->x,(int)Point->y),15,CV_RGB(255,0,0),30,CV_AA,0);
				cvShowImage("SetROI",img);

				cvWaitKey(1000);

				CV_MAT_ELEM(*Mat,short,i,j) = Point->x;
				j++;
				CV_MAT_ELEM(*Mat,short,i,j) = Point->y;
				i++;
				j = 0;

				if(count == 2)
				{
					CvFont font;
					cvInitFont(&font,CV_FONT_HERSHEY_COMPLEX,1,1,0,2);
					cvPutText(img,"Applied Successfully",cvPoint(IMG_HEIGHT/2-35,IMG_WIDTH/2-70),&font,CV_RGB(255,0,0));
					cvShowImage("SetROI",img);
					cvWaitKey(3000);

					for(i=0;i<2;i++)
					 {
						for(j=0;j<2;j++)
						{
							std::cout<<CV_MAT_ELEM(*Mat,short,i,j)<<std::ends;         
						}
						std::cout<<std::endl;
					 }
					
					CvMemStorage * FileMemStorage = cvCreateMemStorage(0);

					CvFileStorage * fs_write_xml = cvOpenFileStorage("ROI.xml",FileMemStorage,CV_STORAGE_WRITE);
					cvWrite( fs_write_xml, "MAT", Mat, cvAttrList(0,0) );
					//cvSave("ROI.xml",Mat);
					
					cvReleaseFileStorage(&fs_write_xml);
					cvReleaseMemStorage(&FileMemStorage);
					cvReleaseImage(&src);
					cvReleaseImage(&dst);
					cvReleaseImage(&img);
					cvReleaseMat(&Mat);
					cvDestroyWindow("SetROI");
					break;
				}
			}	
	}//END while(1)
}