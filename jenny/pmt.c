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
	double* V = getvoltages(1700, numdynodes);
	//voltage array to describe voltages at each stage
	//following 3:2:1:1:1...:1
	//int r[]; //resistor chain array: to be implemented
	double quantumeff = 0.25; //quantum efficiency @ 400 nm, approx
	double hitrate = 1000; //photon packets per second incident on photocathode
	double photonpacket = 1; //number of photons in packet
	double timelength = 50; //time for simulation in seconds
	double timestep = 50*pow(10,-9); //50 ns for pulse
	FILE *temp = fopen("noise.dat", "w");
	int i, j, totalphot;
	totalphot = 0;
	double photons = 0;

	double electrons = 0;
	double lastvolt, voltdrop;
	
	for (i = 0; i < (int)(timelength/timestep); i++) {
		if (i > pow(10,8) && i < pow(10,9)/5 && photonpacket < 2) photonpacket = 2;
		else if (i > pow(10,9)/5 && i < pow(10,9)/4 && photonpacket < 3) photonpacket = 3;
		else if (i > pow(10,9)/5 && i < pow(10,9)/2 && photonpacket < 4) photonpacket = 4;
		else if (i > pow(10,9)/5 && i < pow(10,9) && photonpacket < 5) photonpacket = 5;
		electrons = 0;

		photons = (double)ignpoi((float)(photonpacket*hitrate*timestep));//number photons incident on photocathode in timestep
		totalphot += photons;

		electrons = photontoelectrons(photons, V, numdynodes, quantumeff);
		if (electrons > 0) fprintf(temp, "%lf\n", 1+electrons/10000);

		electrons = 0;//reset for thermionic emission
		
		electrons = ignpoi(thermelec(20)*timestep);
		lastvolt = 0;
		for (j = 0; j < numdynodes+2; j++) {//+2 for anode && cathode stages
			voltdrop = V[j] - lastvolt;
			electrons += dynodeElec(electrons, voltdrop);
			lastvolt = V[j];
		}

		if (electrons > 0) fprintf(temp, "%lf\n", 1+electrons/10000);
	}
	printf("Total number of photoelectrons: %d\n", totalphot);
	int t = fclose(temp);
	gnuplotit();
	return 0;
}

//return expected number of electrons/sec from thermionic emission at temperature T
double thermelec(double T) {
/*	double workfunc = 2; //assume 2eV work function for bialkali photocathode
	double area = M_PI*pow(65/1000,2)/4; //area in m^2 of photocathode
	double temp = T+273.15; //convert to Kelvin
	double A = 1.2*pow(10,6); //Richardson's constant in A/m^2/K^2
	double boltzman = 8.6173303*pow(10,-5); //Boltzmann's constant in eV/K
	double currentdensity = A*temp*temp*exp(-workfunc/boltzman/temp);
	double current = currentdensity*area;
	//if (current > 0) printf("current is %lf\n", current);
	//return current/(1.602*pow(10,-19));//expected electrons emitted per second
//code is returning 0, need better precision

	double elec = 1.602 * pow(10,-19);
	return current/elec;*/
	return 12500;
}


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
		interval = maxvolt/((double)dynodes+11.0);//11 b/c 8:4 adds 10 (e.g. 8:4 = (7+1):(3+1)), then 1 for anode
		for (i = 0; i < dynodes+1; i++) {
			if (i == 0) voltages[i] = 8*interval;
			else if (i == 1) voltages[i] = voltages[i-1] + 4*interval;
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
	double mean = 0;
	//mean number of secondary electrons created by incoming electrons with energy E
	if (in == 0 ) {
		mean = delta * 5 * pow(10,-8); 
	} else mean = in*delta;
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
	snprintf(command, sizeof(command), "gnuplot gnu.pg");
	int t = system(command);
}
