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
	long long cnt;//本结点以下的像素总数
	long long rSum, gSum, bSum;//红色分量、绿色分量、蓝色分量灰度值的和
	bool isLeaf;//是否是叶子结点
	int depth;//本节点的深度
	octNode* child[8];//8个子结点的指针数组 
	octNode()
	{
		rSum = gSum = bSum = isLeaf = depth = 0;
		cnt = 1;
		for (int i = 0; i < 8; i++) {
			child[i] = NULL;
		}
	}
};


vector<vector<octNode*>> allPtr(8, vector<octNode*>()); //第0层到第7层所有结点指针


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

	void insertColor(uint8 r, uint8 g, uint8 b);						//插入一个颜色
	uint8 generatePalette(RGBQUAD* pal);						//生成调色板
private:
	octNode* root;														//八叉树的根
	int colors;															//当前的颜色总数
	int maxColors;														//最大颜色数
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

//释放八叉树的内存空间
octTree::~octTree()
{
	//To do....
	makeEmpty(root);
}


//计算每种颜色在第depth层的结点下标
int calacuteIndex(int depth, uint8 r, uint8 g, uint8 b) {
	int index = 0;
	uint8 mask = 0x80 >> depth;
	if (r & mask) index |= 4;
	if (g & mask) index |= 2;
	if (b & mask) index |= 1;
	return index;
}
//创建新的结点
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
//往八叉树中添加一个像素的颜色
void octTree::insertColor(uint8 r, uint8 g, uint8 b)
{
	//....
	octNode* cur = root, * newnode = NULL;
	allPtr[0].push_back(root);
	int index = 0, i = 0; //i为层数 index为该层的下标
	for (i = 0; i < 8; i++) { //从第0层到第7层共8层
		index = calacuteIndex(i, r, g, b);
		//printf("the index is %d\n", index);
		if (cur->child[index] == NULL) {
			newnode = getNode(0, 0, 0, 1, i + 1, 0);
			cur->child[index] = newnode;
			if(i < 7 && newnode)
				allPtr[i + 1].push_back(newnode);
		}
		else {
			cur->cnt += 1; //路径上的结点记录代表的像素数量
		}
		cur = cur->child[index];
	}
	//此时cur指向第8层象素
	cur->rSum += r; //颜色求和
	cur->gSum += g;
	cur->bSum += b;
	cur->isLeaf = true;
	cur->cnt += 1; //个数加一
	//allLeft.insert(cur); //记录叶结点指针
	//printf("the count of point is %d\n", root->cnt);
}

int getLeaftCount(octNode* root) //得到叶结点个数即颜色种类数
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

void reduceNode(octNode* root, int depth, int* colorCount) { //在八叉树的第depth层减色
	//printf("the kind of color is %d\n", *colorCount);
	if (root == NULL)
		return;
	if (*colorCount <= 256)
		return;
	if (root->depth < depth) { //深度不够递归到达
		for (int i = 0; i < 8; i++) {
			reduceNode(root->child[i], depth, colorCount);
		}
	}
	else { //深度达到了depth层 开始减色
		for (int i = 0; i < 8; i++) {
			octNode* leaf = root->child[i];
			if (leaf != NULL) {
				//printf("reduce a color!\n");
				root->rSum += leaf->rSum;
				root->gSum += leaf->gSum;
				root->bSum += leaf->bSum;
				(*colorCount) -= 1; //颜色种类数--

				delete root->child[i];
				root->child[i] = NULL;
			}
		}
		root->isLeaf = true; //父结点成为新的叶子
		(*colorCount) += 1;
		if (*colorCount <= 256) //减色完成
			return;
	}
}

bool compare(octNode* l, octNode* r)
{
	return (l->cnt) < (r->cnt);
}


void newReduceNode(int depth, int* colorCount) //在八叉树的depth层减色
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

void getPalette(octNode* root, RGBQUAD* pal, int& i) //填写调色板
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
		i++; //指向下一个元素
	}
	else { //不是叶结点继续递归
		for (int index = 0; index < 8; index++) {
			getPalette(root->child[index], pal, i);
		}
	}
}


//根据现有的八叉树，选择256个颜色作为最终的调色板中的颜色
uint8 octTree::generatePalette(RGBQUAD* pal)
{
	//....
	int colorCount = getLeaftCount(root); //叶结点个数就是颜色种类数
	printf("the before colorcount is %d\n", colorCount);
	if (colorCount <= 256)
		return colorCount;

	for (int i = 7; i >= 0; i--) { //从深层向上减结点
		//reduceNode(root, i, &colorCount);
		newReduceNode(i, &colorCount);
		if (colorCount <= 256)
			break;
	}
	printf("the after colorcount is %d\n", colorCount);
	int index = 0;
	getPalette(root, pal, index); //下标i是传引用
	return colorCount;
}


int getDistance(uint8 r, uint8 g, uint8 b, RGBQUAD* pal, int index) //得到两个颜色的欧拉距离的平方
{
	tagRGBQUAD palcolor = pal[index];
	int r_dis = (int)(r - palcolor.rgbRed) * (r - palcolor.rgbRed);
	int g_dis = (int)(g - palcolor.rgbGreen) * (g - palcolor.rgbGreen);
	int b_dis = (int)(b - palcolor.rgbBlue) * (b - palcolor.rgbBlue);
	return r_dis + g_dis + b_dis;

}

//从调色板中选出与给定颜色最接近的颜色的下标
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
	return (uint8)res_index;//给定某颜色，返回其在调色板中最近似颜色的索引值；
}

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		printf("using: exe[0], input file[1], output file[2]\n");
		return -1;
	}
	BITMAPFILEHEADER bf, * pbf;//输入、输出文件的文件头
	BITMAPINFOHEADER bi, * pbi;//输入、输出文件的信息头
	RGBQUAD* pRGBQuad;//待生成的调色板指针
	uint8* pImage;//转换后的图象数据
	DWORD bfSize;//文件大小
	LONG biWidth, biHeight;//图象宽度、高度
	DWORD biSizeImage;//图象的大小，以字节为单位，每行字节数必须是4的整数倍
	unsigned long biFullWidth;//每行字节数必须是4的整数倍

	//打开输入文件
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

	//创建输出文件
	printf("Creating %s ... ", outputName);
	if (!(fpOut = fopen(outputName, "wb")))
	{
		printf("\nCan't create %s!\n", outputName);
		return -1;
	}
	printf("Success!\n");

	//读取输入文件的文件头、信息头
	fread(&bf, sizeof(BITMAPFILEHEADER), 1, fpIn);
	fread(&bi, sizeof(BITMAPINFOHEADER), 1, fpIn);

	//读取文件信息
	biWidth = bi.biWidth;
	biHeight = bi.biHeight;
	biFullWidth = ceil(biWidth / 4.) * 4;//bmp文件每一行的字节数必须是4的整数倍
	biSizeImage = biFullWidth * biHeight;
	bfSize = biFullWidth * biHeight + 54 + 256 * 4;//图象文件的大小，包含文件头、信息头

	//设置输出文件的BITMAPFILEHEADER
	pbf = new BITMAPFILEHEADER;
	pbf->bfType = 19778;
	pbf->bfSize = bfSize;
	pbf->bfReserved1 = 0;
	pbf->bfReserved2 = 0;
	pbf->bfOffBits = 54 + 256 * 4;
	//写出BITMAPFILEHEADER
	if (fwrite(pbf, sizeof(BITMAPFILEHEADER), 1, fpOut) != 1)
	{
		printf("\nCan't write bitmap file header!\n");
		fclose(fpOut);
		return -1;
	}

	//设置输出文件的BITMAPINFOHEADER
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
	//写出BITMAPFILEHEADER
	if (fwrite(pbi, sizeof(BITMAPINFOHEADER), 1, fpOut) != 1)
	{
		printf("\nCan't write bitmap info header!\n");
		fclose(fpOut);
		return -1;
	}

	//构建颜色八叉树
	printf("Building Color OctTree ...  \n");
	octTree* tree;
	tree = new octTree(256);
	uint8 RGB[3];
	//读取图像中每个像素的颜色，并将其插入颜色八叉树
	//printf("the h is %d, the w is %d\n", bi.biHeight, bi.biWidth);

	for (int i = 0; i < bi.biHeight; i++)
	{
		//printf("the %d h point\n", i);
		fseek(fpIn, bf.bfOffBits + i * ceil(biWidth * 3 / 4.) * 4, 0);
		for (int j = 0; j < bi.biWidth; j++)
		{
			//读取一个像素的颜色，并将其插入颜色八叉树
			fread(&RGB, 3, 1, fpIn);
			tree->insertColor(RGB[2], RGB[1], RGB[0]);
		}
	}
	printf("Success!\n");

	//生成并填充调色板
	printf("Generating palette ... \n");
	pRGBQuad = new RGBQUAD[256];
	tree->generatePalette(pRGBQuad);

	//输出256色调色板
	if (fwrite(pRGBQuad, 256 * sizeof(RGBQUAD), 1, fpOut) != 1)
	{
		printf("\nCan't write palette!\n");
		fclose(fpOut);
		return -1;
	}
	printf("Success!\n");

	//填充图像数据
	printf("Generating the output image ... \n");
	pImage = new uint8[biSizeImage];
	memset(pImage, 0, biSizeImage);
	for (int i = 0; i < bi.biHeight; i++)
	{
		fseek(fpIn, bf.bfOffBits + i * ceil(biWidth * 3 / 4.) * 4, 0);
		for (int j = 0; j < bi.biWidth; j++)
		{
			//读取一个像素的颜色，并将其转换位颜色索引值
			fread(&RGB, 3, 1, fpIn);
			pImage[i * biFullWidth + j] = selectClosestColor(RGB[2], RGB[1], RGB[0], pRGBQuad);
		}
	}
	//输出图象数据
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