/*
 *    基于激光以及红外摄像的手势触控设备的设计与实现
 *
 *    Find blob
 *
 *    By Hc
 */
#pragma once
#pragma warning (disable:4244)

#include <vector>
#include "common.h"
#include "PowerVideoCapture.h"
#include "modes.h"

class Blob
{
private:
    cv::Rect    m_box;
    cv::Point2f m_center;
    float       m_area;
	bool        m_newAction;
	bool        cursorMove;
	bool        setROI;
	double      m_slope;
	double      DefSlope;
	clock_t     Fro_time;
	clock_t     Rear_time; 
	int         ScrWidth;  
	int         ScrHeight;
	int         max_fingerCount;
	int         testImgCount;
	CvMat * Mat;
	CvMat * warp_mat;
	std::vector<cv::Point2f>  PointVec;
	std::vector<cv::Point2f>  TempPointVec;
public:
    Blob(const cv::Rect & box, const cv::Point2f & center, float area)
        :   m_box(box),m_center(center),m_area(area) {}

    Blob()
		:m_box(0,0,0,0),m_center(0,0),m_area(0),m_newAction(false),cursorMove(0),setROI(0),m_slope(0),Fro_time(0),Rear_time(0),max_fingerCount(1),testImgCount(0)
    {}

	int GetPointVec(IplImage * src,int flag=0); 
	void ShowAction(PowerVideoCapture & MyCam,int id);
	MoveMode LSM();
	void GetTime();
	void SetROI(PowerVideoCapture & MyCam,int id);         
};