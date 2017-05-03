#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "rnglib.h"
#include "ranlib.h"


double photoelec(int photons, double quantumeff);
double dynodeElec(double in, double E, double denominator);
double photontoelectrons(int photons, double V[], int dynodes, double quantumeff, double denominator);
void gnuplotit();
double* getvoltages(double maxvolt, int dynodes);
double findgain(double V[], int numdynodes, double quantumeff, double denominator);

//assume photons are all 400 nm
int main(){
	int numdynodes = 12; //number of dynodes
	double* V = getvoltages(2000, numdynodes);
	//voltage array to describe voltages at each stage
	//following 3:2:1:1:1...:1
	//int r[]; //resistor chain array: to be implemented
	double quantumeff = 0.25; //quantum efficiency @ 400 nm, approx
	FILE *temp = fopen("degrade2.dat", "w");
	double gain = 0;
	double voltdrop = V[0];
	double denominator = 348061.0;
	
	double exponent;
	double delta;
	
	for (denominator = 348061.0; denominator > 100; denominator = denominator - 100) {
		exponent = (voltdrop - 950)*(voltdrop - 950)/denominator;
		delta = 12*exp(-exponent);
		gain = findgain(V, numdynodes, quantumeff, denominator);
		fprintf(temp, "%lf %lf\n", delta, gain);
	}
	int t = fclose(temp);
	gnuplotit();
	return 0;
}

//calculate number of electrons generated per photon and then average all together to get average gain
double findgain(double V[], int numdynodes, double quantumeff, double denominator) {
	double electrons = 0;
	double num = 0;
	double mean = 0;
	int photon;
	for (photon = 0; photon < 1000000; photon++) {
		electrons = photontoelectrons(1, V, numdynodes, quantumeff, denominator);
		if (electrons > 1) {
			mean += electrons;
			num = num + 1;
		}
	}
	if (num > 0) mean = mean/num;
	return mean;
}

//follow 3:2:1:1:1...:1 and calculate voltages based on max voltage & num dynodes
double* getvoltages(double maxvolt, int dynodes) {
	int i;
	double interval = 0;
	//double voltages[dynodes+1];
	double* voltages = (double*)malloc((dynodes + 1) * sizeof(double));
	if (dynodes < 3) {
		return 0;
	} else {
		interval = maxvolt/((double)dynodes+4.0);//4 b/c 3:2 adds 3 (e.g. 3:2 = (2+1):(1+1)), then 1 for anode
		for (i = 0; i < dynodes+1; i++) {
			if (i == 0) voltages[i] = 3*interval;
			else if (i == 1) voltages[i] = voltages[i-1] + 2*interval;
			else voltages[i] = voltages[i-1] + interval;
		}
	return voltages;
	}
}


// simulate # photoelectrons created by # photons
double photoelec(int photons, double quantumeff) {
	double num = 0;
	int i;
	for (i = 0; i < photons; i++) {
		if (genunf(0.0, 1.0) < (float)quantumeff) num = num + 1; //uniform number gen
	}
	return num;
}

// simulate number of secondary electrons from # coming and energy E
double dynodeElec(double in, double E, double denominator) {
	//20eV, on average energy needed 
	//to get 1 secondary emission electron
	//assume gaussian fit to secondary electron emission
	//12 electrons is max, around 950 eV
	//should look like 12*exp{-(x-950)^2/348061}
	if (E < 0.0) return 0.0;
	double exponent = (E - 950.0)*(E - 950.0)/denominator;
	double delta = 12.0*exp(-exponent);
	double mean;
	//mean number of secondary electrons created by incoming electrons with energy E
	mean = in*delta; 
	return (double)ignpoi((float)mean); //poisson random deviate w/ mean = mean
}

//simulate sequence of dynodes with voltages in V[]
double photontoelectrons(int photons, double V[], int dynodes, double quantumeff, double denominator) {
	double electrons = photoelec(photons, quantumeff);
	double lastvolt = 0;
	double voltdrop;
	int i;
	for (i = 0; i < dynodes+2; i++) {//+1 for anode stage
		voltdrop = V[i] - lastvolt;
		electrons += dynodeElec(electrons, voltdrop, denominator);
		lastvolt = V[i];
	}
	return electrons;
}

//plot with gnuplot
void gnuplotit() {
	char command[400] = {0};
	snprintf(command, sizeof(command), "gnuplot degrade.pg");
	int t = system(command);
}




