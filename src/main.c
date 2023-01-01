#include <stdio.h>
#include <stdlib.h>

#include "pd_api.h"

typedef struct{
	float x;
	float y;
} Vector;

typedef struct{
	Vector location;
	Vector velocity;
	float radius;
} Particle;

static int update(void* userdata);
static void plus(Vector* a, Vector* b);
static void times(Vector* a, float amount);
static void normalise(Vector* a);
static Vector vectorMinus(Vector* a, Vector* b);
static void circle(PlaydateAPI* pd, float x, float y, float radius);

#define setpixel(data, x, y, rowbytes) (data[(y)*rowbytes+(x)/8] &= ~(1 << (uint8_t)(7 - ((x) % 8))))

int width = 0;
int height = 0;
Vector blackhole = {0, 0};

int numberOfBands = 10;
float greyCoefficient = 0;

#define particleCount 3
Particle particles[particleCount];

LCDBitmap* buffer;
uint8_t *data;
int bufferWidth = 50;
int bufferHeight = 30;
int bufferRowBytes = 400;

#ifdef _WINDLL
__declspec(dllexport)
#endif
int eventHandler(PlaydateAPI* pd, PDSystemEvent event, uint32_t arg){
	(void)arg; // arg is currently only used for event = kEventKeyPressed

	if ( event == kEventInit ){
			
		int scale = 2;
		pd->display->setScale(scale);
		width = pd->display->getWidth();
		height = pd->display->getHeight();

		greyCoefficient = 255.0/numberOfBands;

		buffer = pd->graphics->newBitmap(bufferWidth, bufferHeight, kColorWhite);
		pd->graphics->getBitmapData(buffer, &bufferWidth, &bufferHeight, &bufferRowBytes, NULL, &data);

		int i;
		for(i = 0 ; i < particleCount ; i++ ){
			Vector location = {rand() % bufferWidth, rand() % bufferHeight};
			Vector velocity = {0, 0};
			float radius = 1 + rand() % 4;
			Particle p = {location, velocity, radius};
			particles[i] = p;
		}

		blackhole.x = bufferWidth/2;
		blackhole.y = bufferHeight/2;
		
		pd->system->setUpdateCallback(update, pd);
	}
	
	return 0;
}

static int update(void* userdata){

	PlaydateAPI* pd = userdata;
	
	pd->graphics->clear(kColorWhite);  
	 	
	int i;
	for (i = 0; i < particleCount; i++){
		Particle body = particles[i];
		Vector blackholeDirection = vectorMinus(&blackhole, &body.location);
		normalise(&blackholeDirection);
		times(&blackholeDirection, 0.16);
		
		plus(&body.velocity, &blackholeDirection);
		plus(&body.location, &body.velocity);
		//pd->graphics->fillEllipse(100 + body->location.x, body->location.y, body->radius * 2, body->radius * 2, 0, 0, kColorBlack);

		int j;
		for (j = 0; j < particleCount; j++){
			if(j != i){
				Particle other = particles[j];
				Vector bodyDirection = vectorMinus(&body.location, &other.location);
				normalise(&bodyDirection);
				times(&bodyDirection, 0.04);
				plus(&body.velocity, &bodyDirection);
				plus(&body.location, &body.velocity);
			}
		}
		
		particles[i] = body;
	}

	int x;
	int y;
	float sum = 0.0;
	pd->graphics->clearBitmap(buffer, kColorWhite);
	for (x = 0; x < bufferWidth; x+=2){
		for (y = 0; y < bufferHeight; y++){
			sum = 0.0;
			int i;
			for (i = 0; i < particleCount; i++){
				Particle p = particles[i];
				float xD = p.location.x - x;
				float yD = p.location.y - y;
				sum = sum +  (p.radius * 1.5) / sqrt(xD * xD + yD * yD);
			}
			double gray = (sum * numberOfBands) * greyCoefficient;

			if(gray > 255){
				setpixel(data, x, y, bufferRowBytes);
				setpixel(data, x + 1, y, bufferRowBytes);
			}
		}
	}
	pd->graphics->drawScaledBitmap(buffer, 0, 0, width/bufferWidth, height/bufferHeight);  
	pd->system->drawFPS(0,0);

	return 1;
}

static void plus(Vector* a, Vector* b){
	a->x = a->x + b->x;
	a->y = a->y + b->y;
}

static void times(Vector* a, float amount){
	a->x = a->x * amount;
	a->y = a->y * amount;
}

static void normalise(Vector* a){
	double magnitude = sqrt(a->x * a->x + a->y * a->y);
	if(magnitude > 0){
		a->x = a->x/magnitude;
		a->y = a->y/magnitude;
	}
}

static Vector vectorMinus(Vector* a, Vector* b){
	Vector m = {a->x - b->x, a->y - b->y};
	return m;
}

static void circle(PlaydateAPI* pd, float x, float y, float radius){
	pd->graphics->fillEllipse(x, y, radius * 2, radius * 2, 0, 0, kColorBlack);
}
