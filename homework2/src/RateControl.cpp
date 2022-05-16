#include <string.h> 
#include <stdio.h>
#include <stdlib.h>
#include "Global.h"

#include <stdio.h>
#include <math.h>

extern int FrameUpdateQP(int buf, int bits, int frames_left, int QP, int B,float seconds) 
{
	int newQP, dQP;
	float buf_rest, buf_rest_pic;
	
	buf_rest = seconds * B - (float)buf;
	
	newQP = QP;
	
	if (frames_left > 0) 
	{
		buf_rest_pic = buf_rest / (float)frames_left;
		
		dQP = mmax(1,(int)(QP*0.1));
		
		if (bits > buf_rest_pic * 1.15) 
		{
			newQP = mmin(31,QP+dQP);
		}
		else if (bits < buf_rest_pic / 1.15) 
		{
			newQP = mmax(1,QP-dQP);
		}
		else 
		{
		}
	}
	return newQP;
}
