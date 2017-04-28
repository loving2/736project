#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "rnglib.h"
#include "ranlib.h"
//#include "pmt.h"

double photoelec(int photons);
double dynodeElec(double in, double E);
double photontoelectrons(int photons, double V[], int dynodes);
void gnuplotit();

//assume photons are all 400 nm
int main(){
	int numdynodes = 11; //number of dynodes, 1 less because reasons
	double V[11] = {337.5, 562.5, 675, 787.5, 1012.5, 1237.5, 
		1350, 1462.5, 1575, 1687.5, 1800}; 
	//voltage array to describe voltages at each stage
	//following 3:2:1:1:1...:1
	//int r[]; //resistor chain array: to be implemented
	double quantumeff = .25; //quantum efficiency @ 400 nm, approx
	int photon;
	double electrons = 0;
	FILE *temp = fopen("data.dat", "w+");
	for (photon = 0; photon < 10000; photon++) {
		electrons = photontoelectrons(1, V, numdynodes);
		if (electrons > 1) fprintf(temp, "%lf \n", electrons);
	}
	gnuplotit();
	return 0;
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


//plot with gnuplot
void gnuplotit() {
	int t = system("./test1.pg");
}




