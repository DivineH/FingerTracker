/*
 *    基于激光以及红外摄像的手势触控设备的设计与实现
 *
 *    PowerVideoCapture manage cameras
 *
 *    By Hc
 */
#include "cv_common.h"
#include "dev_conf.h"
#include "PowerVideoCapture.h"
#include "cap_dshow.h"

videoInput _VI;

PowerVideoCapture::PowerVideoCapture(int id)
	:deviceID(id)
{
	
}
/*
int PowerVideoCapture::EnumCaptureDevices()
{
	int id;
	int count  = _VI.listDevices(true);

	for(int i = 0;i < count;i++)
		std::cout<<i<<"、"<<_VI.getDeviceName(i)<<std::endl;
	
	std::cout<<"请选择摄像头ID： ";
	std::cin>>id;

	return id;
}

PowerVideoCapture * PowerVideoCapture::CreateCaptureByID(int id,int exposureval)
{
	int sumCam = videoInput::listDevices(true);
	if(id<0 || id>sumCam)
	{
		std::cout<<"ID错误!\n";
		return NULL;
	}
	else
	{
		std::cout<<_VI.getDeviceName(id)<<"正在打开..."<<std::endl;
		PowerVideoCapture * newborn = new PowerVideoCapture(id);
		if (!_VI.setupDevice(id,IMG_WIDTH,IMG_HEIGHT))
		{
			std::cout<<"摄像头打开失败!\n";
			delete newborn;
			return NULL;
		}
		_VI.setVideoSettingCamera(id,_VI.propExposure,exposureval);
		return newborn;
	}
}
*/
bool PowerVideoCapture::setImageSize(int width, int height)
{
	if( width > 0 && height > 0 )
    {
        if( width != _VI.getWidth(deviceID) || height != _VI.getHeight(deviceID) )
        {
            _VI.stopDevice(deviceID);
            _VI.setupDevice(deviceID, width, height);
        }
        return _VI.isDeviceSetup(deviceID);
    }
    return false;
}

bool PowerVideoCapture::getImageSize(int & width, int & height)
{
	width = _VI.getWidth(deviceID);
    height = _VI.getHeight(deviceID);
    return true;
}
/*
bool PowerVideoCapture::setExposureVal(long level)
{
	return _VI.setVideoSettingCamera(deviceID, _VI.propExposure, level, 2, false);
}
	
bool PowerVideoCapture::setFps(long level)
{
	return _VI.setVideoSettingCamera(deviceID, _VI.propRoll, level, 2, false);
}

IplImage * PowerVideoCapture::retrieveFrame()
{
	if( !catched_frame || _VI.getWidth(deviceID) != catched_frame->width || _VI.getHeight(deviceID) != catched_frame->height )
    {
        if (catched_frame)
            cvReleaseImage( &catched_frame );
        int w = _VI.getWidth(deviceID), h = _VI.getHeight(deviceID);
        catched_frame = cvCreateImage( cvSize(w,h), 8, 3 );
    }


    _VI.getPixels( deviceID, (uchar*)catched_frame->imageData, false, true );
    return catched_frame;
}
*/