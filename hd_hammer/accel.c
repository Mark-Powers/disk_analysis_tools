#include <phidget22.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>

PhidgetAccelerometerHandle accelerometer;
PhidgetReturnCode res;
double values[3]; // Used for accelerometer/gyroscope
struct timespec ts;

void handler(void)
{
	clock_gettime(CLOCK_REALTIME, &ts);
	PhidgetAccelerometer_getAcceleration(accelerometer, &values);
	printf("%f,%f,%f,%f\n", ts.tv_sec + 1e-9*ts.tv_nsec, values[0], values[1], values[2]);
}


int main(){
	if(geteuid() != 0)
	{
		fprintf(stderr, "you must be root to use the accelerometer\n");
		exit(1);
	}


	PhidgetAccelerometer_create(&accelerometer);
	res = Phidget_openWaitForAttachment((PhidgetHandle)accelerometer, PHIDGET_TIMEOUT_DEFAULT);
	if (res != EPHIDGET_OK){
		fprintf(stderr, "could not open accelerometer\n");
		exit(1);
	}
	res = PhidgetAccelerometer_setDataInterval(accelerometer, 4); // Minimum is 4 ms (hardware minimum)
	if (res != EPHIDGET_OK){
		fprintf(stderr, "could not set data interval\n");
		exit(1);
	}

	signal(SIGALRM, (void (*)(int)) handler);

	struct itimerval tv;
	tv.it_value.tv_sec = (tv.it_interval.tv_sec = 0);
	tv.it_value.tv_usec = (tv.it_interval.tv_usec = 5*1000); // 5 millisecond
	setitimer(ITIMER_REAL, &tv, NULL);

	for(;;){
	    pause();
	}
}

