#include <stdio.h>
#include <stdlib.h>
#define MAX_SIZE 256

int createGraph(int version);

// initialise as -1 because frequency and duty cycle can never have negative value
int greenFrequency = -1;
int redFrequency = -1;
int greenDutyCycle = -1;
int redDutyCycle = -1;

int main() {
    FILE *datafile = fopen("WaveLED.csv", "r");

    //if data file is not in same directory and name is not accordingly to this WaveLED.csv, then error. 
    if (datafile == NULL) {
        printf("Error: Could not open data file 'WaveLED.csv'\n");
        return 1;
    }

    char line[MAX_SIZE];
    
    //read the csvfile data name and values, if both matches means the data is correct and then display onto gnupplot.
    while (fgets(line, sizeof(line), datafile)) {
        if (sscanf(line, "Frequency of Green LED: %d%%", &greenFrequency) == 1) {
        } else if (sscanf(line, "Duty Cycle of Green LED: %d", &greenDutyCycle) == 1) {
        } else if (sscanf(line, "Frequency of Red LED: %d%%", &redFrequency) == 1) {
        } else if (sscanf(line, "Duty Cycle of Red LED: %d", &redDutyCycle) == 1) {
        }
    }


    //if wrong then display error message for example "blue frequency" & "-1" in the datafile. Both must match not either.
    if (greenFrequency == -1 || redFrequency == -1 || greenDutyCycle == -1 || redDutyCycle == -1) {
        printf("Error: Failed to read LED information from data file\n");
        return 1;
    }

    fclose(datafile);
    
    // Create Graphs version, 1 is 60s version, 2 is 1s version
    int success = createGraph(1);
    success = createGraph(2);

    // If success == 0 means createGraph() has returned 0 which means successful creation of graph
    if(success == 0){
        printf("\nGraphs Created! Check the 2 jpeg images in this folder.");
    }else{
        printf("\nFailed to create graphs");
    }

    return 0;
}

int createGraph(int version){
    FILE *gnupipe = popen("gnuplot -persistent", "w"); 

    // Sets the jpeg size
    fprintf(gnupipe, "%s\n", "set term jpeg size 1920,1080"); 

    // To name the 2 different versions of the output jpeg
    if(version == 1){
        fprintf(gnupipe, "%s\n", "set output '60sGraph.jpeg'"); 
    }else if(version == 2){
        fprintf(gnupipe, "%s\n", "set output '1sGraph.jpeg'"); 
    }

    // Using multiplot to have 2 graphs & set the separator in the csv file
    fprintf(gnupipe, "%s\n", "set multiplot layout 2,1"); 
    fprintf(gnupipe, "%s\n", "set datafile separator ','"); 

    if(version == 1){
        // Set x-range of graph & x-interval and label of x (60s version)
        fprintf(gnupipe, "%s\n", "set xrange [0:60]"); 
        fprintf(gnupipe, "%s\n", "set xtics 2");
        fprintf(gnupipe, "set xlabel 'Time (60 seconds)'\n"); 
    }else if(version == 2){
        // Set x-range of graph & x-interval and label of x (1s version)
        fprintf(gnupipe, "%s\n", "set xrange [0:1]"); 
        fprintf(gnupipe, "%s\n", "set xtics 0.02");
        fprintf(gnupipe, "set xlabel 'Time (1 seconds)'\n"); 
    }

    // Set y-range of graph & y-interval
    fprintf(gnupipe, "%s\n", "set yrange [-0.5:1.5]");
    fprintf(gnupipe, "%s\n", "set ytics 1");

    // Set title to Green LED Freq & Duty Cycle then Plot the Green LED Graph
    fprintf(gnupipe, "set title 'Blink Green LED at %d Hz %d%% Duty Cycle' offset 0,-2.3\n", greenFrequency, greenDutyCycle); 
    fprintf(gnupipe, "%s\n", "plot 'WaveLED.csv' u 1:2 w steps lc 'green' lw 2 notitle"); 

    // Set title to Red LED Freq & Duty Cycle then Plot the Red LED Graph
    fprintf(gnupipe, "set title 'Blink Red LED at %d Hz %d%% Duty Cycle' offset 0,-2.3\n", redFrequency, redDutyCycle); 
    fprintf(gnupipe, "%s\n", "plot 'WaveLED.csv' u 1:3 w steps lc 'red' lw 2 notitle"); 
    
    //exit from multiplot mode, which allows you to create multiple plots within a single plot window
    fprintf(gnupipe, "%s\n", "unset multiplot"); 

    //close the process not close the program, basically end the plotting, once done.
    fclose(gnupipe); 

    return 0;
}
