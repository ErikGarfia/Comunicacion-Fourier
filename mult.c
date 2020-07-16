#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "wav_format.h"

void multiply(wavSamples * srcaSample, wavSamples * srcbSample,wavSamples * outputSample, int numChannels){
	wavSamples * top,* bottom;
	int i,k,N=outputSample->count;
	double * sample,max;
	if(srcaSample->count<srcbSample->count){
		top=srcaSample;
		bottom=srcbSample;
	}else {
		top=srcbSample;
		bottom=srcaSample;
	}

	switch(numChannels){
		case 1:
			sample=(double *)malloc(N*sizeof(double));
			for(i=0,k=0;i<N;i++,k++){
				if(k==bottom->count) k=0;
				sample[i]=(double)top->left[i]*bottom->left[k];

				if(fabs(sample[i])>max) max=fabs(sample[i]);
			}
			for(i=0;i<N;i++){
				sample[i]/=max;

				if(sample[i]>0){
					sample[i]*=32767;
					outputSample->left[i]=(short int)sample[i];
					//printf("normalized:%f\t%d\n",sample[i],outputSample->left[i]);					
				}
				else{
					sample[i]*=32768;
					outputSample->left[i]=(short int)sample[i];
					//printf("normalized:%f\t%d\n",sample[i],outputSample->left[i]);					
				}
			}
		break;
		case 2:
			sample=(double *)malloc((N*2)*sizeof(double));
			for(i=0,k=0;i<N;i++,k++){
				sample[i]=(double)((top->left[i]*bottom->left[k])-(top->right[i]*bottom->right[k]));
				sample[i+N]=(double)((top->left[i]*bottom->right[k])+(top->right[i]*bottom->left[k]));


				if(fabs(sample[i])>max) max=fabs(sample[i]);
				else if(fabs(sample[i+N])>max) max=fabs(sample[i+N]);
				if(k+1==bottom->count) k=-1;
			}
			for(i=0;i<N*2;i++){
				sample[i]/=max;
				if(sample[i]>0)
					sample[i]*=32767;
				else
					sample[i]*=32768;

				if(i<N) outputSample->left[i]=(short int)sample[i];
				else outputSample->right[i-N]=(short int)sample[i];
			}
		break;
	}



}

int main(int argc, char const *argv[]){
	wavHeader srcaHeader,srcbHeader,outputHeader;
	wavSamples srcaSample,srcbSample,outputSample;

	int numChannels[3];

	FILE * srca, * srcb,* output;

	srca=fopen(argv[1],"r");
	srcb=fopen(argv[2],"r");
	output=fopen(argv[3],"w");

	read_wavHeader(srca,&srcaHeader);
	numChannels[0]=srcaHeader.numChannels;
	//print_wavHeader(&srcaHeader);
	read_wavHeader(srcb,&srcbHeader);
	numChannels[1]=srcbHeader.numChannels;
	//print_wavHeader(&srcbHeader);

	if(numChannels[0]==2&&numChannels[1]==2){
		numChannels[2]=2;
	}else{
		numChannels[2]=1;
	}

	srcaSample.count=get_sampleCount(&srcaHeader);
	srcbSample.count=get_sampleCount(&srcbHeader);

	if(srcaSample.count<srcbSample.count){
		outputSample.count = srcaSample.count;
		memcpy(&outputHeader,&srcaHeader,sizeof(srcaHeader));
	}else {
		outputSample.count = srcbSample.count;
		memcpy(&outputHeader,&srcbHeader,sizeof(srcbHeader));
	}

	outputHeader.numChannels=numChannels[2];
	fwrite(&outputHeader,44,1,output);

	printf("  out: %d samples\n",outputSample.count);

	alloc_samples(&srcaSample,numChannels[0]);
	alloc_samples(&srcbSample,numChannels[1]);
	alloc_samples(&outputSample,numChannels[2]);

	printf("  leido (A):%d\n",read_samples(&srcaSample,srca,numChannels[0]));
	fclose(srca);
	printf("  leido (B):%d\n",read_samples(&srcbSample,srcb,numChannels[1]));
	fclose(srcb);

	multiply(&srcaSample,&srcbSample,&outputSample,numChannels[2]);

	write_samples(&outputSample,output,numChannels[2]);

	fclose(output);
	return 0;
}