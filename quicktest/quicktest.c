#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "rnglib.h"
#include "ranlib.h"

void gnuplotit();



int main(){
	float smallpoisson = 0;
	float largepoisson = 0;
	FILE *temp = fopen("quicktest.dat", "w");
	FILE *temp2 = fopen("quicktest2.dat", "w");
	int i, j;
	float mean = 1000;
	for (i = 0; i < 10000; i++) {
		largepoisson = ignpoi(mean);
		fprintf(temp, "%lf\n", largepoisson);
		for (j = 0; j < 1000000; j++) {
			smallpoisson += ignpoi(mean/1000000);
		}
		fprintf(temp2, "%lf\n", smallpoisson);	
		smallpoisson = 0;
	}

	int t = fclose(temp);
	int y = fclose(temp2);
	gnuplotit();
}



//plot with gnuplot
void gnuplotit() {
	char command[400] = {0};
	snprintf(command, sizeof(command), "gnuplot quicktest.pg");
	int t = system(command);
	snprintf(command, sizeof(command), "gnuplot quicktest2.pg");
	int y = system(command);
}
