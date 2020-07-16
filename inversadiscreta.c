

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

const double PI = 4. * atan(1.);
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


typedef struct twiddle_vector{
	double * nth;
	int n;
}twiddle_vector;

void gen_twiddle(twiddle_vector * factor,int N);
void idft_samples(wavSamples * inputSamples, twiddle_vector * factor, int N, wavSamples * outputSamples);
int read_wavHeader(FILE *input, wavHeader * wavData);
int get_sampleCount(wavHeader * header);
int write_stereoSamples(wavSamples * outputSamples, FILE * output);
int read_stereoSamples(wavSamples * inputSamples, FILE * input);

void idft_samples(wavSamples * inputSamples, twiddle_vector * factor, int N, wavSamples * outputSamples){
	int n,k,nk,lim;
	double * re, *im,nkfactor;
	re = (double *)malloc(N*sizeof(double));
	im = (double *)malloc(N*sizeof(double));

	for(k=0;k<N;k++){
		lim=0;
		re[k]=0;
		im[k]=0;		
		for(n=0;n<N;n++){
			nk=n*k;
			if(nk>=N*(lim+1)) lim++;

			nkfactor=factor->nth[nk-(N*lim)];	
			re[k]+=inputSamples->left[n]*cos(nkfactor)-(inputSamples->right[n]*sin(nkfactor));
			im[k]+=inputSamples->right[n]*cos(nkfactor)+(inputSamples->left[n]*sin(nkfactor));

		}

		
		if(re[k]>32767) {
			re[k]=32767;
		}

		if(im[k]>32767){
			 im[k]=32767;
		}

		if(re[k]<(-32768)){
			re[k]=-32768;
		}
		if(im[k]<(-32768)){
			 im[k]=-32768;
		}


		outputSamples->left[k] = re[k];
		outputSamples->right[k] = im[k];
		
		
	}
	
}


int main(int argc,char *argv[]){
	wavHeader inputHeader,outputHeader;
	wavSamples inputSamples,outputSamples;
	FILE *input,*output;
	twiddle_vector factor;

	input=fopen(argv[1],"r");
	output=fopen(argv[2],"w");
	
	if(input==NULL){
		return -1;
	}

	read_wavHeader(input,&inputHeader);
	memcpy(&outputHeader,&inputHeader,sizeof(inputHeader));
	fwrite(&outputHeader,44,1,output);
	

	inputSamples.count = get_sampleCount(&inputHeader);
	outputSamples.count = inputSamples.count;	

	inputSamples.left = (short int *)malloc(inputSamples.count*data_size);
	inputSamples.right = (short int *)malloc(inputSamples.count*data_size);
	outputSamples.left = (short int *)malloc(inputSamples.count*data_size);
	outputSamples.right = (short int *)malloc(inputSamples.count*data_size);

	read_stereoSamples(&inputSamples,input);
	fclose(input);

	gen_twiddle(&factor,inputSamples.count);
	idft_samples(&inputSamples,&factor,inputSamples.count,&outputSamples);

	write_stereoSamples(&outputSamples,output);	

	fclose(output);
}


void gen_twiddle(twiddle_vector * factor,int N){
	int i;
	double w = (2*PI)/N;
	factor->n = N;
	factor->nth = (double *)malloc(N*sizeof(double));

	for(i=0;i<N;i++) factor->nth[i]=w*i;

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

int get_sampleCount(wavHeader * header){
	int N = (8*header->subChunk2Size)/(header->numChannels*header->bitsPerSample);
	return N;
}

int write_stereoSamples(wavSamples * outputSamples, FILE * output){
	int i,N = outputSamples->count, count=0;

	for(i=0;i<N;i++){
		count+=fwrite(&outputSamples->left[i],data_size,1,output);
		count+=fwrite(&outputSamples->right[i],data_size,1,output);		
	}

	return count;
}

int read_stereoSamples(wavSamples * inputSamples, FILE * input){
	int i,N = inputSamples->count, count=0;
	for(i=0;i<N;i++){
		count+=fread(&inputSamples->left[i],data_size,1,input);
		count+=fread(&inputSamples->right[i],data_size,1,input);
	}

	return count;
}