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

int Blob::GetPointVec(IplImage * src,int flag)
{
	/*if(!m_newAction)
	{
		PointVec.clear();
		TempPointVec.clear();
	}*/
	int count;
	CvMemStorage*	mem_storage	= NULL;
	CvMoments myMoments;

	if( mem_storage==NULL ) mem_storage = cvCreateMemStorage(0);
	else cvClearMemStorage(mem_storage);

	CvSeq* contour_list = 0;
	
	count = cvFindContours(src,mem_storage,&contour_list, sizeof(CvContour),
			CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE);

	if(count == 0 || count > 2)
	{
		cvReleaseMemStorage(&mem_storage);
		return 0;
	}
	if(count == 2)
		max_fingerCount = 2;

	int temp = 0;
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
				//std::cout<<"获得点\n";
				
				if(!flag)
				{
					//透视变换
					/*float x = (obj.m_center.x*CV_MAT_ELEM(*warp_mat,float,0,0) + obj.m_center.y*CV_MAT_ELEM(*warp_mat,float,1,0) + CV_MAT_ELEM(*warp_mat,float,2,0));
					float y = (obj.m_center.x*CV_MAT_ELEM(*warp_mat,float,0,1) + obj.m_center.y*CV_MAT_ELEM(*warp_mat,float,1,1) + CV_MAT_ELEM(*warp_mat,float,2,1));
					float z = (obj.m_center.x*CV_MAT_ELEM(*warp_mat,float,0,2) + obj.m_center.y*CV_MAT_ELEM(*warp_mat,float,1,2) + CV_MAT_ELEM(*warp_mat,float,2,2));
*/
					float x = (obj.m_center.x*CV_MAT_ELEM(*warp_mat,float,0,0) + obj.m_center.y*CV_MAT_ELEM(*warp_mat,float,0,1) + CV_MAT_ELEM(*warp_mat,float,0,2));
					float y = (obj.m_center.x*CV_MAT_ELEM(*warp_mat,float,1,0) + obj.m_center.y*CV_MAT_ELEM(*warp_mat,float,1,1) + CV_MAT_ELEM(*warp_mat,float,1,2));
					float z = (obj.m_center.x*CV_MAT_ELEM(*warp_mat,float,2,0) + obj.m_center.y*CV_MAT_ELEM(*warp_mat,float,2,1) + CV_MAT_ELEM(*warp_mat,float,2,2));

					obj.m_center.x = x/z;
					obj.m_center.y = y/z;

					if(!(obj.m_center.x >= 0 && obj.m_center.x <= ScrWidth
						&& obj.m_center.y >= 0 && obj.m_center.y <= ScrHeight))
					{
						//std::cout<<"出界\n";
						if (isHole)
							c = c->h_next;//one_hole->h_next is another_hole
						else
							c = c->v_next;//one_contour->h_next is one_hole
						isHole = true;
						continue;
					}
				}

				if(temp == 0)
				{
					//std::cout<<"保存点\n";
					PointVec.push_back(obj.m_center);
					if(cursorMove)
						SetCursorPos((int)obj.m_center.x,(int)obj.m_center.y);
					temp++;
				}
				else if(temp == 1)
					TempPointVec.push_back(obj.m_center);
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
	bool originImg = false;
	std::vector<INPUT> EventVec;
	INPUT input[4];
	Mat = (CvMat *)cvLoad("ROI.xml");
	warp_mat =  (CvMat *)cvLoad("Warp_Mat.xml");
	CvMat * _cam_map_x = (CvMat *)cvLoad("cam_map_x.xml");
    CvMat * _cam_map_y = (CvMat *)cvLoad("cam_map_y.xml");

	HDC hdc=GetDC(NULL);   //获得屏幕设备描述表句柄   
  	ScrWidth=GetDeviceCaps(hdc,HORZRES);   //获取屏幕水平分辨率   
  	ScrHeight=GetDeviceCaps(hdc,VERTRES);     //获取屏幕垂直分辨率  
	ReleaseDC(NULL,hdc);   //释放屏幕设备描述表

	/*for(i=0;i<2;i++)
	{
		for(j=0;j<2;j++)
		{
			std::cout<<CV_MAT_ELEM(*Mat,short,i,j)<<std::ends;         
		}
		std::cout<<std::endl;
	}*/
	DefSlope = (double)(CV_MAT_ELEM(*Mat,short,1,1) - CV_MAT_ELEM(*Mat,short,0,1))/
		       (double)(CV_MAT_ELEM(*Mat,short,1,0) - CV_MAT_ELEM(*Mat,short,0,0));
	//std::cout<<DefSlope;

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

	cvNamedWindow("Dst0");
	cvNamedWindow("SrcFingerTrace");
	cvNamedWindow("DstFingerTrace");

	
	IplImage * src0=cvCreateImage(cvSize(IMG_WIDTH,IMG_HEIGHT), IPL_DEPTH_8U, 3);
	IplImage * dst0=cvCreateImage(cvSize(IMG_WIDTH,IMG_HEIGHT), IPL_DEPTH_8U, 1);
	IplImage * src=cvCreateImage(cvSize(IMG_WIDTH,IMG_HEIGHT), IPL_DEPTH_8U, 3);
	IplImage * dst=cvCreateImage(cvSize(IMG_WIDTH,IMG_HEIGHT), IPL_DEPTH_8U, 1);
	IplImage * clone=cvCreateImage(cvSize(IMG_WIDTH,IMG_HEIGHT), IPL_DEPTH_8U, 3);

	int wasteImg = 0;

	while(1)
	{
		if(MyCam.isFrameNew(id))       
		{
			if(originImg)
				MyCam.getPixels(id, (unsigned char *)src->imageData, false, true); 
			else
			{
				if(setROI)
				{
					std::cout<<"setROi\n";
					wasteImg++;
					if(wasteImg <= 5)
					{
						MyCam.getPixels(id, (unsigned char *)src0->imageData, false, true);
						continue;
					}
				}
				std::cout<<"截图\n";
				MyCam.getPixels(id, (unsigned char *)src0->imageData, false, true);
				cvFlip(src0,NULL,-1); 
				cvCvtColor(src0,dst0,CV_BGR2GRAY);
				originImg = true;
				continue;
			}
			 
			cvFlip(src,NULL,-1); 
			cvCvtColor(src,dst,CV_BGR2GRAY);

			cvAbsDiff(dst0,dst,dst);
			cvThreshold(dst,dst,200,255,CV_THRESH_BINARY);		
			cvSmooth(dst,dst,CV_GAUSSIAN);
			
			IplConvKernel * pKernel = cvCreateStructuringElementEx(15,15,8,8,CV_SHAPE_ELLIPSE,NULL);
			cvErode(dst,dst,pKernel,1);
			cvDilate(dst,dst,pKernel,1);

			char c=cvWaitKey(1);
			if(c==27) 
				break;

			/*cvSetImageROI(src,cvRect(CV_MAT_ELEM(*Mat,short,0,0),CV_MAT_ELEM(*Mat,short,0,1),
				                     CV_MAT_ELEM(*Mat,short,1,0),CV_MAT_ELEM(*Mat,short,1,1)));

			cvSetImageROI(dst,cvRect(CV_MAT_ELEM(*Mat,short,0,0),CV_MAT_ELEM(*Mat,short,0,1),
				                     CV_MAT_ELEM(*Mat,short,1,0),CV_MAT_ELEM(*Mat,short,1,1)));
            */
			cvRemap(src, clone, _cam_map_x, _cam_map_y);
			cvShowImage("Dst0",dst0);
			cvShowImage("SrcFingerTrace",src);
			//cvShowImage("DstFingerTrace",clone);
			cvShowImage("DstFingerTrace",dst);
			if(BlobFinder.GetPointVec(dst,0) == 0)    
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
						input[0].ki.dwFlags = 0;
						EventVec.push_back(input[0]);
						input[1].type = INPUT_KEYBOARD;
						input[1].ki.wVk = 0xBB;  //‘+’
						input[1].ki.dwFlags = 0;
						EventVec.push_back(input[1]);
						input[2].ki.wVk = 0xBB;
						input[2].ki.dwFlags = KEYEVENTF_KEYUP;
						EventVec.push_back(input[2]);
						input[3].ki.wVk = 0x11;
						input[3].ki.dwFlags = KEYEVENTF_KEYUP;
						EventVec.push_back(input[3]);
						break;
					case ZoomOut:
						input[0].type = INPUT_KEYBOARD;
						input[0].ki.wVk = 0x11;  //ctrl
						input[0].ki.dwFlags = 0;
						EventVec.push_back(input[0]);
						input[1].type = INPUT_KEYBOARD;
						input[1].ki.wVk = 0xBD;  //‘-’
						input[1].ki.dwFlags = 0;
						EventVec.push_back(input[1]);
						input[2].ki.wVk = 0xBD;
						input[2].ki.dwFlags = KEYEVENTF_KEYUP;
						EventVec.push_back(input[2]);
						input[3].ki.wVk = 0x11;
						input[3].ki.dwFlags = KEYEVENTF_KEYUP;
						EventVec.push_back(input[3]);
						break;
					}			
					Injector::EventInject(EventVec);
					std::cout<<"cursorMove="<<cursorMove<<"\n";
					testImgCount = 0;
					cursorMove = 0;
					PointVec.clear();
					TempPointVec.clear();
					m_newAction = false;
				}
				else
					continue;
			}
			else          //图片中有光斑
			{
				testImgCount++;
				//先检测鼠标是否要移动
				if(testImgCount == 5)
				{
					GetTime();
					switch(LSM())
					{
					case MoveUp:
					case MoveDown:
					case MoveLeft:
					case MoveRight:
						cursorMove = 1;
						break;
					case MInvalid:
					case ClickLeft:
					case ClickRight:
					case ZoomIn:
					case ZoomOut:
						cursorMove = 0;
						break;
					}
					PointVec.clear();
					TempPointVec.clear();
					m_newAction = false;
				}
				if(m_newAction)
					continue;
				GetTime();
				m_newAction = true;
			}
		}
	}
	
	cvReleaseMat(&Mat);
	cvReleaseMat(&_cam_map_x);
	cvReleaseMat(&_cam_map_y);
	cvReleaseImage(&src);
	cvReleaseImage(&dst);
	cvDestroyAllWindows();
}

MoveMode Blob::LSM()
{
	std::cout<<max_fingerCount<<"\n";
	if(max_fingerCount == 1)
	{
		
		double sumX = 0,sumY = 0,Sxx = 0,Sxy = 0,MeanX = 0,MeanY = 0,Corrcoef = 0,aveX,aveY;
		int n = 0;
		int xMove = 0,yMove = 0;    //坐标x，y是增加(1)还是减小(-1)

		std::vector<cv::Point2f>::iterator Point;
	
		for(Point = PointVec.begin();Point != PointVec.end();++Point)
		{
			//std::cout<<"LSM\n";
			std::cout<<Point->x<<' '<<Point->y<<std::endl;
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

		std::cout<<MeanX<<' '<<MeanY<<' '<<Corrcoef<<std::endl;
		if(MeanX < BLOB_SHIFT && MeanY < BLOB_SHIFT)
		{
			std::cout<<"Rear_time - Fro_time = "<<(Rear_time - Fro_time)<<std::endl;
			if(Rear_time - Fro_time <= MAX_TIME)
				return ClickLeft;
			return ClickRight;
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
	else
	{
		max_fingerCount = 1;
		double dis1,dis2;
		int zoom = 0;
		std::vector<cv::Point2f>::iterator Point1,Point2;

		for(Point1 = TempPointVec.begin();Point1 != TempPointVec.end();++Point1)
		{
			std::cout<<Point1->x<<' '<<Point1->y<<std::endl;
		}

		Point1 = TempPointVec.begin();
		Point2 = PointVec.begin();

		if(Point1 != TempPointVec.end() && Point2 != PointVec.end())
		{
			dis1 = pow((Point1->x - Point2->x),2) + pow((Point1->y - Point2->y),2);
		}
	
		for(;Point1 != TempPointVec.end() && Point2 != PointVec.end();++Point1,++Point2)
		{
			dis2 = pow((Point1->x - Point2->x),2) + pow((Point1->y - Point2->y),2);
			if(dis2 > dis1)
				zoom++;
			else if(dis2 < dis1)
				zoom--;
			dis1 = dis2;
		}
		PointVec.clear();
		TempPointVec.clear();
		if(zoom > 0)
			return ZoomIn;
		return ZoomOut;
	}
}

void Blob::GetTime()
{
	if(!m_newAction)
		Fro_time = clock();
	else
		Rear_time = clock();
}

void Blob::SetROI(PowerVideoCapture & MyCam,int id)
{
	PointVec.clear();
	setROI = 1;
	Mat = cvCreateMat(2,2,CV_16SC1);
	CvPoint2D32f srcQuad[4],dstQuad[4];
	warp_mat = cvCreateMat(3,3,CV_32FC1);
	int i = 0,j = 0,count = 0;

	HDC hdc=GetDC(NULL);   //获得屏幕设备描述表句柄   
  	ScrWidth=GetDeviceCaps(hdc,HORZRES);   //获取屏幕水平分辨率   
  	ScrHeight=GetDeviceCaps(hdc,VERTRES);     //获取屏幕垂直分辨率  
	ReleaseDC(NULL,hdc);   //释放屏幕设备描述表

	dstQuad[0].x = 0;
	dstQuad[0].y = 0;
	dstQuad[1].x = ScrWidth;
	dstQuad[1].y = 0;
	dstQuad[2].x = 0;
	dstQuad[2].y = ScrHeight;
	dstQuad[3].x = ScrWidth;
	dstQuad[3].y = ScrHeight;
  	
	for(i=0;i<4;i++)
		std::cout<<dstQuad[i].x<<' '<<dstQuad[i].y<<std::endl;
	i=0;


	cvNamedWindow("SetROI");
	cvNamedWindow("Dst");

	IplImage * src0=cvCreateImage(cvSize(IMG_WIDTH,IMG_HEIGHT), IPL_DEPTH_8U, 3);
	IplImage * dst0=cvCreateImage(cvSize(IMG_WIDTH,IMG_HEIGHT), IPL_DEPTH_8U, 1);
	IplImage * src=cvCreateImage(cvSize(IMG_WIDTH,IMG_HEIGHT), IPL_DEPTH_8U, 3);
	IplImage * dst=cvCreateImage(cvSize(IMG_WIDTH,IMG_HEIGHT), IPL_DEPTH_8U, 1);
	IplImage * img=cvCreateImage(cvSize(IMG_WIDTH,IMG_HEIGHT),IPL_DEPTH_8U,3);

	bool originImg = false;

	while(1)
	{
		if(MyCam.isFrameNew(id))       
		{
			if(originImg)
				MyCam.getPixels(id, (unsigned char *)src->imageData, false, true); 
			else
			{
				MyCam.getPixels(id, (unsigned char *)src0->imageData, false, true);
				cvFlip(src0,NULL,-1); 
				cvCvtColor(src0,dst0,CV_BGR2GRAY);
				originImg = true;
				continue;
			}

			cvFlip(src,NULL,-1); 
			cvCvtColor(src,dst,CV_BGR2GRAY);

			cvAbsDiff(dst0,dst,dst);
			
			cvThreshold(dst,dst,150,255,CV_THRESH_BINARY);
			cvShowImage("Dst",dst);
			IplConvKernel * pKernel = cvCreateStructuringElementEx(15,15,8,8,CV_SHAPE_ELLIPSE,NULL);
			cvErode(dst,dst,pKernel,1);
			cvDilate(dst,dst,pKernel,1);

			char c=cvWaitKey(1);
			if(c==27) 
			{
				cvReleaseImage(&src);
				cvReleaseImage(&dst);
				cvReleaseImage(&img);
				cvReleaseMat(&Mat);
				cvDestroyWindow("SetROI");
				cvDestroyWindow("Dst");
				break;
			}

			cvShowImage("SetROI",src);
			
			
			if(BlobFinder.GetPointVec(dst,1) == 0)     
				continue;
			else
			{
				std::vector<cv::Point2f>::iterator Point = PointVec.begin();
				if(Point == PointVec.end())
					continue;
				if(count > 0)
				{
					int len = abs(Point->x - srcQuad[count-1].x) + abs(Point->y - srcQuad[count-1].y);
					std::cout<<len<<"\n";
					std::cout<<Point->x<<' '<<Point->y<<"\n";
					std::cout<<srcQuad[count-1].x<<' '<<srcQuad[count-1].y<<"\n";
					if(len < 100)
					{
						std::cout<<"距离\n";
						PointVec.clear();
						continue;
					}
				}

				cvCircle(img,cvPoint((int)Point->x,(int)Point->y),10,CV_RGB(255,0,0),30,CV_AA,0);
				cvShowImage("SetROI",img);

				cvWaitKey(1000);

				srcQuad[count].x = Point->x; 
				srcQuad[count].y = Point->y; 
				count++;

				if(count == 4)
				{
					std::cout<<count;
					CvFont font;
					cvInitFont(&font,CV_FONT_HERSHEY_COMPLEX,1,1,0,2);
					cvPutText(img,"Applied Successfully",cvPoint(IMG_HEIGHT/2-35,IMG_WIDTH/2-70),&font,CV_RGB(255,0,0));
					cvShowImage("SetROI",img);
					cvWaitKey(3000);

					float xMin = srcQuad[0].x,yMin = srcQuad[0].y;
					float xMax = srcQuad[0].x,yMax = srcQuad[0].y;

					for(i=0;i<4;i++)
					 {
						std::cout<<srcQuad[i].x<<' '<<srcQuad[i].y;
						std::cout<<std::endl;
						if(xMin > srcQuad[i].x)
							xMin = srcQuad[i].x;
						if(yMin > srcQuad[i].y)
							yMin = srcQuad[i].y;

						if(xMax < srcQuad[i].x)
							xMax = srcQuad[i].x;
						if(yMax < srcQuad[i].y)
							yMax = srcQuad[i].y;
					 }

					CV_MAT_ELEM(*Mat,short,0,0) = (short)xMin;
					CV_MAT_ELEM(*Mat,short,0,1) = (short)yMin;
					CV_MAT_ELEM(*Mat,short,1,0) = (short)xMax;
					CV_MAT_ELEM(*Mat,short,1,1) = (short)yMax;

					cvGetPerspectiveTransform(srcQuad,dstQuad,warp_mat);
					std::cout<<"*********\n";
					for(int i=0;i<4;i++)
						std::cout<<srcQuad[i].x<<' '<<srcQuad[i].y<<'\n';
					cvSave("Warp_Mat.xml",warp_mat);
					cvSave("ROI.xml",Mat);
					
					cvReleaseImage(&src);
					cvReleaseImage(&dst);
					cvReleaseImage(&img);
					cvReleaseMat(&Mat);
					cvDestroyWindow("SetROI");
					cvDestroyWindow("Dst");
					break;
				}
			}
			}
	}//END while(1)
}