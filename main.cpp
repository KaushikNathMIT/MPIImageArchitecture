#include <iostream>
#include "mpi.h"
#include <values.h>
#include <math.h>
#include <cstdio>
#include <cstdlib>
#include <strings.h>
#include <vector>

void readImage(char *imageFile);

void horizontalStrips(int);

using namespace std;


int numberProcesses, WIDTH, HEIGHT, MAX_COLOR, stripSize, stripStart, stripEnd;

unsigned char* fullImage;
unsigned char** imageStrip;
short **imageResidual;
short *fullImageResidual;

int main(int argc, char *argv[]) {
    int i , j, bufferInt[20], rank;
    char imageFile[20];

    MPI_Status status;
    MPI_Request *requests;
    MPI_Status *statuses;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numberProcesses);

    if(rank == 0) {
        cout<<"Number of processes is "<<numberProcesses<<"\n";
        cout << "Enter file Name!\n";
        scanf(" %s", imageFile);
        readImage(imageFile);
        //Now for requests, allocate appropriate memory
        requests = (MPI_Request *) malloc(sizeof(MPI_Request) * numberProcesses);
        statuses = (MPI_Status *) malloc(sizeof(MPI_Status) * numberProcesses);
        /*for(int i=0;i<HEIGHT;i++,printf("\n"))
            for(int j=0;j<WIDTH;j++)
                printf("%3d ",fullImage[i*WIDTH+j]);*/
    }

    //Start Communication
    //cout<<"Communication started";

    MPI_Bcast(&WIDTH, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&HEIGHT, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&MAX_COLOR, 1, MPI_INT, 0, MPI_COMM_WORLD);

    horizontalStrips(rank);

    printf("\n[Proces %d] Image Width: %d X %d => strip size: %d (%d..%d)\n",
           rank, WIDTH, HEIGHT, stripSize, stripStart, stripEnd);

    MPI_Finalize();
    return 0;
}

void horizontalStrips(int rank) {

    //Now all the processes have the width, height and max colour.
    //What's next is to cut the images into strips.

    //Step 1.  Calculate the strip size.
    stripSize = HEIGHT/numberProcesses;
    //Step 2. define the start and end point of each strip
    stripStart = rank*stripSize;
    stripEnd = stripStart + stripSize - 1;
    //for the last strip to consume all remaining pixels
    if (rank == numberProcesses - 1 && HEIGHT % numberProcesses != 0) {
        stripSize = HEIGHT - stripSize * (numberProcesses - 1);
        stripEnd = HEIGHT - 1;
    }
}

void readImage(char *inputFile) {
    FILE *fin;
    char data[70];

    if ((fin = fopen(inputFile, "r")) == NULL) {
        perror("Error opening input file");
        exit(1);
    }

    printf("Reading input file HEADER:\n");

    // Reading format
    fscanf(fin, "%s", data);
    while (data[0] == '#') // ignoring comments
    {
        printf("\tFound comment: %s", data);
        fgets(data, 70, fin);
        printf("%s", data);
        fscanf(fin, "%s", data);
    }
    if (strcasecmp(data, "P2") != 0) {
        printf("Illegal format type of input file. Expected encoding: \"P2\". "
                       "Found encoding \"%s\"\n",
               data);
        exit(1);
    } else
        printf("\tFile format found for %s: %s\n", inputFile, data);

    // Reading Width
    fscanf(fin, "%s", data);
    while (data[0] == '#') // ignoring comments
    {
        printf("\tFound comment: %s", data);
        fgets(data, 70, fin);
        printf("%s", data);
        fscanf(fin, "%s", data);
    }
    sscanf(data, "%d", &WIDTH);
    printf("\tImage width: %d\n", WIDTH);

    // Reading Height
    fscanf(fin, "%s", data);
    while (data[0] == '#') // ignoring comments
    {
        printf("\tFound comment: %s", data);
        fgets(data, 70, fin);
        printf("%s", data);
        fscanf(fin, "%s", data);
    }
    sscanf(data, "%d", &HEIGHT);
    printf("\tImage height: %d\n", HEIGHT);

    // Reading MAX_COLOR
    fscanf(fin, "%s", data);
    while (data[0] == '#') // ignoring comments
    {
        printf("\tFound comment: %s", data);
        fgets(data, 70, fin);
        printf("%s", data);
        fscanf(fin, "%s", data);
    }
    sscanf(data, "%d", &MAX_COLOR);
    printf("\tImage number of colors: %d\n", MAX_COLOR);

    // Reading data

    printf("Reading input file DATA.\n");
    fullImage = (unsigned char*) malloc(WIDTH * HEIGHT * sizeof(unsigned char *));
    int i, j, temp;

    for (i = 0; i < HEIGHT; i++)
        for (j = 0; j < WIDTH; j++) {
            fscanf(fin, "%d", &temp);
            fullImage[i * WIDTH + j] = temp;
        }
    fclose(fin);
}