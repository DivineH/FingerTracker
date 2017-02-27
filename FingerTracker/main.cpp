/*
 *    基于激光以及红外摄像的手势触控设备的设计与实现
 *
 *    Main Function Entry
 *
 *    By Hc
 */
#pragma warning (disable:4996)

#include <cctype>
#include <conio.h>

#include "common.h"
#include "cv_common.h"

using namespace std;

WorkingMode WorkMode = WInvalid;
PowerVideoCapture MyCam(0);
Blob BlobFinder;

int main()
{
	char slt;
	cout<<"是否需要确定手指移动的有效区域以及图像矫正？(Y or N)\n";

	cin>>slt;
	slt = toupper(slt);
	while(slt != 'Y' && slt != 'N')
	{
		system("cls");
		cout<<"是否需要确定手指移动的有效区域以及图像矫正？(Y or N)\n";
		cin>>slt;
		slt = toupper(slt);
	}
	
	int id;
	int count  = MyCam.listDevices(true);

	for(int i = 0;i < count;i++)
		cout<<i<<"、"<<MyCam.getDeviceName(i)<<endl;
	
	cout<<"请选择摄像头ID： ";
	cin>>id;
	if(id<0 || id>count-1)
	{
		cout<<"ID输入错误!"<<endl;
		cvWaitKey();
		return 0;
	}

	cout<<MyCam.getDeviceName(id)<<"正在打开..."<<endl;
	if(!MyCam.setupDevice(id,IMG_WIDTH,IMG_HEIGHT))
	{
		cout<<MyCam.getDeviceName(id)<<"打开失败!";
		cvWaitKey();
		return 0;
	}

	MyCam.setVideoSettingCamera(id,MyCam.propExposure,EXPOSUREVAL);
	cout<<"OK!\n";

	if(slt == 'Y')
		BlobFinder.SetROI(MyCam,MyCam.GetID());
	slt = 'N';
	if(slt == 'N')
	{
		BlobFinder.ShowAction(MyCam,id);
	}
	
	return 0;
}