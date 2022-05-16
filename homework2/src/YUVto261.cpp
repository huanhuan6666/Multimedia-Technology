#define  _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 
#include "Global.h"

FILE* pOutFile=NULL;
int long_vectors=0;
int mv_outside_frame=0;
int pels,lines,cpels;

bool YUV_to_261(char* InFileName, char* OutFileName, int MaxFrame, int ref_frame_rate, int PNumber, int Quant1, int Quant2)
{ 

    PictImage *stored_image = NULL;
    int start_rate_control;
	int targetrate=1500000;//码率控制
	char symbol[10];
	strcpy(symbol, "-\\|/|");

	unsigned char *pImageData;
    pImageData=new unsigned char[352*288*3/2];
	FILE* pInFile;
	pInFile=fopen(InFileName,"rb");
	if(pInFile==NULL)
	{
		printf("Can not open the yuv file!\n");
		return false;
	}
	pOutFile=fopen(OutFileName,"wb");
	if(pOutFile==NULL)
	{
		printf("Can not open the output file!\n");
		return false;
	}

	PictImage *prev_image = NULL;
    PictImage *curr_image = NULL;
    PictImage *curr_recon = NULL;
    PictImage *prev_recon = NULL;
	
	int frame_no,first_frameskip=0;  
    int start=1;//从第1帧到第MaxFrame帧
    int orig_frameskip=1;//输入图像原始偏移
    int frameskip=1;//非发送帧
		
	Pict *pic = (Pict *)malloc(sizeof(Pict));
	Bits *bits = (Bits *)malloc(sizeof(Bits));
	//率控制
	Bits *total_bits = (Bits *)malloc(sizeof(Bits));
    Bits *intra_bits = (Bits *)malloc(sizeof(Bits));
	
	pic->BQUANT = DEF_BQUANT;
    pic->seek_dist = DEF_SEEK_DIST;
    pic->use_gobsync = DEF_INSERT_SYNC;//=0
    pic->PB = 0;
    pic->TR = 0;
    pic->QP_mean = (float)0.0;
    pic->unrestricted_mv_mode = 0;
	pic->picture_coding_type =0; // PCT_INTRA;
    pic->source_format = SF_CIF;
	switch (pic->source_format) {
    case (SF_SQCIF):
		fprintf(stdout, "Encoding format: SQCIF (128x96)\n");
		pels = 128;
		lines = 96;
		break;
    case (SF_QCIF):
		fprintf(stdout, "Encoding format: QCIF (176x144)\n");
		pels = 176;
		lines = 144;
		break;
    case (SF_CIF):
		fprintf(stdout, "Encoding format: CIF (352x288)\n");
		pels = 352;
		lines = 288;
		break;
    case (SF_4CIF):
		fprintf(stdout, "Encoding format: 4CIF (704x576)\n");
		pels = 704;
		lines = 576;
		break;
    case (SF_16CIF):
		fprintf(stdout, "Encoding format: 16CIF (1408x1152)\n");
		pels = 1408;
		lines = 1152;
		break;
    default:
		fprintf(stderr,"Illegal coding format\n");
		exit(-1);
	}
    cpels = pels/2;
	curr_recon = InitImage(pels*lines);
	
	//率控制
	start_rate_control = 0;//DEF_START_RATE_CONTROL;

	

	initbits ();
	
	fseek(pInFile,0, SEEK_SET);
	fread(pImageData,1,352*288*3/2,pInFile);
	pic->QUANT=Quant1;
	curr_image = FillImage(pImageData);
	curr_recon = CodeOneIntra(curr_image, Quant1, bits, pic);
	bits->header += alignbits (); // pictures shall be byte aligned 
		
	//率控制
	AddBitsPicture(bits);
	memcpy(intra_bits,bits,sizeof(Bits));
	ZeroBits(total_bits);
	//* number of seconds to encode *
	int chosen_frameskip=1;//jwp
	//* compute first frameskip *
	float seconds = (MaxFrame - start + chosen_frameskip) * orig_frameskip/ ref_frame_rate;
	first_frameskip = chosen_frameskip;
	frameskip = chosen_frameskip;

		

	//第二帧 
	pic->QUANT=Quant2;
	for(frame_no=start+first_frameskip;frame_no<=MaxFrame;frame_no+=frameskip)
	{
		printf("%c  %3.1f%% finished!\r", symbol[frame_no%4], float(frame_no)/MaxFrame*100);
		fseek(pInFile,(frame_no-1)*352*288*3/2, SEEK_SET);
		fread(pImageData,1,352*288*3/2,pInFile);
		if(pImageData==NULL)
			return false;
			
		pic->picture_coding_type =1; // PCT_INTER;
			
		pic->QUANT=Quant2;
			
		prev_image=curr_image;
		prev_recon=curr_recon;
		curr_image = FillImage(pImageData);
		pic->TR+=(((frameskip+(pic->PB?98:0))*orig_frameskip)%256);
		
		fseek(pOutFile,0,SEEK_END);
		if(((frame_no-1)%PNumber)==0)
		{
			pic->picture_coding_type =0; // PCT_INTRA;
			pic->QUANT=Quant1;
			curr_recon = CodeOneIntra(curr_image, Quant1, bits, pic);
			AddBitsPicture(bits);
			memcpy(intra_bits,bits,sizeof(Bits));
				
		}
		else
		{ 
			CodeOneInter(prev_image,curr_image,prev_recon,curr_recon,pic->QUANT,frameskip,bits,pic);
			AddBitsPicture(bits); 
		}
		bits->header += alignbits (); //* pictures shall be byte aligned *
			
		//率控制
			
		AddBits(total_bits, bits);
		//* Aim for the targetrate with a once per frame rate control scheme *
		if (targetrate != 0)
			if (frame_no - start > (MaxFrame - start) * start_rate_control/100.0)					
				pic->QUANT = FrameUpdateQP(total_bits->total + intra_bits->total, bits->total / (pic->PB?2:1), (MaxFrame-frame_no) / chosen_frameskip, pic->QUANT, targetrate, seconds);
		frameskip = chosen_frameskip;

	}//end for frame_no
		
	delete prev_image;
	delete prev_recon;
	delete curr_image;
	curr_recon=NULL;
	free(bits);
	free(pic);
	delete pImageData;
	fclose(pInFile);
	fclose(pOutFile);
	return true;
}