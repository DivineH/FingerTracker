/*
 *    ���ڼ����Լ�������������ƴ����豸�������ʵ��
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