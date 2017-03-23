//
// Created by kaushik on 23/2/17.
//

#include <iostream>
#include "mpi.h"
#include <strings.h>
#include <cstring>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

void readImage(Mat matrix);

void horizontalStrips(int);

void sendStripstoAllProcesses(int rank);

void writeIntermediatoryImage();

void stichImages();

void cleanUp();

void readImageForStiching(char *inputFile) ;

void writeStichedImage(const char string[17]);

void validateStripImage();




int numberProcesses, WIDTH, HEIGHT, MAX_COLOR, stripSize, stripStart, stripEnd;

unsigned char *fullImage, *stripImage;
unsigned char **stripMatrix;
//short **imageResidual;
//short *fullImageResidual;
char *outFile, *outputFile;
unsigned char *tempBuffer;
Mat stripImageMat;


int main(int argc, char *argv[]) {
    int i, j, bufferInt[20], rank;
    char imageFile[20];
    double t1,t2;
    MPI_Status status;
    MPI_Request *requests;
    MPI_Status *statuses;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numberProcesses);
    t1=MPI_Wtime();
    if (rank == 0) {
        cout << "Number of processes is " << numberProcesses << "\n";
        VideoCapture cap(0);
        Mat frame;
        cap >> frame; // get a new frame from camera
        cvtColor(frame, frame, COLOR_BGR2GRAY);
        //readImage("../res/images.jpg");
        readImage(frame);
        //Now for requests, allocate appropriate memory
        requests = (MPI_Request *) malloc(sizeof(MPI_Request) * numberProcesses);
        statuses = (MPI_Status *) malloc(sizeof(MPI_Status) * numberProcesses);
    }

    //Start Communication
    //cout<<"Communication started";

    MPI_Bcast(&WIDTH, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&HEIGHT, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&MAX_COLOR, 1, MPI_INT, 0, MPI_COMM_WORLD);

    horizontalStrips(rank);
    //printf("\n[Process %d] Image Width: %d X %d => strip size: %d (%d..%d)\n", rank, WIDTH, HEIGHT, stripSize, stripStart, stripEnd);

    stripMatrix = (unsigned char **) malloc(stripSize * sizeof(unsigned char *));
    for (i = 0; i < stripSize; i++) {
        stripMatrix[i] = (unsigned char *) malloc(WIDTH * sizeof(unsigned char));
    }
    sendStripstoAllProcesses(rank);
    stripImageMat = Mat(stripSize, WIDTH, CV_8U, stripMatrix);
    stripImageMat.data=tempBuffer;
    stripImageMat = stripImageMat(Rect(0,0, WIDTH, stripSize));
    /*char winName[] = "window at rank ";
    winName[15] = rank+'0';
    winName[16] = '\0';
    imshow(winName, stripImageMat);
    waitKey(0);*/


    //writeIntermediatoryImage();
    MPI_Barrier(MPI_COMM_WORLD);
    //TODO: Do whatever you wnant to. The intermediatory images are generated in "/res/it". Apply any filter or operation you want.
    t1=MPI_Wtime();
    GaussianBlur(stripImageMat, stripImageMat, Size(7,7), 1.5, 1.5);
    Canny(stripImageMat, stripImageMat, 0, 30, 3);
    t2 = MPI_Wtime();
    cout<<"Finalized Time elapsed "<<t2-t1<<"\n";
    stichImages();

    if (rank==0) writeStichedImage("../res/final.pgm");
    MPI_Barrier(MPI_COMM_WORLD);
    cleanUp();
    MPI_Finalize();
    return 0;
}

void cleanUp() {
    free(outFile);
    free(tempBuffer);
    free(fullImage);
    free(stripMatrix);
}

void stichImages() {
    readImageForStiching(outputFile);
    //validateStripImage();
    MPI_Gather(tempBuffer, WIDTH*stripSize, MPI_UNSIGNED_CHAR, fullImage,WIDTH*stripSize, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
}

void validateStripImage() {
    for(int i=0;i<stripSize; i++) {
        for(int j=0; j<WIDTH; j++) {
            stripMatrix[i][j] = stripImage[i*WIDTH+j];
        }
    }
    writeIntermediatoryImage();
}

void writeStichedImage(const char* outputFile) {
    FILE *fout;
    int i, j;

    if ((fout = fopen(outputFile, "w")) == NULL) {
        perror("Error opening output file");
        exit(1);
    }

    fprintf(fout, "%s\n%d %d\n%d\n", "P2", WIDTH, HEIGHT, 256);
    fprintf(fout, "#Created by Kaushik\n");

    for (i = 0; i < HEIGHT; i++)
        for (j = 0; j < WIDTH; j++)
            fprintf(fout, "%d\n", fullImage[i*WIDTH+j]);
    fclose(fout);
}
void sendStripstoAllProcesses(int rank) {
    tempBuffer = new unsigned char[WIDTH * stripSize];
    MPI_Scatter(fullImage, stripSize * WIDTH, MPI_UNSIGNED_CHAR, tempBuffer, stripSize * WIDTH, MPI_UNSIGNED_CHAR, 0,
                MPI_COMM_WORLD);


    /*cout << "Strip size recieved Process [ " << rank << " ] " << strlen((const char *) tempBuffer) << "\n";
    if (rank == 0) {
        for (int i = 0; i < stripSize; i++) {
            for (int j = 0; j < WIDTH; j++) {
                if (fullImage[i * WIDTH + j] != tempBuffer[i * WIDTH + j]) {
                    cout << "\n Rank = " << rank << "\n";
                    exit(0);
                }
            }
        }
    }*/
    for (int i = 0; i < stripSize; i++) {
        for (int j = 0; j < WIDTH; j++) {
            stripMatrix[i][j] = tempBuffer[i * WIDTH + j];
        }
    }

    outFile = new char[4];
    strcpy(outFile, "outn");
    outFile[3] = rank + '0';
    strcat(outFile, ".pgm");
}

void writeIntermediatoryImage() {
    FILE *fout;
    int i, j;
    outputFile = new char[10];
    strcpy(outputFile, "../res/it/");
    strcat(outputFile, outFile);
    printf("[Process %c] Writing output file\n", outputFile[13]);

    if ((fout = fopen(outputFile, "w")) == NULL) {
        perror("Error opening output file");
        exit(1);
    }

    fprintf(fout, "%s\n%d %d\n%d\n", "P2", WIDTH, stripSize, 256);
    fprintf(fout, "#Created by Kaushik\n");

    for (i = 0; i < stripSize; i++)
        for (j = 0; j < WIDTH; j++)
            fprintf(fout, "%d\n", stripMatrix[i][j]);
    fclose(fout);

    printf("[Process %c]\tFinished writing output file %s\n", outputFile[13], outputFile);
}

void horizontalStrips(int rank) {

    //Now all the processes have the width, height and max colour.
    //What's next is to cut the images into strips.

    //Step 1.  Calculate the strip size.
    stripSize = HEIGHT / numberProcesses;
    //Step 2. define the start and end point of each strip
    stripStart = rank * stripSize;
    stripEnd = stripStart + stripSize - 1;
    //for the last strip to consume all remaining pixels
    if (rank == numberProcesses - 1 && HEIGHT % numberProcesses != 0) {
        stripSize = HEIGHT - stripSize * (numberProcesses - 1);
        stripEnd = HEIGHT - 1;
    }

}

void readImageForStiching(char *inputFile) {
    //printf("I have to read now %s", inputFile);
    free(stripImage);
    stripImage = (unsigned char *) malloc(WIDTH * stripSize * sizeof(unsigned char *));
    int i, j, temp;
    //imshow("here", matrix);
    //waitKey(0);
    uint8_t *myData = stripImageMat.data;
    for (i = 0; i < stripSize; i++)
        for (j = 0; j < WIDTH; j++) {
            stripImage[i * WIDTH + j] = myData[i*WIDTH+j];
        }
    //fclose(fin);
}

void readImage(Mat matrix) {
    int i, j;
    //Mat matrix = imread(inputFile, 0);
    WIDTH = matrix.cols;
    HEIGHT = matrix.rows;
    fullImage = (unsigned char *) malloc(WIDTH * HEIGHT * sizeof(unsigned char *));
    //imshow("here", matrix);
    //waitKey(0);
    unsigned char *myData = matrix.data;
    for (i = 0; i < HEIGHT; i++)
        for (j = 0; j < WIDTH; j++) {
            fullImage[i * WIDTH + j] = myData[i*WIDTH+j];
        }
}