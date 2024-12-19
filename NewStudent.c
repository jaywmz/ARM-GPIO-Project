/* 
=== HOW TO RUN ===
Step 1: cd into C file location
Step 2: gcc -o Newstudent Newstudent.c -lwiringPi -lpthread
Step 3: ./Newstudent

=== PRE-REQUISITES ===
Install wiringPi: https://learn.sparkfun.com/tutorials/raspberry-gpio/c-wiringpi-setup
softPwm is installed with wiringPi

=== USEFUL COMMANDS ===
Check wiringPi version: gpio -v
Check GPIO status: gpio readall

=== GPIO PIN CONNECTION ===
27 GREEN LED
13 RED LED
GROUND

GPIO14 to Monitor GPIO15
GPIO15 to Monitor GPIO14
GROUND

=== RASPBERRY PI VERSION ===
Check Version Command: cat /etc/os-release

PRETTY_NAME="Raspbian GNU/Linux 10 (buster)"
NAME="Raspbian GNU/Linux"
VERSION_ID="10"
VERSION="10 (buster)"
VERSION_CODENAME=buster
*/

#include <wiringPi.h>
#include <softPwm.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wiringSerial.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>

/* DEFINITIONS */
#define RED 27      // GPIO Pin 27
#define GREEN 13    // GPIO Pin 13

// Program States
#define TURN_OFF 0
#define TURN_ON 1
#define BLINK 2
#define BLINK_BOTH 3 // New option
#define EXIT 4
#define DATA_POINT 5000 // 1s = 50 data point
//can contain up to 5000 datapoint
#define MAX_SIZE 5000 
// LED Blink Selection
#define BLINK_GREEN 1
#define BLINK_RED 2
#define CONFIRM 1

/* FUNCTION PROTOTYPES */
void setupProgram();
void startProgram();
int getUserSelection();
void turnOffLeds();
void turnOnLeds();
void blink();
void blinkBoth();
int getBlinkLed();
int getBlinkFrequency();
int getBlinkBrightness();
int confirmBlinkSelection(int blinkLed, int blinkFrequency, int blinkBrightness);
void blinkLedWithConfig(int blinkLed, int blinkFrequency, int blinkBrightness);
void endProgram();
void CalculateData(int blinkLed, int blinkFrequency, int blinkBrightness);
void AppendExcel(int blinkLed, int blinkFrequency, int blinkBrightness);
int getValidInput(int min, int max);
// Define two thread function prototypes
void *blinkGreen(void *arg);
void *blinkRed(void *arg);

// Define thread structure
typedef struct {
    int frequency;
    int brightness;
} ThreadArgs;

/* MAIN PROGRAM */
int main(void) {

    setupProgram();
    startProgram();
    endProgram();
    return 0;
}

// Making redWave and greenWave a global variable so that it could print both colour waves into same file.
// (previously when doing green, red would get deleted and when doing red, green would get deleted)
int redWave[MAX_SIZE] = {0};
int greenWave[MAX_SIZE] = {0};

// To print each LED frequency in the csv file
int greenFreq, redFreq, greenDuty,redDuty;

void CalculateData(int blinkLed, int blinkFrequency, int blinkBrightness) {

    float brightness = blinkBrightness;
    int x = 0;                             // Used for loop iterations
    int count = 0;                         // Data points counter to keep track of the number of data points           
    int loop = 0;                          // Loop counter to to adjust the position of the waveform 

    // To find predicted digital waveform    // 
    float digital_Waveform = 50 / blinkFrequency; // 1 second is divided into 50 points, (0.02 each), then used to find out out length per wave
    float waveHigh = digital_Waveform * (brightness/100); // Deducing total HIGH time according to input duty cycle.

    //the loop is used to generate 5000 data points per 0.02s which is equivalent to around 80+ seconds
    while (count <= DATA_POINT) {
        // For loop to Print 1 for HIGHs in waveform
        for (x = 0; x <= waveHigh; x++){       
            if (blinkLed == BLINK_GREEN) {
                // Adding HIGH to green array. On first loop, loop will be 0. On second loop onwards, will add each wave length to it
                greenWave[x + loop + 1] = 1; 
            }
            else {
                // Adding HIGH to red array. Same logic for loop as above.
                redWave[x + loop + 1] = 1; 
            } 
            count++; // Increase count by 1

            //Keeps track of number of data points, ensures loop breaks when 5000 data points have been generated
            if (count > DATA_POINT) {
                break;
            }
        }

        for(x=waveHigh; x < digital_Waveform; x++){  // Printing 0 for LOWs in the remaining waveform
            if (blinkLed == BLINK_GREEN) {
                // Adding LOW to green array
                greenWave[x + loop + 1] = 0; 
            }
            else {
                //Adding LOW to red array
                redWave[x + loop + 1 ] = 0; 
            }
            count++; // Increase count by 1

            //Keeps track of number of data points, ensures loop breaks when 5000 data points have been generated
            if (count > DATA_POINT) {
                break;
            }
        }

        // Maintain position of array while looping
        loop = loop + digital_Waveform;

        // 0 is technically an extra point, thus this helps create a line for gnuplot by giving value when t=0.
        greenWave[0] = greenWave[1];
        redWave[0] = redWave[1];
    }
    
}

void AppendExcel(int blinkLed, int blinkFrequency, int blinkBrightness) {
    //x is an integer used for loop iterations
    int x = 0;

    // To display the frequency in the csv file for each LED
    if(blinkLed == BLINK_GREEN){
        greenFreq = blinkFrequency;
        greenDuty= blinkBrightness;
    }else{
        redFreq = blinkFrequency;
        redDuty= blinkBrightness;
    }    
    //file pointer used for file handling & creates the csv file
    FILE *fpt = fopen("WaveLED.csv", "w+");

    // Prints green wave and red wave into  csv file
    fprintf(fpt, "Frequency of Green LED: %d\n", greenFreq);
    fprintf(fpt, "Duty Cycle of Green LED: %d\n",greenDuty);
    fprintf(fpt, "Frequency of Red LED: %d\n", redFreq);
    fprintf(fpt, "Duty Cycle of Red LED: %d\n", redDuty);
    fprintf(fpt,"Time, Green LED, Red LED\n");
    for (x = 0; x < DATA_POINT; x++) {
        fprintf(fpt, "%.2f, %6d, %7d\n", (x*0.02), greenWave[x], redWave[x]);
    }

    fclose(fpt);
}

// Function to get a valid integer input within a specified range
int getValidInput(int min, int max) {
    char inputBuffer[256];  // A buffer to store user input
    
    while (1) {
        int choice;
        printf("Enter your choice (%d-%d): ", min, max);
        
        if (fgets(inputBuffer, sizeof(inputBuffer), stdin) != NULL) {
            char* endptr;
            choice = (int)strtol(inputBuffer, &endptr, 10);

            if (*endptr == '\n' || *endptr == '\0') {
                if (choice >= min && choice <= max) {
                    return choice;
                } else {
                    printf("Invalid input. Please enter a number between %d and %d.\n", min, max);
                }
            } else {
                printf("Invalid input. Please enter a valid number between %d and %d.\n", min, max);
            }
        } else {
            printf("Input error. Please try again.\n");
        }
    }
}

/* 
Sets up the LED GPIO pins as output and PWM
*/
void setupProgram() {
    wiringPiSetupGpio();
    pinMode(RED, OUTPUT);
    pinMode(GREEN, OUTPUT);
    softPwmCreate(GREEN, 0, 100);
    softPwmCreate(RED, 0, 100);
    system("clear");
}

/* 
Takes the input of the user selection and direct it to different states of the program
*/
void startProgram() {

    int selection;

    do {
        selection = getUserSelection();

        switch(selection) {

            case TURN_OFF:
                turnOffLeds();
                break;
            case TURN_ON:
                turnOnLeds();
                break;
            case BLINK:
                blink();
                break;
            case BLINK_BOTH:
                blinkBoth();
                break;
            case EXIT:
                break;
            default:
                printf("\nInvalid Input. Try Again...\n");
                break;
        }

    } while (selection != EXIT);
    
    return;
}

/* 
The main menu that determines what the user wants to do
*/
int getUserSelection() {

    int selection;

    printf("\n===== LAD STUDENT DEVICE =====\n");
    printf("\n[0] Turn off both LEDs\n");
    printf("[1] Turn on both LEDs\n");
    printf("[2] Blink LED\n");
    printf("[3] Blink both LED\n"); // New Option
    printf("[4] Exit\n");
    printf("\nYour Selection: ");

    selection = getValidInput(0, 4); // Get a valid selection (0-4)
    return selection;
}

/* 
For troubleshooting, turning off LEDs and PWM
*/
void turnOffLeds() {
    system("clear");
    printf("\nTurning off both LEDs...\n");
    digitalWrite(GREEN, LOW);
    softPwmWrite(GREEN, 0);
    digitalWrite(RED, LOW);
    softPwmWrite(RED, 0);
}

/* 
For troubleshooting, turning on LEDs and PWM at 100
*/
void turnOnLeds() {
    system("clear");
    printf("\nTurning on both LEDs...\n");
    digitalWrite(GREEN, HIGH);
    softPwmWrite(GREEN, 100);
    digitalWrite(RED, HIGH);
    softPwmWrite(RED, 100);
}

/* 
When user wants to blink LED, this function will get all the blinking configuration
It gets from the user the LED to blink, frequency and brightness.
Then, it will call a function to attempt handshake with Monitor before executing the blink
*/
void blink()
{
    while(1) //uses an infinite loop to repeatedly prompt user for their selection and will only exit the loop if the selection is confirmed, avoiding recursion(stackoverflow) 
    {
    system("clear");
    printf("\nBlink...\n");
    
    int blinkLed = getBlinkLed();
    int frequency = getBlinkFrequency();
    int brightness = getBlinkBrightness();

    if (confirmBlinkSelection(blinkLed, frequency, brightness) == CONFIRM) 
    {
        printf("Brightness is %d",brightness);
        blinkLedWithConfig(blinkLed, frequency, brightness); //blinkled based on configuration
        CalculateData(blinkLed, frequency, brightness); //append configuration to datafile
        AppendExcel(blinkLed, frequency, brightness);
        system("clear"); //clear screen
        break; //exit the loop if the selection is confirmed
    }
    }
}

void blinkBoth() {
    // Clear the console screen to start with a clean display
    system("clear");
    
    // Display a message indicating that both LEDs will blink
    printf("\nBlink both LEDs...\n");

    int frequencyGreen, frequencyRed, brightnessGreen, brightnessRed;
    int blinkLedGreen = BLINK_GREEN;  // Set the LED to GREEN
    int blinkLedRed = BLINK_RED;      // Set the LED to RED

    // Prompt the user for configuration values for the GREEN LED
    printf("Enter configuration for GREEN LED:\n");
    frequencyGreen = getBlinkFrequency();   // Get blinking frequency from user
    brightnessGreen = getBlinkBrightness(); // Get brightness level from user

    // Prompt the user for configuration values for the RED LED
    printf("Enter configuration for RED LED:\n");
    frequencyRed = getBlinkFrequency();     
    brightnessRed = getBlinkBrightness();   

    // Check if the user confirms the configuration for both LEDs
    if (confirmBlinkSelection(blinkLedGreen, frequencyGreen, brightnessGreen) == CONFIRM &&
        confirmBlinkSelection(blinkLedRed, frequencyRed, brightnessRed) == CONFIRM) {
        // Both LEDs' configurations are confirmed, so they will blink concurrently.
        
        system("clear");
        printf("\nBlinking both LEDs...\n");

        int ledState = LOW; // Define the initial state of the LEDs

        // Create thread handles for Both LEDs
        pthread_t greenThread, redThread;

        // Create separate ThreadArgs structs for the green and red LEDs
        ThreadArgs greenArgs;
        greenArgs.frequency = frequencyGreen;
        greenArgs.brightness = brightnessGreen;

        ThreadArgs redArgs;
        redArgs.frequency = frequencyRed;
        redArgs.brightness = brightnessRed;

        // Create threads for each LED, passing the respective configuration data
        pthread_create(&greenThread, NULL, blinkGreen, &greenArgs);
        pthread_create(&redThread, NULL, blinkRed, &redArgs);

        // Wait for both threads to finish before proceeding
        pthread_join(greenThread, NULL);
        pthread_join(redThread, NULL);

        // Record data for both LEDs in an Excel file
        CalculateData(BLINK_GREEN, frequencyGreen, brightnessGreen);
        CalculateData(BLINK_RED, frequencyRed, brightnessRed);

        AppendExcel(BLINK_GREEN, frequencyGreen, brightnessGreen);
        AppendExcel(BLINK_RED, frequencyRed, brightnessRed);
    } else {
        // If the configurations are not confirmed, inform the user and return to the main menu
        system("clear");
        printf("Configuration not confirmed. Returning to the main menu.\n");
    }
}


void *blinkGreen(void *arg) {
    // Cast the argument back to ThreadArgs struct
    ThreadArgs *args = (ThreadArgs *)arg;
    
    // Extract frequency and brightness values from the ThreadArgs struct
    int frequency = *(int *)arg;
    int brightnessGreen = args->brightness;

    // Calculate the duration of each on/off cycle based on frequency
    int onOffTime = 1000 / frequency; // Frequency in milliseconds
    
    // Set the number of blinks you desire, 600 for 60 Second 10Hz
    int blinks = 600; 
    
    int ledState = LOW;

    // Loop to control LED blinking
    while (blinks > 0) {
        if (ledState == LOW) {
                // Turn the LED on
                ledState = HIGH;
                softPwmWrite(GREEN, brightnessGreen);
                
                
            } else {
                // Turn the LED off
                ledState = LOW;
                softPwmWrite(GREEN, 0);
            }
        
        // Toggle the LED state (on/off)
        digitalWrite(GREEN, HIGH);
        delay(onOffTime / 2);
        digitalWrite(GREEN, LOW);
        delay(onOffTime / 2);
        
        // Decrement the remaining number of blinks
        blinks--;
    }
}

void *blinkRed(void *arg) {
    // Cast the argument back to ThreadArgs struct
    ThreadArgs *args = (ThreadArgs *)arg;

    // Extract frequency and brightness values from the ThreadArgs struct
    int frequency = *(int *)arg;
    int brightnessRed = args->brightness;

    // Calculate the duration of each on/off cycle based on frequency
    int onOffTime = 1000 / frequency; 
    
    // Set the number of blinks you desire, 600 for 60 Second 10Hz
    int blinks = 600;
    int ledState = LOW;

    // Loop to control LED blinking
    while (blinks > 0) {
        if (ledState == LOW) {
                // Turn the LED on
                ledState = HIGH;
                softPwmWrite(RED, brightnessRed); // Set brightness
                
                
            } else {
                // Turn the LED off
                ledState = LOW;
                softPwmWrite(RED, 0); // Turn off the LED
            }

        // Toggle the LED state (on/off)   
        digitalWrite(RED, HIGH);
        delay(onOffTime / 2);
        digitalWrite(RED, LOW);
        delay(onOffTime / 2);
        
        // Decrement the remaining number of blinks
        blinks--;
    }
}

/* 
Menu to get user selction on LED to blink
*/
int getBlinkLed() {

    int selection;

    printf("\nSelect LED to blink.\n\n");
    printf("[1] Green LED\n");
    printf("[2] Red LED\n");
    printf("\nYour Selection: ");

    selection = getValidInput(1, 2); // Get a valid selection (1-2)

    if (selection != BLINK_GREEN && selection != BLINK_RED) {
        system("clear");
        printf("Invalid Input. Try Again...\n\n");
        getBlinkLed(); 
    } else {
        system("clear");
        return selection;
    }
}

/* 
Menu to get user selction on Frequency to blink
*/
int getBlinkFrequency() {

    int selection;

    printf("Enter frequency to blink.\n\n");
    printf("Enter whole numbers between 0 to 10\n");
    printf("\nFrequency (Hz): ");

    selection = getValidInput(0, 10); // Get a valid selection (0-10)

    if (selection < 0 || selection > 10) {
        system("clear");
        printf("Invalid Input. Try Again...\n\n");
        getBlinkFrequency(); 
    } else {
        system("clear");
        return selection;
    }
}

/* 
Menu to get user selction on LED brightness
*/
int getBlinkBrightness() {

    int selection;

    printf("Select LED brightness during blink.\n\n");
    printf("Enter whole numbers between 0 to 100\n");
    printf("Brightness (%%): ");

    selection = getValidInput(0, 100); // Get a valid selection (0-100)

    if (selection < 0 || selection > 100) {
        system("clear");
        printf("Invalid Input. Try Again...\n\n");
        return getBlinkBrightness(); 
    } else {
        system("clear");
        return selection;
    }

}

/* 
Menu for user to acknowldge the blink configurations input
*/
int confirmBlinkSelection(int blinkLed, int blinkFrequency, int blinkBrightness) {
    
    int selection;
    char blinkLedString[] = "Green";

    if (blinkLed == BLINK_RED) {
        strcpy(blinkLedString, "Red");
    }

    printf("Confirm your blink configrations.\n\n");
    printf("LED to blink: %s\n", blinkLedString);
    printf("Blink Frequency: %dHz\n", blinkFrequency);
    printf("Blink Brightness: %d%%\n\n", blinkBrightness);
    printf("[1] Confirm Configuration\n");
    printf("[0] Return to Home\n");
    printf("\nYour Selection: ");

    scanf("%d", &selection);

    if (selection < 0 || selection > 1) {
        system("clear");
        printf("Invalid Input. Try Again...\n\n");
        confirmBlinkSelection(blinkLed, blinkFrequency, blinkBrightness);
    } else {
        return selection;
    }
}

/* 
Blinks the LED according to the user configuration
*/
void blinkLedWithConfig(int blinkLed, int blinkFrequency, int blinkBrightness) {

    printf("\nBlinking...\n");
    
    // Setting Frequency
    float onOffTime = 1.0f / blinkFrequency * 1000;

    // Setting Blink LED
    if (blinkLed == BLINK_GREEN) {
        blinkLed = GREEN;
    } else blinkLed = RED;

    // Blinking
    unsigned long previousMillis = 0;
    int ledState = LOW;

    for (int blink = 0; blink < 20;)
    {
        unsigned long currentMillis = millis();

        if (currentMillis - previousMillis >= onOffTime) {
            previousMillis = currentMillis;
            if (ledState == LOW) {
                ledState = HIGH;
                softPwmWrite(blinkLed, blinkBrightness);
            } else {
                ledState = LOW;
                softPwmWrite(blinkLed, 0);
            }
            blink++;
            digitalWrite(blinkLed, ledState);
        }
    }

}

/* 
Resetting and cleaning up before safely exiting the program.
*/
void endProgram() {
    system("clear");
    printf("\nCleaning Up...\n");
    // Turn Off LEDs
    digitalWrite(GREEN, LOW);
    digitalWrite(RED, LOW);

    // Turn Off LED Software PWM
    softPwmWrite(GREEN, 0);
    softPwmWrite(RED, 0);

    // Reset Pins to Original INPUT State
    pinMode(GREEN, INPUT);
    pinMode(RED, INPUT);

    printf("Bye!\n\n");
}