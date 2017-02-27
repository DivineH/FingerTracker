/*
 *    ���ڼ����Լ�������������ƴ����豸�������ʵ��
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
	cout<<"�Ƿ���Ҫȷ����ָ�ƶ�����Ч�����Լ�ͼ�������(Y or N)\n";

	cin>>slt;
	slt = toupper(slt);
	while(slt != 'Y' && slt != 'N')
	{
		system("cls");
		cout<<"�Ƿ���Ҫȷ����ָ�ƶ�����Ч�����Լ�ͼ�������(Y or N)\n";
		cin>>slt;
		slt = toupper(slt);
	}
	
	int id;
	int count  = MyCam.listDevices(true);

	for(int i = 0;i < count;i++)
		cout<<i<<"��"<<MyCam.getDeviceName(i)<<endl;
	
	cout<<"��ѡ������ͷID�� ";
	cin>>id;
	if(id<0 || id>count-1)
	{
		cout<<"ID�������!"<<endl;
		cvWaitKey();
		return 0;
	}

	cout<<MyCam.getDeviceName(id)<<"���ڴ�..."<<endl;
	if(!MyCam.setupDevice(id,IMG_WIDTH,IMG_HEIGHT))
	{
		cout<<MyCam.getDeviceName(id)<<"��ʧ��!";
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