/*
 *    ���ڼ����Լ�������������ƴ����豸�������ʵ��
 *
 *    Modes in the program
 *
 *    By Hc
 */
#pragma once

enum WorkingMode
{
	WInvalid = 0,
	Normal  = 1,
	Rectify = 2,
};

enum MoveMode
{
	MInvalid   =  0,
	MoveUp     =  1,
	MoveDown   =  2,
	MoveLeft   =  3,
	MoveRight  =  4,
	ClickLeft  =  5,
	ClickRight =  6,
	ZoomIn     =  7,
	ZoomOut    =  8,
	DoubleClick = 9,
};