#ifndef SENSOR_TYPE_H
#define SENSOR_TYPE_H

typedef	short           int16;
typedef	unsigned short	uint16;

typedef union _int16vec{ //Three-dimensional vector constructed of signed 16 bits fixed point numbers
	struct {
		int16	x;
		int16	y;
		int16	z;
	}u;
	int16	v[3];
} int16vec;

#endif
