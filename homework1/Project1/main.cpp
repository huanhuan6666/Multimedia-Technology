#define _CRT_SECURE_NO_DEPRECATE

#include <windows.h>
#include <cstdio>
#include <cmath>
#include <vector>
#include <algorithm>
using namespace std;

typedef unsigned char uint8;

struct octNode
{
	long long cnt;//��������µ���������
	long long rSum, gSum, bSum;//��ɫ��������ɫ��������ɫ�����Ҷ�ֵ�ĺ�
	bool isLeaf;//�Ƿ���Ҷ�ӽ��
	int depth;//���ڵ�����
	octNode* child[8];//8���ӽ���ָ������ 
	octNode()
	{
		rSum = gSum = bSum = isLeaf = depth = 0;
		cnt = 1;
		for (int i = 0; i < 8; i++) {
			child[i] = NULL;
		}
	}
};


vector<vector<octNode*>> allPtr(8, vector<octNode*>()); //��0�㵽��7�����н��ָ��


class octTree
{
public:
	octTree()
	{
		root = new octNode;
	};
	octTree(int maxColorNum)
	{
		maxColors = maxColorNum;
		root = new octNode;
	}
	~octTree();

	void insertColor(uint8 r, uint8 g, uint8 b);						//����һ����ɫ
	uint8 generatePalette(RGBQUAD* pal);						//���ɵ�ɫ��
private:
	octNode* root;														//�˲����ĸ�
	int colors;															//��ǰ����ɫ����
	int maxColors;														//�����ɫ��
};

void makeEmpty(octNode* root) {
	if (root == NULL)
		return;
	for (int i = 0; i < 8; i++) {
		if (root->child[i] != NULL) {
			makeEmpty(root->child[i]);
			delete root->child[i];
		}
	}
}

//�ͷŰ˲������ڴ�ռ�
octTree::~octTree()
{
	//To do....
	makeEmpty(root);
}


//����ÿ����ɫ�ڵ�depth��Ľ���±�
int calacuteIndex(int depth, uint8 r, uint8 g, uint8 b) {
	int index = 0;
	uint8 mask = 0x80 >> depth;
	if (r & mask) index |= 4;
	if (g & mask) index |= 2;
	if (b & mask) index |= 1;
	return index;
}
//�����µĽ��
octNode* getNode(uint8 r, uint8 g, uint8 b, int count, int depth, int isLeaf) {
	octNode* res = new octNode;
	for (int i = 0; i < 8; i++)
		res->child[i] = NULL;
	res->rSum = r;
	res->gSum = g;
	res->bSum = b;
	res->depth = depth;
	res->isLeaf = isLeaf;
	return res;
}
//���˲��������һ�����ص���ɫ
void octTree::insertColor(uint8 r, uint8 g, uint8 b)
{
	//....
	octNode* cur = root, * newnode = NULL;
	allPtr[0].push_back(root);
	int index = 0, i = 0; //iΪ���� indexΪ�ò���±�
	for (i = 0; i < 8; i++) { //�ӵ�0�㵽��7�㹲8��
		index = calacuteIndex(i, r, g, b);
		//printf("the index is %d\n", index);
		if (cur->child[index] == NULL) {
			newnode = getNode(0, 0, 0, 1, i + 1, 0);
			cur->child[index] = newnode;
			if(i < 7 && newnode)
				allPtr[i + 1].push_back(newnode);
		}
		else {
			cur->cnt += 1; //·���ϵĽ���¼�������������
		}
		cur = cur->child[index];
	}
	//��ʱcurָ���8������
	cur->rSum += r; //��ɫ���
	cur->gSum += g;
	cur->bSum += b;
	cur->isLeaf = true;
	cur->cnt += 1; //������һ
	//allLeft.insert(cur); //��¼Ҷ���ָ��
	//printf("the count of point is %d\n", root->cnt);
}

int getLeaftCount(octNode* root) //�õ�Ҷ����������ɫ������
{
	int sum = 0;
	if (root == NULL)
		return 0;
	if (root->isLeaf)
		return 1;
	for (int i = 0; i < 8; i++) {
		sum += getLeaftCount(root->child[i]);
	}
	return sum;
}

void reduceNode(octNode* root, int depth, int* colorCount) { //�ڰ˲����ĵ�depth���ɫ
	//printf("the kind of color is %d\n", *colorCount);
	if (root == NULL)
		return;
	if (*colorCount <= 256)
		return;
	if (root->depth < depth) { //��Ȳ����ݹ鵽��
		for (int i = 0; i < 8; i++) {
			reduceNode(root->child[i], depth, colorCount);
		}
	}
	else { //��ȴﵽ��depth�� ��ʼ��ɫ
		for (int i = 0; i < 8; i++) {
			octNode* leaf = root->child[i];
			if (leaf != NULL) {
				//printf("reduce a color!\n");
				root->rSum += leaf->rSum;
				root->gSum += leaf->gSum;
				root->bSum += leaf->bSum;
				(*colorCount) -= 1; //��ɫ������--

				delete root->child[i];
				root->child[i] = NULL;
			}
		}
		root->isLeaf = true; //������Ϊ�µ�Ҷ��
		(*colorCount) += 1;
		if (*colorCount <= 256) //��ɫ���
			return;
	}
}

bool compare(octNode* l, octNode* r)
{
	return (l->cnt) < (r->cnt);
}


void newReduceNode(int depth, int* colorCount) //�ڰ˲�����depth���ɫ
{
	if (*colorCount <= 256)
		return;
	printf("reduce the %d depth node\n", depth);
	sort(allPtr[depth].begin(), allPtr[depth].end(), compare);
	int size = allPtr[depth].size();
	octNode* leaf = NULL;
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < 8; j++) {
			leaf = allPtr[depth][i]->child[j];
			if (leaf != NULL) {
				allPtr[depth][i]->rSum += leaf->rSum;
				allPtr[depth][i]->bSum += leaf->bSum;
				allPtr[depth][i]->gSum += leaf->gSum;
				--(*colorCount);
				delete leaf;
				allPtr[depth][i]->child[j] = NULL;

			}
		}
		allPtr[depth][i]->isLeaf = true;
		++(*colorCount);
		if (*colorCount <= 256)
			return;
	}
}

void getPalette(octNode* root, RGBQUAD* pal, int& i) //��д��ɫ��
{
	if (root == NULL) {
		return;
	}
	if (root->isLeaf) {
		pal[i].rgbRed = root->rSum / root->cnt;
		pal[i].rgbGreen = root->gSum / root->cnt;
		pal[i].rgbBlue = root->bSum / root->cnt;
		pal[i].rgbReserved = 0;
		//printf("the %d color is %d, %d, %d\n", i, pal[i].rgbRed, pal[i].rgbGreen, pal[i].rgbBlue);
		i++; //ָ����һ��Ԫ��
	}
	else { //����Ҷ�������ݹ�
		for (int index = 0; index < 8; index++) {
			getPalette(root->child[index], pal, i);
		}
	}
}


//�������еİ˲�����ѡ��256����ɫ��Ϊ���յĵ�ɫ���е���ɫ
uint8 octTree::generatePalette(RGBQUAD* pal)
{
	//....
	int colorCount = getLeaftCount(root); //Ҷ������������ɫ������
	printf("the before colorcount is %d\n", colorCount);
	if (colorCount <= 256)
		return colorCount;

	for (int i = 7; i >= 0; i--) { //��������ϼ����
		//reduceNode(root, i, &colorCount);
		newReduceNode(i, &colorCount);
		if (colorCount <= 256)
			break;
	}
	printf("the after colorcount is %d\n", colorCount);
	int index = 0;
	getPalette(root, pal, index); //�±�i�Ǵ�����
	return colorCount;
}


int getDistance(uint8 r, uint8 g, uint8 b, RGBQUAD* pal, int index) //�õ�������ɫ��ŷ�������ƽ��
{
	tagRGBQUAD palcolor = pal[index];
	int r_dis = (int)(r - palcolor.rgbRed) * (r - palcolor.rgbRed);
	int g_dis = (int)(g - palcolor.rgbGreen) * (g - palcolor.rgbGreen);
	int b_dis = (int)(b - palcolor.rgbBlue) * (b - palcolor.rgbBlue);
	return r_dis + g_dis + b_dis;

}

//�ӵ�ɫ����ѡ���������ɫ��ӽ�����ɫ���±�
uint8 selectClosestColor(uint8 r, uint8 g, uint8 b, RGBQUAD* pal)
{
	int res_index = 0, each = 0;
	int dis = getDistance(r, g, b, pal, 0);
	for (int i = 1; i < 256; i++) {
		each = getDistance(r, g, b, pal, i);
		if (dis > each)
		{
			dis = each;
			res_index = i;
		}
	}
	//printf("color %d %d %d closest index is %d\n", r, g, b, res_index);
	return (uint8)res_index;//����ĳ��ɫ���������ڵ�ɫ�����������ɫ������ֵ��
}

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		printf("using: exe[0], input file[1], output file[2]\n");
		return -1;
	}
	BITMAPFILEHEADER bf, * pbf;//���롢����ļ����ļ�ͷ
	BITMAPINFOHEADER bi, * pbi;//���롢����ļ�����Ϣͷ
	RGBQUAD* pRGBQuad;//�����ɵĵ�ɫ��ָ��
	uint8* pImage;//ת�����ͼ������
	DWORD bfSize;//�ļ���С
	LONG biWidth, biHeight;//ͼ���ȡ��߶�
	DWORD biSizeImage;//ͼ��Ĵ�С�����ֽ�Ϊ��λ��ÿ���ֽ���������4��������
	unsigned long biFullWidth;//ÿ���ֽ���������4��������

	//�������ļ�
	char* inputName, * outputName;
	FILE* fpIn, * fpOut;
	inputName = argv[1];
	outputName = argv[2];
	printf("Opening %s ... ", inputName);
	if (!(fpIn = fopen(inputName, "rb")))
	{
		printf("\nCan't open %s!\n", inputName);
		return -1;
	}
	printf("Success!\n");

	//��������ļ�
	printf("Creating %s ... ", outputName);
	if (!(fpOut = fopen(outputName, "wb")))
	{
		printf("\nCan't create %s!\n", outputName);
		return -1;
	}
	printf("Success!\n");

	//��ȡ�����ļ����ļ�ͷ����Ϣͷ
	fread(&bf, sizeof(BITMAPFILEHEADER), 1, fpIn);
	fread(&bi, sizeof(BITMAPINFOHEADER), 1, fpIn);

	//��ȡ�ļ���Ϣ
	biWidth = bi.biWidth;
	biHeight = bi.biHeight;
	biFullWidth = ceil(biWidth / 4.) * 4;//bmp�ļ�ÿһ�е��ֽ���������4��������
	biSizeImage = biFullWidth * biHeight;
	bfSize = biFullWidth * biHeight + 54 + 256 * 4;//ͼ���ļ��Ĵ�С�������ļ�ͷ����Ϣͷ

	//��������ļ���BITMAPFILEHEADER
	pbf = new BITMAPFILEHEADER;
	pbf->bfType = 19778;
	pbf->bfSize = bfSize;
	pbf->bfReserved1 = 0;
	pbf->bfReserved2 = 0;
	pbf->bfOffBits = 54 + 256 * 4;
	//д��BITMAPFILEHEADER
	if (fwrite(pbf, sizeof(BITMAPFILEHEADER), 1, fpOut) != 1)
	{
		printf("\nCan't write bitmap file header!\n");
		fclose(fpOut);
		return -1;
	}

	//��������ļ���BITMAPINFOHEADER
	pbi = new BITMAPINFOHEADER;
	pbi->biSize = 40;
	pbi->biWidth = biWidth;
	pbi->biHeight = biHeight;
	pbi->biPlanes = 1;
	pbi->biBitCount = 8;
	pbi->biCompression = 0;
	pbi->biSizeImage = biSizeImage;
	pbi->biXPelsPerMeter = 0;
	pbi->biYPelsPerMeter = 0;
	pbi->biClrUsed = 0;
	pbi->biClrImportant = 0;
	//д��BITMAPFILEHEADER
	if (fwrite(pbi, sizeof(BITMAPINFOHEADER), 1, fpOut) != 1)
	{
		printf("\nCan't write bitmap info header!\n");
		fclose(fpOut);
		return -1;
	}

	//������ɫ�˲���
	printf("Building Color OctTree ...  \n");
	octTree* tree;
	tree = new octTree(256);
	uint8 RGB[3];
	//��ȡͼ����ÿ�����ص���ɫ�������������ɫ�˲���
	//printf("the h is %d, the w is %d\n", bi.biHeight, bi.biWidth);

	for (int i = 0; i < bi.biHeight; i++)
	{
		//printf("the %d h point\n", i);
		fseek(fpIn, bf.bfOffBits + i * ceil(biWidth * 3 / 4.) * 4, 0);
		for (int j = 0; j < bi.biWidth; j++)
		{
			//��ȡһ�����ص���ɫ�������������ɫ�˲���
			fread(&RGB, 3, 1, fpIn);
			tree->insertColor(RGB[2], RGB[1], RGB[0]);
		}
	}
	printf("Success!\n");

	//���ɲ�����ɫ��
	printf("Generating palette ... \n");
	pRGBQuad = new RGBQUAD[256];
	tree->generatePalette(pRGBQuad);

	//���256ɫ��ɫ��
	if (fwrite(pRGBQuad, 256 * sizeof(RGBQUAD), 1, fpOut) != 1)
	{
		printf("\nCan't write palette!\n");
		fclose(fpOut);
		return -1;
	}
	printf("Success!\n");

	//���ͼ������
	printf("Generating the output image ... \n");
	pImage = new uint8[biSizeImage];
	memset(pImage, 0, biSizeImage);
	for (int i = 0; i < bi.biHeight; i++)
	{
		fseek(fpIn, bf.bfOffBits + i * ceil(biWidth * 3 / 4.) * 4, 0);
		for (int j = 0; j < bi.biWidth; j++)
		{
			//��ȡһ�����ص���ɫ��������ת��λ��ɫ����ֵ
			fread(&RGB, 3, 1, fpIn);
			pImage[i * biFullWidth + j] = selectClosestColor(RGB[2], RGB[1], RGB[0], pRGBQuad);
		}
	}
	//���ͼ������
	if (fwrite(pImage, biSizeImage, 1, fpOut) != 1)
	{
		printf("\nCan't write image data!\n");
		fclose(fpOut);

		return -1;
	}
	printf("Success!\n");


	delete tree;
	delete pbf;
	delete pbi;
	delete[] pRGBQuad;
	delete[] pImage;
	fclose(fpIn);
	fclose(fpOut);
	printf("All done!\n");
	return 0;
}