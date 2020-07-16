#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#define NFILTER 100
const 	double PI = 4. * atan(1.);
const 	int data_size = sizeof(short int);

typedef struct wavHeader{
	unsigned char 	chunkId[4];
	unsigned int	chunkSize;
	unsigned char 	format[4],
					subChunk1Id[4];
	unsigned int 	subChunk1Size;
	unsigned short	audioFormat,
					numChannels;
	unsigned int	sampleRate,
					byteRate;
	unsigned short	blockAlign,
					bitsPerSample;
	unsigned char 	subChunk2Id[4];
	int				subChunk2Size;
} wavHeader;

typedef struct wavSamples{
	short int
		* left,
		* right;
	int count;
} wavSamples;


void genRC(double filter[],int n,int fc);
void convolute(wavSamples * inputSamples, double h[], int h_count);
int read_wavHeader(FILE *input, wavHeader * wavData);
void convolute(wavSamples * inputSamples, double h[], int h_count);

int main(int argc, char const *argv[]){

	double filter[NFILTER];
	//genRC(arreglo de muestras del filtro, número de muestras del filtro, frecuencia de corte)
	genRC(filter,NFILTER,2000);
	FILE * input, * output;

	wavHeader inputHeader;
	wavSamples inputSamples;

	input = fopen(argv[1],"r");
	output = fopen(argv[2],"w");
	
	read_wavHeader(input,&inputHeader);

	fwrite(&inputHeader,44,1,output);
	
	inputSamples.count = (8*inputHeader.subChunk2Size)/(inputHeader.numChannels*inputHeader.bitsPerSample);
	inputSamples.left = (short int *)malloc(inputSamples.count*data_size);

	fread(inputSamples.left,data_size,inputSamples.count,input);

	convolute(&inputSamples,filter,NFILTER);

	fwrite(inputSamples.left,data_size,inputSamples.count,output);

	fclose(input);
	fclose(output);

	return 0;
}

/*-Genera las NFILTER muestras del filtro-*/
void genRC(double filter[],int n,int fc){
	double kernel,impulse,ts;
	int sampleRate,i;

	kernel = 1/(2*fc*PI);
	sampleRate = 44100;
	ts=1/(double)sampleRate;
	
	for(i=0;i<n;i++){
		impulse = -(1)/(kernel);
		impulse=impulse*i*ts;
		filter[i]= exp(impulse);
	}
}
/*--Convoolución entre filtro y muestras del archivo leído */
void convolute(wavSamples * inputSamples, double h[], int h_count){
	int n,k,sampleCount = inputSamples->count;
	double y[sampleCount],max_y=0,min_y=0;
	short int x;

	for(n=0;n<sampleCount;n++){
		y[n]=0;	
		for(k=0;k<h_count;k++){
			if(n-k>=0){
				y[n]+=inputSamples->left[n-k]*h[k];
			}
		}
		
		if(y[n]<0){
			min_y=(-1)*y[n];
			if(min_y>max_y) max_y = min_y;
		}else if(y[n]>max_y) max_y = y[n];
	}
	for(n=0;n<sampleCount;n++){
		x =(short int)(y[n]*32767.0/max_y);
		inputSamples->left[n]=x;
	}

}

int read_wavHeader(FILE *input, wavHeader * wavData){
	int bcount=0;
		bcount+=fread(wavData->chunkId,4,1,input)*4;
		bcount+=fread(&wavData->chunkSize,4,1,input)*4;
		bcount+=fread(wavData->format,4,1,input)*4;
		bcount+=fread(wavData->subChunk1Id,4,1,input)*4;		
		bcount+=fread(&wavData->subChunk1Size,4,1,input)*1;		
		bcount+=fread(&wavData->audioFormat,2,1,input)*2;
		bcount+=fread(&wavData->numChannels,2,1,input)*2;
		bcount+=fread(&wavData->sampleRate,4,1,input)*4;
		bcount+=fread(&wavData->byteRate,4,1,input)*4;
		bcount+=fread(&wavData->blockAlign,2,1,input)*2;
		bcount+=fread(&wavData->bitsPerSample,2,1,input)*2;		
		bcount+=fread(&wavData->subChunk2Id,4,1,input)*4;		
		bcount+=fread(&wavData->subChunk2Size,2,2,input)*2;
	return bcount;
}