#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "rnglib.h"
#include "ranlib.h"


double photoelec(int photons);
double dynodeElec(double in, double E);
double photontoelectrons(int photons, double V[], int dynodes);
void gnuplotit(double min, double max, double stddev, double mean);
double* findmean();
double findstddev(double mean);
double* getvoltages(double maxvolt, int dynodes);
double findgain();

//assume photons are all 400 nm
int main(){
	int numdynodes = 11; //number of dynodes
	//double V[11+2] = {337.5, 562.5, 675, 787.5, 900, 1012.5, 1125, 1237.5, 
		//1350, 1462.5, 1575, 1687.5, 1800}; 
	double* V = getvoltages(1800, numdynodes);
	//voltage array to describe voltages at each stage
	//following 3:2:1:1:1...:1
	//int r[]; //resistor chain array: to be implemented
	double quantumeff = .25; //quantum efficiency @ 400 nm, approx
	int photon;
	double electrons = 0;
	FILE *temp = fopen("data.dat", "w");
	for (photon = 0; photon < 1000000; photon++) {
		electrons = photontoelectrons(1, V, numdynodes);
		if (electrons > 1) {
			fprintf(temp, "%lf\n", electrons);
		}
	}
	int t = fclose(temp);
	double *values;
	values = findmean();
	double stddev = findstddev(*(values+2));
	gnuplotit(*(values), *(values+1), stddev, *(values+2));
	return 0;
}

//follow 3:2:1:1:1...:1 and calculate voltages based on max voltage & num dynodes
double* getvoltages(double maxvolt, int dynodes) {
	int i;
	double interval = 0;
	static double* voltages;
	if (dynodes < 3) {
		return 0;
	} else {
		interval = maxvolt/((double)dynodes+5.0);
		for (i = 0; i < dynodes; i++) {
			if (i == 0) voltages[i] = 3*interval;
			else if (i == 1) voltages[i] = voltages[i-1] + 2*interval;
			else voltages[i] = voltages[i-1] + interval;
		}
	return voltages;
	}
}


// simulate # photoelectrons created by # photons
double photoelec(int photons) {
	double num = 0;
	int i;
	for (i = 0; i < photons; i++) {
		if (genunf(0.0, 1.0) < 0.25) num = num + 1; //uniform number gen
	}
	return num;
}

// simulate number of secondary electrons from # coming and energy E
double dynodeElec(double in, double E) {
	//20eV, on average energy needed 
	//to get 1 secondary emission electron
	//assume gaussian fit to secondary electron emission
	//12 electrons is max, around 950 eV
	//should look like 12*exp{-(x-950)^2/348061}
	if (E < 0.0) return 0.0;
	double exponent = (E - 950.0)*(E - 950.0)/348061.0;
	double delta = 12.0*exp(-exponent);
	double mean;
	//mean number of secondary electrons created by incoming electrons with energy E
	mean = in*delta; 
	return (double)ignpoi((float)mean); //poisson random deviate w/ mean = mean
}

//simulate sequence of dynodes with voltages in V[]
double photontoelectrons(int photons, double V[], int dynodes) {
	double electrons = photoelec(photons);
	double lastvolt = 0.0;
	double voltdrop;
	int i;
	for (i = 0; i < dynodes; i++) {
		voltdrop = V[i] - lastvolt;
		electrons += dynodeElec(electrons, voltdrop);
		lastvolt = V[i];
	}
	return electrons;
}

//simple implementations to pass to gnuplot
double* findmean() {
	FILE *tmp = fopen("data.dat", "r");
	static double values[3];
	double dummy;
	double min = 1000000.0;
	double max = 0.0;
	double mean = 0.0;
	int num = 0;
	while (fscanf(tmp, "%lf", &(dummy)) != EOF) {
		num++;
		mean += dummy;

		if (dummy > max) max = dummy;
		if (min > dummy) min = dummy;
	}
	int t = fclose(tmp);
	mean = mean/(double)num;
	values[0] = min;
	values[1] = max;
	values[2] = mean;
	return values;
}
double findstddev(double mean) {
	FILE *tmp = fopen("data.dat", "r");
	double stddev, dummy, num;
	while (fscanf(tmp, "%lf", &(dummy)) != EOF) {
		num++;
		stddev += pow(dummy - mean, 2);
	}
	int t = fclose(tmp);
	return sqrt(stddev/num);
}

//plot with gnuplot
void gnuplotit(double min, double max, double stddev, double mean) {
	char command[400] = {0};
	snprintf(command, sizeof(command), "gnuplot -c test1.pg %g %g %g %g", min, max, stddev, mean);
	int t = system(command);
}




