#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "rnglib.h"
#include "ranlib.h"


double photoelec(int photons, double quantumeff);
double dynodeElec(double in, double E);
double photontoelectrons(int photons, double V[], int dynodes, double quantumeff);
void gnuplotit();
double* getvoltages(double maxvolt, int dynodes);
double findgain(double V[], int numdynodes, double quantumeff);
double thermelec(double T);

//assume photons are all 400 nm
int main(){
	int numdynodes = 12; //number of dynodes
	double* V = getvoltages(2000, numdynodes);
	//voltage array to describe voltages at each stage
	//following 3:2:1:1:1...:1
	//int r[]; //resistor chain array: to be implemented
	double quantumeff = 0.25; //quantum efficiency @ 400 nm, approx
	double hitrate = 1000; //photons per second incident on photocathode
	double timelength = 1; //1 second for simulation
	double timestep = 50*pow(10,-9); //50 ns for pulse
	FILE *temp = fopen("jenny.dat", "w");
	
	int i;
	
	double photons = 0;

	for (i = 0; i < (int)(timelength/timestep); i++) {

		photons = (double)ignpoi((float)(hitrate*timestep));
	double electrons = 0;
	double num = 0;
	double mean = 0;
	int photon;
	for (photon = 0; photon < 1000000; photon++) {
		electrons = photontoelectrons(1, V, numdynodes, quantumeff);
		if (electrons > 1) {
			mean += electrons;
			num = num + 1;
		}
	}
		
		fprintf(temp, "%lf\n", electrons);
	}
	int t = fclose(temp);
	int y = fclose(temp2);
	gnuplotit();
	return 0;
}

//return expected number of electrons/sec from thermionic emission at temperature T
double thermelec(double T) {
	double workfunc = 2; //assume 2eV work function for bialkali photocathode
	double area = M_PI*pow(65/1000,2)/4; //area in m^2 of photocathode
	double temp = T+273.15; //convert to Kelvin
	double A = 1.2*pow(10,6); //Richardson's constant in A/m^2/K^2
	double boltzman = 8.6173303*pow(10,-5); //Boltzmann's constant in eV/K
	double currentdensity = A*temp*temp*exp(-workfunc/boltzman/temp);
	double current = currentdensity*area;
	return current/(1.602*pow(10,-19));//expected electrons emitted per second
}


ignpoi(mean);
//calculate number of electrons generated per photon and then average all together to get average gain
double findgain(double V[], int numdynodes, double quantumeff) {
	double electrons = 0;
	double num = 0;
	double mean = 0;
	int photon;
	for (photon = 0; photon < 1000000; photon++) {
		electrons = photontoelectrons(1, V, numdynodes, quantumeff);
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
double photontoelectrons(int photons, double V[], int dynodes, double quantumeff) {
	double electrons = photoelec(photons, quantumeff);
	double lastvolt = 0;
	double voltdrop;
	int i;
	for (i = 0; i < dynodes+2; i++) {//+1 for anode stage
		voltdrop = V[i] - lastvolt;
		electrons += dynodeElec(electrons, voltdrop);
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
