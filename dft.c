#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

const 	double PI = 4. * atan(1.);
const 	int data_size = sizeof(short int);

typedef struct complex{
	double re,im;
}complex;

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

void print_byteAsString(unsigned char [], int,char *);
void print_wavHeader(wavHeader * );

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

void convert_monoStereo(wavHeader * header){
	header->numChannels++;
	header->byteRate = (header->sampleRate*header->numChannels*header->bitsPerSample)/8;
	header->blockAlign =(header->numChannels*header->bitsPerSample)/8;
	header->subChunk2Size*=2;
}

int get_sampleCount(wavHeader * header){
	return (8*header->subChunk2Size)/(header->numChannels*header->bitsPerSample);
}

int write_stereoSamples(wavSamples * outputSamples, FILE * output){
	int i,N = outputSamples->count, count=0;
	for(i=0;i<N/2;i++){
		count+=fwrite(&outputSamples->left[i],data_size,1,output);
		count+=fwrite(&outputSamples->right[i],data_size,1,output);		
	}

	return count;
}

int read_stereoSamples(wavSamples * inputSamples, FILE * input){
	int i,N = inputSamples->count, count=0;
	for(i=0;i<N/2;i++){
		count+=fread(&inputSamples->left[i],data_size,1,input);
		count+=fread(&inputSamples->right[i],data_size,1,input);
	}
	return count;
}

typedef struct twiddle_vector{
	double * nth;
	int n;
}twiddle_vector;

void gen_twiddle(twiddle_vector * factor,int N){
	int i;
	double w = (2*PI)/N;
	factor->n = N;
	factor->nth = (double *)malloc(N*sizeof(double));

	for(i=0;i<N;i++) factor->nth[i]=w*i;
}

void dft_samples(short int * samples, twiddle_vector * factor, int N, wavSamples * outputSamples){
	int n,k,nk,lim;
	double scale=1,* re, *im;
	scale/=N;
	re = (double *)malloc(N*sizeof(double));
	im = (double *)malloc(N*sizeof(double));

	for(k=0;k<N;k++){
		lim=0;
		re[k]=0;
		im[k]=0;		
		for(n=0;n<N;n++){
			nk=n*k;
			if(nk>=N*(lim+1)) lim++;
			re[k]+=scale*samples[n]*cos(factor->nth[nk-(N*lim)]);
			im[k]+=(-1*scale)*samples[n]*sin(factor->nth[nk-(N*lim)]);	
		}
		outputSamples->left[k] = re[k];
		outputSamples->right[k] = im[k];
	}
	
}

int main(int argc, char const *argv[]){
	wavHeader inputHeader, outputHeader;
	wavSamples inputSamples, outputSamples;

	twiddle_vector factor;
	FILE * input, * output;

	input = fopen(argv[1],"r");
	output = fopen(argv[2],"w");
	
	read_wavHeader(input,&inputHeader);

	memcpy(&outputHeader,&inputHeader,sizeof(inputHeader));

	convert_monoStereo(&outputHeader);
	
	fwrite(&outputHeader,44,1,output);

	inputSamples.count = get_sampleCount(&inputHeader);
	outputSamples.count = inputSamples.count*2;

	inputSamples.left = (short int *)malloc(inputSamples.count*data_size);
	outputSamples.left = (short int *)malloc(inputSamples.count*data_size);
	outputSamples.right = (short int *)malloc(inputSamples.count*data_size);

	fread(inputSamples.left,data_size,inputSamples.count,input);
	fclose(input);
	
	gen_twiddle(&factor,inputSamples.count);
	dft_samples(inputSamples.left,&factor,inputSamples.count,&outputSamples);

	write_stereoSamples(&outputSamples,output);	

	fclose(output);

	return 0;
}