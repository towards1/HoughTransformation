#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <time.h>
#include "imageRec.h"

imageBuffer imageBuffer1[IMAGE_BUFFER_SIZE];
imageBuffer imageBuffer2[IMAGE_BUFFER_SIZE];
imageBuffer imageBuffer3[IMAGE_BUFFER_SIZE];

int loadImageFromFile(const char *path, imageBuffer *destBuffer)
{
	FILE *fp;
	unsigned char tmpBuff[3];

	fp = fopen(path, "rb");

	for (int i = 0; i < IMAGE_BUFFER_SIZE; i++)
	{
		if (!fread(tmpBuff, 1, 3, fp))
		{
			tmpBuff[0] = (i % 32) * 8;
		}

		//sum up channel red, green and blue
		destBuffer[i] = (imageBuffer)tmpBuff[0] + tmpBuff[1] + tmpBuff[2];

	}


	fclose(fp);

	return 0;
}

int writeBmpFile(const char *path, imageBuffer *image)
{
	int w = IMAGE_WIDTH;
	int h = IMAGE_HIGHT;
	FILE *f;
	unsigned char *img = NULL;
	int filesize = 54 + 3 * w*h;  //w is your image width, h is image height, both int
	int x;
	int y;

	img = (unsigned char *)malloc(3 * w*h);
	if (img)
	{
		memset(img, 0, 3 * w * h);

		for (int i = 0; i < w; i++)
		{
			for (int j = 0; j < h; j++)
			{
				x = i;
				y = j;

				img[(x + y * w) * 3 + 2] = (unsigned char)(image[i + j * w] / 0x80);
				img[(x + y * w) * 3 + 1] = (unsigned char)(image[i + j * w] / 0x80);
				img[(x + y * w) * 3 + 0] = (unsigned char)(image[i + j * w] / 0x80);
			}
		}

		unsigned char bmpfileheader[14] = { 'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0 };
		unsigned char bmpinfoheader[40] = { 40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0 };
		unsigned char bmppad[3] = { 0,0,0 };

		bmpfileheader[2] = (unsigned char)(filesize);
		bmpfileheader[3] = (unsigned char)(filesize >> 8);
		bmpfileheader[4] = (unsigned char)(filesize >> 16);
		bmpfileheader[5] = (unsigned char)(filesize >> 24);

		bmpinfoheader[4] = (unsigned char)(w);
		bmpinfoheader[5] = (unsigned char)(w >> 8);
		bmpinfoheader[6] = (unsigned char)(w >> 16);
		bmpinfoheader[7] = (unsigned char)(w >> 24);
		bmpinfoheader[8] = (unsigned char)(h);
		bmpinfoheader[9] = (unsigned char)(h >> 8);
		bmpinfoheader[10] = (unsigned char)(h >> 16);
		bmpinfoheader[11] = (unsigned char)(h >> 24);

		f = fopen(path, "wb");
		fwrite(bmpfileheader, 1, 14, f);
		fwrite(bmpinfoheader, 1, 40, f);
		for (int i = 0; i < h; i++)
		{
			fwrite(img + (w * (h - i - 1) * 3), 3, w, f);
			fwrite(bmppad, 1, (4 - (w * 3) % 4) % 4, f);
		}

		free(img);
		fclose(f);
	}
	return 0;
}

int main()
{
	clock_t t1, t2;

	loadImageFromFile("test.rgb", imageBuffer1);
	
	getchar();

	t1 = clock();

	for (int i = 0; i < 100; i++)
	{
		normalizeImage(imageBuffer1, imageBuffer1); //0.2 ms

		gausFilter(imageBuffer1, imageBuffer2); // 14 ms

		sobelFilter(imageBuffer1, imageBuffer2); //14 ms

		nonMaximumSuppression(imageBuffer2, imageBuffer3); //9 ms

		clearBuffer(imageBuffer2, 0, IMAGE_HIGHT); //1.5 ms

		houghTransformMiniscus(imageBuffer3, imageBuffer2); //36 ms mit sqrt

		//gausFilter(imageBuffer2, imageBuffer3);// 14 ms

		normalizeImage(imageBuffer2, imageBuffer3); //0.2 ms

		getMaximaList(imageBuffer3, imageBuffer2, 0x7FFF * 0.3, 8);  // 0.5 ms

	}

	t2 = clock();
	double time_taken = ((double)t2 - (double)t1) / CLOCKS_PER_SEC; // in seconds 
	printf("operation took %f seconds to execute \n", time_taken);


    writeBmpFile("test_out1.bmp", imageBuffer1);

	normalizeImage(imageBuffer2, imageBuffer2);
	writeBmpFile("test_out2.bmp", imageBuffer2);

	return 0;
}