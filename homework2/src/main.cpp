#include <iostream>
#include <time.h>

bool YUV_to_261(char* InFileName, char* OutFileName, int MaxFrame,int ref_frame_rate, int PNumber, int Quant1, int Quant2);

/*************************************************
/*  �������壺
/*  InFileName���������YUV�ļ���
/*  OutFileName������������263�ļ���
/*  MaxFrame���������֡��
/*  ref_frame_rate��֡Ƶ
/*  PNumber��P֡�ĸ���
/*  Quant1��֡��֡��������
/*  Quant2��֡��֡��������
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
