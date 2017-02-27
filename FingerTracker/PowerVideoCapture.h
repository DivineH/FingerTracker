/*
 *    基于激光以及红外摄像的手势触控设备的设计与实现
 *
 *    PowerVideoCapture
 *
 *    By Hc
 */
#pragma once

#include <iostream>
#include <vector>
#include <cv.h>
#include "cap_dshow.h"

class PowerVideoCapture:public videoInput
{
public:
	/*
    static PowerVideoCapture * CreateCaptureByID( int id,int exposureval);
    static int EnumCaptureDevices();                */
	PowerVideoCapture(int id) ;

public:
    ~PowerVideoCapture() {}
    bool setImageSize(int width, int height);
    bool getImageSize(int & width, int & height);
    //bool setExposureVal(long level);
	//bool setFps(long level);
    //IplImage* retrieveFrame();              
	int GetID(){return deviceID;}
	        
private:
	int deviceID;
};