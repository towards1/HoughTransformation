#include <stdio.h>
#include "imageRec.h"


const short sobelFilterKernel[3][3] = { { 3, 10, 3 }, { 0, 0, 0 }, { -3, -10, -3 } };
const imageBuffer gausFilterKernel[5] = { 25, 61, 83, 61, 25 };

void normalizeImage(imageBuffer* srcImage, imageBuffer* dstImage)
{
	int min = 0xFFFF;
	int max = 0;

	for (int i = 0; i < IMAGE_BUFFER_SIZE; i++)
	{
		if (srcImage[i] < min) min = srcImage[i];
		if (srcImage[i] > max) max = srcImage[i];
	}

	for (int i = 0; i < IMAGE_BUFFER_SIZE; i++)
	{
		dstImage[i] = (imageBuffer)(((int)srcImage[i] - min) * 0x8000 / (max - min + 1));
	}
}

void gausFilter(imageBuffer * image, imageBuffer * helperImage)
{
	int n = sizeof(gausFilterKernel) / sizeof(gausFilterKernel[0]);
	int acc;
	int acck = 0;
	int i = 0;

	for (int y = 0; y < IMAGE_HIGHT; y++)
	{
		for (int x = 0; x < IMAGE_WIDTH; x++)
		{
			acc = 0;
			acck = 0;

			for (int j = 0; j < n; j++)
			{
				int pos = x + j - n / 2;

				if (pos >= 0 && pos < IMAGE_WIDTH)
				{
					acc += image[i + j - n / 2] * gausFilterKernel[j];
					acck += gausFilterKernel[j];
				}
			}

			helperImage[i] = acc / acck;

			i++;
		}
	}

	i = 0;

	for (int y = 0; y < IMAGE_HIGHT; y++)
	{
		for (int x = 0; x < IMAGE_WIDTH; x++)
		{
			acc = 0;
			acck = 0;

			for (int j = 0; j < n; j++)
			{
				int pos = y + j - n / 2;

				if (pos >= 0 && pos < IMAGE_HIGHT)
				{
					acc += helperImage[i + (j - n / 2) * IMAGE_WIDTH] * gausFilterKernel[j];
					acck += gausFilterKernel[j];
				}
			}

			image[i] = acc / acck;

			i++;
		}
	}
}

/*short combineXy(signed char x, signed char y)
{
	union {
		struct { signed char x, y; } d;
		short ixy;
	}u;

	u.d.x = x;
	u.d.y = y;
	return u.ixy;
}
*/
signed char getX(imageBuffer combinedXy)
{
	return (combinedXy & 0xFF) - 0x80;
}

signed char getY(imageBuffer combinedXy)
{
	return (combinedXy / 0x100) - 0x80;
}

void sobelFilter(imageBuffer * srcImage, imageBuffer * dstImage)
{
	int n = sizeof(gausFilterKernel) / sizeof(gausFilterKernel[0]);
	int i = 0;

	for (int y = 0; y < IMAGE_HIGHT; y++)
	{
		for (int x = 0; x < IMAGE_WIDTH; x++)
		{
			int dx = 0;
			int dy = 0;

			if (x > 0 && x < IMAGE_WIDTH - 1 && y > 0 && y < IMAGE_HIGHT - 1)
			{
				for (int ix = 0; ix < 3; ix++)
				{
					for (int iy = 0; iy < 3; iy++)
					{
						int srcVal = srcImage[(x + ix - 1) + (y + iy - 1) * IMAGE_WIDTH];

						dx += srcVal * sobelFilterKernel[ix][iy];
						dy += srcVal * sobelFilterKernel[iy][ix];
					}
				}

				dx /= 0x800;
				dy /= 0x800;

				if (dx > 127) dx = 127;
				if (dx < -128) dx = -128;
				if (dy > 127) dy = 127;
				if (dy < -128) dy = -128;

				//dstImage[i] = combineXy(dx,dy);
				dstImage[i] = dx + 0x80 + (dy + 0x80) * 0x100;
			}
			else
			{
				dstImage[i] = 0x8080;
			}

			i++;
		}
	}
}

int getSlope(imageBuffer value)
{
	int dy = (value / 0x100) - 0x80;
	int dx = (value & 0xFF) - 0x80;

	if (value > 20)
		int i = 1;

	return (dx * dx + dy * dy);
}

bool isSteeper(short value, short refValue)
{
	if (getSlope(value) > getSlope(refValue))
		return 1;
	else
		return 0;
}


//srcImage must be a result from sobelFilter
void nonMaximumSuppression(imageBuffer * srcImage, imageBuffer * dstImage, int minSlope)
{
	int i = 0;
	int slope;

	for (int y = 0; y < IMAGE_HIGHT; y++)
	{
		for (int x = 0; x < IMAGE_WIDTH; x++)
		{

			if (x > 0 && x < IMAGE_WIDTH - 1 && y > 0 && y < IMAGE_HIGHT - 1)
			{
				slope = getSlope(srcImage[i]);

				if (slope < minSlope)
				{
					dstImage[i] = 0x8080;
				}
				else
				{
					int count = 0;

					if (getSlope(srcImage[i - 1 - IMAGE_WIDTH]) > slope) count++;
					if (getSlope(srcImage[i - 0 - IMAGE_WIDTH]) > slope) count++;
					if (getSlope(srcImage[i + 1 - IMAGE_WIDTH]) > slope) count++;
					if (getSlope(srcImage[i - 1]) > slope) count++;
					if (getSlope(srcImage[i + 1]) > slope) count++;
					if (getSlope(srcImage[i - 1 + IMAGE_WIDTH]) > slope) count++;
					if (getSlope(srcImage[i - 0 + IMAGE_WIDTH]) > slope) count++;
					if (getSlope(srcImage[i + 1 + IMAGE_WIDTH]) > slope) count++;

					if (count > 2)
						dstImage[i] = 0x8080;
					else
						dstImage[i] = srcImage[i];
				}
			}
			else
			{
				dstImage[i] = 0x8080;
			}

			i++;
		}
	}
}


/*void drawLine(imageBuffer* dstImage, int x0, int y0, int x1, int y1)
{
	int x = x0;
	int y = y0;
	int dx = x1 - x0;
	int dy = y1 - y0;
	int err = dx + dy, e2; // error value e_xy

	while (true)
	{
		dstImage[y * IMAGE_WIDTH + x] += 1;// 64 + cr;
		if (x == x1 && y == y1) break;
		e2 = 2 * err;
		if (e2 > dy) { err += dy; x++; }
		if (e2 < dx) { err += dx; y++; }
	}
}*/

void houghTransformCircles(imageBuffer * srcImage, imageBuffer * dstImage)
{
	int i = 0;
	int xslope, yslope;
	int dx, dy;

	for (int y = 0; y < IMAGE_HIGHT; y++)
	{
		for (int x = 0; x < IMAGE_WIDTH; x++)
		{
			if (srcImage[i] != 0x8080)
			{
				xslope = getX(srcImage[i]);
				yslope = getY(srcImage[i]);

				if (xslope * xslope > yslope * yslope)
				{
					for (int xi = 0; xi < IMAGE_WIDTH; xi++)
					{
						int yi = (xi - x) * yslope / xslope + y;

						if (yi > 0 && yi < IMAGE_HIGHT)
							dstImage[yi * IMAGE_WIDTH + xi]++;
					}
				}
				else
				{
					for (int yi = 0; yi < IMAGE_HIGHT; yi++)
					{
						int xi = (yi - y) * xslope / yslope + x;

						if (xi > 0 && xi < IMAGE_WIDTH)
							dstImage[yi * IMAGE_WIDTH + xi]++;
					}
				}
			}

			i++;
		}
	}

}

void intenerlal_houghTransformLines(imageBuffer * srcImage, imageBuffer * dstImage, int filter)
{
	int i = 0;
	const int cx = IMAGE_WIDTH / 2;
	const int cy = IMAGE_HIGHT / 2;

	for (int y = 0; y < IMAGE_HIGHT; y++)
	{
		for (int x = 0; x < IMAGE_WIDTH; x++)
		{
			if (srcImage[i] != 0x8080)
			{
				int xslope = getX(srcImage[i]);
				int yslope = getY(srcImage[i]);
				int xslope2 = xslope * xslope;
				int yslope2 = yslope * yslope;

				if (filter == 0 || (filter == 1 && yslope2 > xslope2) || (filter == 2 && yslope2 < xslope2))
				{

					int inter = ((x - cx) * xslope + (y - cy) * yslope) * 0x100 / (xslope2 + yslope2);
					int htx = cx + inter * xslope / 0x100;
					int hty = cy + inter * yslope / 0x100;

					if (htx > 0 && htx < IMAGE_WIDTH && hty > 0 && hty < IMAGE_HIGHT && dstImage[hty * IMAGE_WIDTH + htx] < 0xFFFF)
						dstImage[hty * IMAGE_WIDTH + htx] += 1;

					//std::cout << htx << " " << hty << " - " << xslope << " " << yslope << " " << ".\n";
					//printf("%i %i %i %i\n", htx, hty, xslope, yslope);
				}

			}

			i++;
		}
	}
}

void houghTransformLines(imageBuffer* srcImage, imageBuffer* dstImage)
{
	intenerlal_houghTransformLines(srcImage, dstImage, 0);
}

void houghTransformVerticalLines(imageBuffer* srcImage, imageBuffer* dstImage)
{
	intenerlal_houghTransformLines(srcImage, dstImage, 2);
}

void houghTransformHorizontalLines(imageBuffer* srcImage, imageBuffer* dstImage)
{
	intenerlal_houghTransformLines(srcImage, dstImage, 1);
}

static unsigned int usqrt4(unsigned int val) {
	unsigned int a, b;

	a = 256;   // starting point is relatively unimportant

	b = val / a; a = (a + b) / 2;
	b = val / a; a = (a + b) / 2;
	b = val / a; a = (a + b) / 2;
	b = val / a; a = (a + b) / 2;
	b = val / a; a = (a + b) / 2;

	return a;
}

void houghTransformMiniscus(imageBuffer * srcImage, imageBuffer * dstImage)
{
	int i = 0;

	for (int y = 0; y < IMAGE_HIGHT; y++)
	{
		for (int x = 0; x < IMAGE_WIDTH; x++)
		{
			if (srcImage[i] != 0)
			{
				int xslope = getX(srcImage[i]);
				int yslope = getY(srcImage[i]);

				if (xslope != 0)
				{
					int preCalc = 0x1000 * yslope / xslope;


					for (int htx = 0; htx < IMAGE_WIDTH; htx++)
					{
						int dx = htx - x;
						//int dy = dx * yslope / xslope;
						int dy = dx * preCalc / 0x1000;

						if (dx != 0)
						{
							int r = usqrt4(dx * dx + dy * dy);
							//int r = sqrt(dx * dx + dy * dy);

							int hty = y + dy - r;

							if (hty >= 0 && hty < IMAGE_HIGHT)
								dstImage[hty * IMAGE_WIDTH + htx] += 1;
						}

					}

				}
			}

			i++;
		}
	}
}

void findMaxima(imageBuffer* srcImage, imageBuffer* dstImage, int threshold, int minDistance)
{
	int ws2 = minDistance / 2;
	int maxVal;
	int maxInd;

	for (int i = 0; i < IMAGE_BUFFER_SIZE; i++)
	{
		dstImage[i] = srcImage[i];
	}

	for (int y = ws2; y < IMAGE_HIGHT; y += ws2)
	{
		for (int x = ws2; x < IMAGE_WIDTH; x += ws2)
		{
			maxVal = threshold;
			maxInd = -1;

			for (int ix = -ws2; ix < ws2; ix++)
			{
				for (int iy = -ws2; iy < ws2; iy++)
				{
					int i = (x + ix) + (y + iy) * IMAGE_WIDTH;

					if (srcImage[i] > maxVal)
					{
						maxInd = i;
						maxVal = srcImage[i];
					}
				}
			}

			for (int ix = -ws2; ix < ws2; ix++)
			{
				for (int iy = -ws2; iy < ws2; iy++)
				{
					int i = (x + ix) + (y + iy) * IMAGE_WIDTH;
					if (i != maxInd)
						dstImage[i] = 0;
				}
			}


		}
	}
}

void bubbleSort(pixel_list* array, int length)
{
	int i, j, tmp;

	for (i = 1; i < length; i++)
	{
		for (j = 0; j < length - i; j++)
		{
			if (array[j].value < array[j + 1].value)
			{
				tmp = array[j].value;
				array[j].value = array[j + 1].value;
				array[j + 1].value = tmp;
			}
		}
	}
}

int getPixelList(imageBuffer* image, pixel_list* list, int listLenght)
{
	int i = 0;
	int j = 0;

	for (int y = 0; y < IMAGE_HIGHT; y++)
	{
		for (int x = 0; x < IMAGE_WIDTH; x++)
		{
			if (image[i] != 0 && j < listLenght)
			{
				list[j].value = image[i];
				list[j].x = x;
				list[j].y = y;
				j++;
			}
			
			i++;
		}
	}

	bubbleSort(list, listLenght);

	return j;
}

void convertToSlope(imageBuffer* srcImage, imageBuffer* dstImage)
{
	for (int i = 0; i < IMAGE_BUFFER_SIZE; i++)
	{
		dstImage[i] = getSlope(srcImage[i]);
	}
}

void binarize(imageBuffer * srcImage, imageBuffer * dstImage, int threshold)
{
	for (int i = 0; i < IMAGE_BUFFER_SIZE; i++)
	{
		dstImage[i] = (srcImage[i] > 0) ? 0x4FFF : 0;
	}
}

void clearBuffer(imageBuffer * dstImage, int startLine, int endLine)
{
	for (int i = 0; i < IMAGE_BUFFER_SIZE; i++)
	{
		if (i / IMAGE_WIDTH >= startLine && i / IMAGE_WIDTH < endLine)
			dstImage[i] = 0;
	}
}

void clearBuffer(imageBuffer* dstImage)
{
	clearBuffer(dstImage, 0, IMAGE_HIGHT);
}
