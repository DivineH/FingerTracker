/*
 *    基于激光以及红外摄像的手势触控设备的设计与实现
 *
 *    Device Configuration
 *
 *    By Hc
 */
#pragma once


const int IMG_HEIGHT = 480;
const int IMG_WIDTH  = 640;


const int BLOB_MIN_SIZE = 100;            
const int BLOB_MAX_SIZE = 500*100;
const int ROI_AREA_MIN  = 180000;
const int BLOB_MAX_COUNT = 2;


const int EXPOSUREVAL = -6;

const int BLOB_SHIFT = 500;

const double COR_MIN = 0.5;
const int MAX_TIME = 300;
const int DOUBLECLICK_TIME = 800;