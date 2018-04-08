#include <wiringPi.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>

#define pwmPin    	1
#define Trig    	0
#define Echo    	4

float disMeasure(void)
{
	struct timeval tv1;
	struct timeval tv2;
	long time1, time2;
    float dis;

	digitalWrite(Trig, LOW);
	delayMicroseconds(2);

	digitalWrite(Trig, HIGH);
	delayMicroseconds(10);      //发出超声波脉冲
	digitalWrite(Trig, LOW);
								
	while(!(digitalRead(Echo) == 1));
	gettimeofday(&tv1, NULL);           //获取当前时间

	while(!(digitalRead(Echo) == 0));
	gettimeofday(&tv2, NULL);           //获取当前时间

	time1 = tv1.tv_sec * 1000000 + tv1.tv_usec;   //微秒级的时间
	time2  = tv2.tv_sec * 1000000 + tv2.tv_usec;

	dis = (float)(time2 - time1) / 1000000 * 34000 / 2;  //求出距离

	return dis;
}

int main(void)
{
	int i;
	float dis;

	if(wiringPiSetup() == -1){ //when initialize wiring failed,print messageto screen
		printf("setup wiringPi failed !");
		return 1; 
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

	FILE * temp = fopen("data.temp", "w");
	FILE * gnuplotPipe = popen ("gnuplot -persistent", "w");	
	fprintf(gnuplotPipe, "%s \n", "set xr [-200:200]");
	fprintf(gnuplotPipe, "%s \n", "set yr [0:200]");
	fprintf(gnuplotPipe, "%s \n", "set polar ");
	fprintf(gnuplotPipe, "%s \n", "plot 200");
	
	delay(2000);
	
	while(1){
		for (i = 0; i <200;i++)
		{
			pwmWrite(pwmPin,50 + i);
			dis = disMeasure();
			printf("%d \n",i);
			printf("%0.2f cm\n\n",dis);
			if (dis < 200)
			{
				fprintf(temp, "%lf %lf \n", dis*(cos(i*10*3.14/200)), dis*(sin(i*10*3.14/200))); //Write the data to a temporary file
				fprintf(gnuplotPipe, "%s \n", "plot 'data.temp'");
			}
			delay(100);
		}
	}
	return 0;
}
