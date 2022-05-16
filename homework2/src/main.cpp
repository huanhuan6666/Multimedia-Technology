#include <iostream>
#include <time.h>

bool YUV_to_261(char* InFileName, char* OutFileName, int MaxFrame,int ref_frame_rate, int PNumber, int Quant1, int Quant2);

/*************************************************
/*  参数定义：
/*  InFileName：待编码的YUV文件名
/*  OutFileName：编码后输出的263文件名
/*  MaxFrame：待编码的帧数
/*  ref_frame_rate：帧频
/*  PNumber：P帧的个数
/*  Quant1：帧内帧的量化阶
/*  Quant2：帧间帧的量化阶
**************************************************/
int main(int argc,char** argv)
{
	long int t1, t2;
	t1 = clock();

	if(argc<8)
	{
		printf("Incorrect parameters, program exit. \n");
		return 0;
	}

	int MaxFrame,ref_frame_rate,PNumber,Quant1,Quant2; 
	MaxFrame = atoi(argv[3]);
	ref_frame_rate = atoi(argv[4]);
	PNumber = atoi(argv[5]);
	Quant1 = atoi(argv[6]);
	Quant2 = atoi(argv[7]);
	printf("Starting to convering velocity file file. \n");
	YUV_to_261(argv[1],argv[2],MaxFrame,ref_frame_rate, PNumber, Quant1, Quant2);
 

	t2 = clock();
    printf("********************************************\n");
    printf("                                            \n");
	printf("**the total time is %.3fS\n", (t2-t1)/1000.);
    printf("                                            \n");
    printf("********************************************\n");
	getchar();
	return 0;
} 
