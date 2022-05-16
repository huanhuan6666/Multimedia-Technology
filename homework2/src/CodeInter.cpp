#include <string.h> 
#include <stdio.h>
#include <stdlib.h>
#include "Global.h"
/**********************************************************************
*
*	Name:        CodeOneInter
*	Description:	code one image normally or two images
*                      as a PB-frame (CodeTwoPB and CodeOnePred merged)
*	
*	Input:        pointer to image, prev_image, prev_recon, Q
*        
*	Returns:	pointer to reconstructed image
*	Side effects:	memory is allocated to recon image
*
************************************************************************/
void CodeOneInter(PictImage *prev,PictImage *curr,
				  PictImage *pr,PictImage *curr_recon,
				  int QP, int frameskip, Bits *bits, Pict *pic)
{
	ZeroBits(bits);
	unsigned char *prev_ipol=NULL,*pi=NULL,*orig_lum=NULL;
	PictImage *prev_recon=NULL;
	MotionVector *MV[6][MBR+1][MBC+2]; 
	MotionVector ZERO = {0,0,0,0,0};
	int i,j,k;
	int newgob,Mode;
	int *qcoeff_P;
	int CBP, CBPB=0;
    MB_Structure *recon_data_P; 
    MB_Structure *diff; 
	
	/* ���������Ʊ��� */
	float QP_cumulative = (float)0.0;
	int abs_mb_num = 0, QuantChangePostponed = 0;
	int QP_new, QP_prev, dquant, QP_xmitted=QP;
	QP_new = QP_xmitted = QP_prev = QP; /* ���ƾ�QP */
	
	/* ͼ���ֵ */
	if(!mv_outside_frame)
	{
		pi = InterpolateImage(pr->lum,pels,lines);
		prev_ipol = pi;
		prev_recon = pr;
		orig_lum = prev->lum;
	}
	
	/* Ϊÿ���������MV */
	for (i = 1; i < (pels>>4)+1; i++) 
	{
		for (k = 0; k < 6; k++) 
		{
			MV[k][0][i] = (MotionVector *)malloc(sizeof(MotionVector));
			MarkVec(MV[k][0][i]);
		}
		MV[0][0][i]->Mode = MODE_INTRA;
	}
	/* ����ͼ��߽��MV��Ϊ0 */
	for (i = 0; i < (lines>>4)+1; i++) 
	{
		for (k = 0; k < 6; k++) 
		{
			MV[k][i][0] = (MotionVector *)malloc(sizeof(MotionVector));
			ZeroVec(MV[k][i][0]);
			MV[k][i][(pels>>4)+1] = (MotionVector *)malloc(sizeof(MotionVector));
			ZeroVec(MV[k][i][(pels>>4)+1]);
		}
		MV[0][i][0]->Mode = MODE_INTRA;
		MV[0][i][(pels>>4)+1]->Mode = MODE_INTRA;
	}
	
	/* �����Ͱ������˶���ֵ */
	MotionEstimatePicture(curr->lum, prev_recon->lum, prev_ipol, pic->seek_dist, MV, pic->use_gobsync);
	
	QP_new = QP_xmitted = QP_prev = QP; /* ���ƾ� QP */

	dquant = 0; 
	for ( j = 0; j < lines/MB_SIZE; j++) 
	{
	
		newgob = 0;
		if (j == 0) 
		{
			pic->QUANT = QP;
			bits->header += CountBitsPicture(pic);//����ͼ�������
		}
		else if (pic->use_gobsync && j%pic->use_gobsync == 0) 
		{
			bits->header += CountBitsSlice(j,QP); //���GOBͬ��ͷ
			newgob = 1;
		}
		for ( i = 0; i < pels/MB_SIZE; i++) 
		{
			/* ����dquant */
			dquant = QP_new - QP_prev;
			if (dquant != 0 && i != 0 && MV[0][j+1][i+1]->Mode == MODE_INTER4V) 
			{
				dquant = 0;
				QP_xmitted = QP_prev;
				QuantChangePostponed = 1;
			}
			else 
			{
				QP_xmitted = QP_new;
				QuantChangePostponed = 0;
			}
			if (dquant > 2)  
			{ 
				dquant =  2; 
				QP_xmitted = QP_prev + dquant;
			}
			if (dquant < -2) 
			{ 
				dquant = -2;
				QP_xmitted = QP_prev + dquant;
			}
			
			pic->DQUANT = dquant;
			
			/* ��dquant != 0���޸ĺ������ (���� MODE_INTER -> MODE_INTER_Q) */
			Mode = ModifyMode(MV[0][j+1][i+1]->Mode,pic->DQUANT);
			MV[0][j+1][i+1]->Mode = Mode;
			
			pic->MB = i + j * (pels/MB_SIZE);
			
			if (Mode == MODE_INTER || Mode == MODE_INTER_Q || Mode==MODE_INTER4V) 
			{
				/* Ԥ��P-��� */
				diff = Predict_P(curr,prev_recon,prev_ipol,	i*MB_SIZE,j*MB_SIZE,MV,pic->PB);
			}
			else 
			{
				diff = (MB_Structure *)malloc(sizeof(MB_Structure));
				FillLumBlock(i*MB_SIZE, j*MB_SIZE, curr, diff);//д����ͼ��  curr:ͼ������ diff:�������
				FillChromBlock(i*MB_SIZE, j*MB_SIZE, curr, diff);//дɫ��ͼ��
			}
			
			/* P��INTRA��� */
			qcoeff_P = MB_Encode(diff, QP_xmitted, Mode); //�Ժ�����ݣ�P��Ϊ�в����ݣ�����DCT�任����
			CBP = FindCBP(qcoeff_P, Mode, 64); 
			if (CBP == 0 && (Mode == MODE_INTER || Mode == MODE_INTER_Q)) 
				ZeroMBlock(diff); //���������Ϊ0
			else
				MB_Decode(qcoeff_P, diff, QP_xmitted, Mode);//���任
			recon_data_P = MB_Recon_P(prev_recon, prev_ipol,diff,i*MB_SIZE,j*MB_SIZE,MV,pic->PB);//�ؽ�Pͼ��
			Clip(recon_data_P); //ʹ 0<=recon_data_P<=255 
			free(diff);
			
			if(pic->PB==0)
				ZeroVec(MV[5][j+1][i+1]); //PB֡ʸ������Ϊ0
			
			if ((CBP==0) && (CBPB==0) &&  (EqualVec(MV[0][j+1][i+1],&ZERO)) && 	(EqualVec(MV[5][j+1][i+1],&ZERO)) && (Mode == MODE_INTER || Mode == MODE_INTER_Q)) 
			{
				/* �� CBP �� CBPB Ϊ0, 16x16 �˶�ʸ��Ϊ0,PBʸ����Ϊ0��
				���ұ���ģʽΪMODE_INTER��MODE_INTER_Q�������ú�����*/
				if (!syntax_arith_coding)
					CountBitsMB(Mode,1,CBP,CBPB,pic,bits);//���������Ϣ
			}
			else /* ���������� */
			{      
				if (!syntax_arith_coding) /* VLC */
				{ 
					CountBitsMB(Mode,0,CBP,CBPB,pic,bits);
					if (Mode == MODE_INTER  || Mode == MODE_INTER_Q)
					{
						bits->no_inter++;
						CountBitsVectors(MV, bits, i, j, Mode, newgob, pic);//����˶�ʸ������
					}
					else if (Mode == MODE_INTER4V) 
					{
						bits->no_inter4v++;
						CountBitsVectors(MV, bits, i, j, Mode, newgob, pic);
					}
					else 
					{
						/* MODE_INTRA �� MODE_INTRA_Q */
						bits->no_intra++;
						if (pic->PB)
							CountBitsVectors(MV, bits, i, j, Mode, newgob, pic);
					}
					if (CBP || Mode == MODE_INTRA || Mode == MODE_INTRA_Q)
						CountBitsCoeff(qcoeff_P, Mode, CBP, bits, 64);//���ϵ��
				} // end VLC 
				
				QP_prev = QP_xmitted;
			}//end Normal MB
			
			abs_mb_num++;
			QP_cumulative += QP_xmitted;     
			
			ReconImage(i,j,recon_data_P,curr_recon);//�ؽ�ͼ��
			free(recon_data_P);
			free(qcoeff_P);
		}//end for i
	}//end for j
	
	pic->QP_mean = QP_cumulative/(float)abs_mb_num;
	/* �ͷ��ڴ� */
	free(pi);
	for (j = 0; j < (lines>>4)+1; j++)
		for (i = 0; i < (pels>>4)+2; i++) 
			for (k = 0; k < 6; k++)
				free(MV[k][j][i]);
	return;
			
}

void ZeroVec(MotionVector *MV)
{
	MV->x = 0;
	MV->y = 0;
	MV->x_half = 0;
	MV->y_half = 0;
	return;
}
void MarkVec(MotionVector *MV)
{
	MV->x = NO_VEC;
	MV->y = NO_VEC;
	MV->x_half = 0;
	MV->y_half = 0;
	return;
}

void CopyVec(MotionVector *MV2, MotionVector *MV1)
{
	MV2->x = MV1->x;
	MV2->x_half = MV1->x_half;
	MV2->y = MV1->y;
	MV2->y_half = MV1->y_half;
	return;
}

int EqualVec(MotionVector *MV2, MotionVector *MV1)
{
	if (MV1->x != MV2->x)
		return 0;
	if (MV1->y != MV2->y)
		return 0;
	if (MV1->x_half != MV2->x_half)
		return 0;
	if (MV1->y_half != MV2->y_half)
		return 0;
	return 1;
}

unsigned char *InterpolateImage(unsigned char *image, int width, int height)
{
	unsigned char *ipol_image, *ii, *oo;
	int i,j;
	
	ipol_image = (unsigned char *)malloc(sizeof(char)*width*height*4);
	ii = ipol_image;
	oo = image;
	
	/* main image */
	for (j = 0; j < height-1; j++) 
	{
		for (i = 0; i  < width-1; i++) 
		{
			*(ii + (i<<1)) = *(oo + i);
			*(ii + (i<<1)+1) = (unsigned char)((*(oo + i) + *(oo + i + 1) + 1)>>1); 
			*(ii + (i<<1)+(width<<1)) = (unsigned char)((*(oo + i) + *(oo + i + width) + 1)>>1); 
			*(ii + (i<<1)+1+(width<<1)) = (unsigned char)((*(oo+i) + *(oo+i+1) + *(oo+i+width) + *(oo+i+1+width) + 2)>>2);									 
		}
		/* last pels on each line */
		*(ii+ (width<<1) - 2) = *(oo + width - 1);
		*(ii+ (width<<1) - 1) = *(oo + width - 1);
		*(ii+ (width<<1)+ (width<<1)-2) = (unsigned char)((*(oo+width-1)+*(oo+width+width-1)+1)>>1); 
		*(ii+ (width<<1)+ (width<<1)-1) = (unsigned char)((*(oo+width-1)+*(oo+width+width-1)+1)>>1); 
		ii += (width<<2);
		oo += width;
	}
	
	/* last lines */
	for (i = 0; i < width-1; i++)
	{
		*(ii+ (i<<1)) = *(oo + i);    
		*(ii+ (i<<1)+1) = (unsigned char)((*(oo + i) + *(oo + i + 1) + 1)>>1);
		*(ii+ (width<<1)+ (i<<1)) = *(oo + i);    
		*(ii+ (width<<1)+ (i<<1)+1) = (unsigned char)((*(oo + i) + *(oo + i + 1) + 1)>>1);
		
	}
	
	/* bottom right corner pels */
	*(ii + (width<<1) - 2) = *(oo + width -1);
	*(ii + (width<<1) - 1) = *(oo + width -1);
	*(ii + (width<<2) - 2) = *(oo + width -1);
	*(ii + (width<<2) - 1) = *(oo + width -1);
	
	return ipol_image;
}

void MotionEstimatePicture(unsigned char *curr, unsigned char *prev, 
						   unsigned char *prev_ipol, int seek_dist, 
						   MotionVector *MV[6][MBR+1][MBC+2], int gobsync)
{
	int i,j,k;
	int pmv0,pmv1,xoff,yoff;
	int curr_mb[16][16];
	int sad8 = INT_MAX, sad16, sad0;
	int newgob;
	MotionVector *f0,*f1,*f2,*f3,*f4;
	
	/* �˶����Ʋ��洢���MV */
	for ( j = 0; j < lines/MB_SIZE; j++) 
	{
		
		newgob = 0;
		if (gobsync && j%gobsync == 0) 
		{
			newgob = 1;
		}
		
		for ( i = 0; i < pels/MB_SIZE; i++) 
		{
			for (k = 0; k < 6; k++)
				MV[k][j+1][i+1] = (MotionVector *)malloc(sizeof(MotionVector));
			
			/* ���������� */
			f0 = MV[0][j+1][i+1];
			f1 = MV[1][j+1][i+1];
			f2 = MV[2][j+1][i+1];
			f3 = MV[3][j+1][i+1];
			f4 = MV[4][j+1][i+1];
			
			FindPMV(MV,i+1,j+1,&pmv0,&pmv1,0,newgob,0);
			
			if (long_vectors) 
			{
				xoff = pmv0/2; /* �����ܱ�2���� */
				yoff = pmv1/2;
			}
			else 
			{
				xoff = yoff = 0;
			}
			
			MotionEstimation(curr, prev, i*MB_SIZE, j*MB_SIZE, xoff, yoff, seek_dist, MV, &sad0);
			
			sad16 = f0->min_error;
			
			f0->Mode = ChooseMode(curr,i*MB_SIZE,j*MB_SIZE, mmin(sad8,sad16));
			
			/* �����ؾ������� */
			if (f0->Mode != MODE_INTRA) 
			{
				FindMB(i*MB_SIZE,j*MB_SIZE ,curr, curr_mb);//��ǰ������curr_mb
				FindHalfPel(i*MB_SIZE,j*MB_SIZE,f0, prev_ipol, &curr_mb[0][0],16,0);
				sad16 = f0->min_error;

				/* ѡ��0�˶�ʸ�������16x16���˶�ʸ�� */
				if (sad0 < sad16) 
				{
					f0->x = f0->y = 0;
					f0->x_half = f0->y_half = 0;
				}
				
			}
			else 
				for (k = 0; k < 5; k++)
					ZeroVec(MV[k][j+1][i+1]);
		}
	}
	
	return;
}

void ZeroMBlock(MB_Structure *data)
{
	int n;
	register int m;
	
	for (n = 0; n < MB_SIZE; n++)
		for (m = 0; m < MB_SIZE; m++)
			data->lum[n][m] = 0;
		for (n = 0; n < (MB_SIZE>>1); n++)
			for (m = 0; m < (MB_SIZE>>1); m++) 
			{
				data->Cr[n][m] = 0;
				data->Cb[n][m] = 0;
			}
	return;
}
