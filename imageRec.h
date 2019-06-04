#define IMAGE_WIDTH 640
#define IMAGE_HIGHT 480
#define IMAGE_BUFFER_SIZE (IMAGE_WIDTH * IMAGE_HIGHT)

#define RADIUS_VAL 500 

typedef unsigned short imageBuffer;

struct pixel_list {
	imageBuffer value;
	int x;
	int y;
};

void normalizeImage(imageBuffer* srcImage, imageBuffer* dstImage);

void gausFilter(imageBuffer* image, imageBuffer* helperImage);

void sobelFilter(imageBuffer* srcImage, imageBuffer* dstImage);

void nonMaximumSuppression(imageBuffer* srcImage, imageBuffer* dstImage, int minSlope);

void houghTransformCircles(imageBuffer* srcImage, imageBuffer* dstImage);

void houghTransformLines(imageBuffer* srcImage, imageBuffer* dstImage);

void houghTransformVerticalLines(imageBuffer* srcImage, imageBuffer* dstImage);

void houghTransformHorizontalLines(imageBuffer* srcImage, imageBuffer* dstImage);

void houghTransformMiniscus(imageBuffer* srcImage, imageBuffer* dstImage);

void findMaxima(imageBuffer* srcImage, imageBuffer* dstImage, int threshold, int minDistance);

int getPixelList(imageBuffer* image, pixel_list* list, int listLenght);

void convertToSlope(imageBuffer* srcImage, imageBuffer* dstImage);

void binarize(imageBuffer* srcImage, imageBuffer* dstImage, int threshold);

void clearBuffer(imageBuffer* dstImage, int startLine, int endLine);

void clearBuffer(imageBuffer* dstImage);