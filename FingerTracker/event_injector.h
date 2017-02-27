/*
 *    基于激光以及红外摄像的手势触控设备的设计与实现
 *
 *    Inject events into the OS
 *
 *    By Hc
 */
#pragma once

#include <windows.h>
#include <vector>

class Injector
{
public:
	static bool EventInject(std::vector<INPUT> & EventVec);
};