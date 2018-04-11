#include <wiringPi.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>

#define pwmPin    	1			/* PWM PIN Number */
#define Trig    	0			/* Digital PIN Trigger */
#define Echo    	4			/* Digital PIN Receiver */
#define PI 			3.141592	/* Define PI */

void InitSys(void);				/* Initialisation of the system */
float DisMeasure(void);  		/* Function used to measure the distance of obstacle */
void DefautScan(void);			/* Defaut scan: 0 - 180°, 20cm - 100cm*/
void FigureOutPut(void);		/* Output the figure of detection */
void CalcResult(void);			/* Output the analysis of detection */

int DistObjMax;					/* Distance of obstacle */
int DistObjMin;					/* Distance of obstacle */
int AngleMax;					/* Angle of the scan*/
int AngleMin;					/* Angle of the scan*/
	

void main(void)
{
	InitSys();
	DefautScan();	
	FigureOutPut();
	CalcResult();
}

/* Initialisation of the system */
void InitSys(void)
{
	/* When initialize wiring failed,print messageto screen */
	if(wiringPiSetup() == -1)
	{ 
		printf("setup wiringPi failed !");
		system("pause");
	}	
	
	/* Initialization of the detector */
	pinMode(Echo, INPUT);
	pinMode(Trig, OUTPUT);
	
	/* Initialization of the pwm pin:
	- pwmFrequency in Hz = 19.2e6 Hz / pwmClock / pwmRange. 
	- duty cycle = pwmWrite/pwmRange */	
	pinMode(pwmPin, PWM_OUTPUT);
	pwmSetMode(PWM_MODE_MS);
	pwmSetClock(192);	// delta = 10us
	pwmSetRange(2000); 	// f = 50HZ; T = 20ms
	pwmWrite(pwmPin,0);
	
}

/* Defaut scan: 0 - 180°, 20cm - 100cm*/
void DefautScan(void)
{
	float DistObj;					/* Distance of obstacle */
	float AngleDeg;					/* Angle of the scan in degree */
	float AngleRad;					/* Angle of the scan in rad */
	float XCoord;					/* X coordinate */
	float YCoord;					/* Y coordinate */
	int i;
	
	FILE * data = fopen("data.temp", "w");
	
	DistObjMax = 20;
	DistObjMin = 100;
	AngleMax = 0;
	AngleMin = 180;
	for (i = 0; i <200;i++)
	{
		pwmWrite(pwmPin,50 + i);			/* Turn the detector */
		DistObj = DisMeasure();				/* Get the distance */
		AngleRad = i*PI/200.0;				/* Calculate the angle */
		AngleDeg = i*180.0/200.0;			/* Calculate the angle */
		XCoord = DistObj*(cos(AngleRad));	/* Calculate the X coordinate */
		YCoord = DistObj*(sin(AngleRad));	/* Calculate the Y coordinate */
		
		if ((floor(DistObj) >= 20)&&(ceil(DistObj) <= 100))
		{
			printf("Angle %0.2f° \n",AngleDeg);
			printf("%0.2f cm\n\n",DistObj);
			fprintf(data, "%lf %lf \n", XCoord, YCoord); /* Write the data to a temporary file */
			(DistObjMax < ceil(DistObj)) ? (DistObjMax = ceil(DistObj)) : 1;
			(DistObjMin > floor(DistObj)) ? (DistObjMin = floor(DistObj)) : 1;
			(AngleMax < ceil(AngleDeg)) ? (AngleMax = ceil(AngleDeg)) : 1;
			(AngleMin > floor(AngleDeg)) ? (AngleMin = floor(AngleDeg)) : 1;			
		}
		delay(100);
	}
}

/* Output the figure of detection */
void FigureOutPut(void)
{
	FILE * gnuplotPipe = popen ("gnuplot -persistent", "w");	
	fprintf(gnuplotPipe, "%s \n", "set xr [-100:100]");
	fprintf(gnuplotPipe, "%s \n", "set yr [0:100]");
	fprintf(gnuplotPipe, "%s \n", "plot 'data.temp'");	
}

/* Output the analysis of detection */
void CalcResult(void)
{
	if ((DistObjMax > DistObjMin)&&(AngleMax > AngleMin))
	{
		printf("Between \n %d cm and %d cm \n %d° and %d° \n  There is one obstacle \n",DistObjMin,DistObjMax,AngleMin,AngleMax);
	}
	else
	{
		printf("No obstacle detected \n");		
	}	
}

/* Function used to measure the distance of obstacle */
float DisMeasure(void)
{
	struct timeval tv1;
	struct timeval tv2;
	long time1, time2;
    float dis;

	digitalWrite(Trig, LOW);
	delayMicroseconds(2);

	digitalWrite(Trig, HIGH);
	delayMicroseconds(10);      					/* Generation of the pulse */
	digitalWrite(Trig, LOW);
								
	while(!(digitalRead(Echo) == 1));
	gettimeofday(&tv1, NULL);           			/* Get the current time */

	while(!(digitalRead(Echo) == 0));
	gettimeofday(&tv2, NULL);           			/* Get the current time */

	time1 = tv1.tv_sec * 1000000 + tv1.tv_usec;   	/* Time in us */
	time2  = tv2.tv_sec * 1000000 + tv2.tv_usec;

	dis = (float)(time2 - time1) / 1000000 * 34000 / 2;  /* Calculate the distance */

	return dis;
}