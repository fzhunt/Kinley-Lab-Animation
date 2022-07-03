#include <stdio.h>
#include <iostream>
#include <string.h>
#define _USE_MATH_DEFINES

class Row
{
public:
	double p;
	double r;
	double y;
	double t;
	Row::Row() {
		p = 0;
		r = 0;
		y = 0;
	}
	Row::Row(double time, double pitch, double roll, double yaw) {
		t = time;
		p = pitch;
		r = roll;
		y = yaw;
	}
};