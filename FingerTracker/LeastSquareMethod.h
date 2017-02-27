/*
 *    基于激光以及红外摄像的手势触控设备的设计与实现
 *
 *    Least square method
 *
 *    By Hc
 */
#pragma once

#include <windows.h>
#include <math.h>
#include "cv_common.h"
#include "common.h"
#include "modes.h"

MoveMode LSM(std::vector<cv::Point2f> PointVec)
{
	double sumX = 0,sumY = 0,Sxx = 0,Sxy = 0,aveX,aveY,slope;
	int n = 0;
	int xMove = 0,yMove = 0;    //坐标x，y是增加(1)还是减小(-1)

	std::vector<cv::Point2f>::iterator Point;
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
		Sxy += (Point->x - aveX) * (Point->y - aveY);
	}
	
	slope = Sxy/Sxx;
	return MoveDown;
}