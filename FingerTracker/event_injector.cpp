/*
 *    基于激光以及红外摄像的手势触控设备的设计与实现
 *
 *    Inject events into the OS
 *
 *    By Hc
 */
#include "event_injector.h"

bool Injector::EventInject(std::vector<INPUT> & EventVec)
{
	if (!EventVec.size()) return false;

    INPUT * inputs = new INPUT[EventVec.size()];
          
    for (int pos=0; pos<EventVec.size(); ++pos)
    {
		inputs[pos].type = EventVec[pos].type;
		if(inputs[pos].type == INPUT_MOUSE)
		{
			inputs[pos].mi.dwFlags = EventVec[pos].mi.dwFlags;
			inputs[pos].mi.time = 0;
		}
		else
		{
			inputs[pos].ki.wVk = EventVec[pos].ki.wVk;
			inputs[pos].ki.dwFlags = EventVec[pos].ki.dwFlags;
			inputs[pos].ki.time = 0;
		}
    }

	for(int i=0;i<EventVec.size();i++)
		SendInput(1, inputs+i, sizeof(INPUT));

    delete [] inputs;
	EventVec.clear();

	return true;
}