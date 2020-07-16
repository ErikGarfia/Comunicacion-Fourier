#include <stdlib.h>
#include <stdio.h>
/*--Esta es la primera versión de las definición  waveHeader y funciones [read|write]WavHeader @ wav_format.h--*/
struct riffHeader{
	short int 
			chunkId[2],
			chunkSize[2],
			format[2],
			subChunk1Id[2],
			subChunk1Size[2],
			audioFormat,
			numChannels;

	unsigned int
			sampleRate;

	int		byteRate;
	short int
			blockAlign,
			bitsPerSample,
			subChunk2Id[2];
	int 	subChunk2Size;
};
typedef struct riffHeader riffHeader;


void readWavHeader(FILE *input, riffHeader * wavData){

	fread(wavData->chunkId,2,2,input);
	fread(wavData->chunkSize,2,2,input);
	fread(wavData->format,2,2,input);
	fread(wavData->subChunk1Id,2,2,input);
	fread(wavData->subChunk1Size,2,2,input);
	fread(&wavData->audioFormat,2,1,input);
	fread(&wavData->numChannels,2,1,input);
	fread(&wavData->sampleRate,4,1,input);
	fread(&wavData->byteRate,4,1,input);
	fread(&wavData->blockAlign,2,1,input);
	fread(&wavData->bitsPerSample,2,1,input);
	fread(&wavData->subChunk2Id,2,2,input);
	fread(&wavData->subChunk2Size,4,1,input);

}

void writeWavHeader(FILE *output, riffHeader * wavData){

	fwrite(wavData->chunkId,2,2,output);
	fwrite(wavData->chunkSize,2,2,output);
	fwrite(wavData->format,2,2,output);
	fwrite(wavData->subChunk1Id,2,2,output);
	fwrite(wavData->subChunk1Size,2,2,output);
	fwrite(&wavData->audioFormat,2,1,output);
	fwrite(&wavData->numChannels,2,1,output);
	fwrite(&wavData->sampleRate,2,2,output);
	fwrite(&wavData->byteRate,4,1,output);
	fwrite(&wavData->blockAlign,2,1,output);
	fwrite(&wavData->bitsPerSample,2,1,output);
	fwrite(&wavData->subChunk2Id,2,2,output);
	fwrite(&wavData->subChunk2Size,4,1,output);
}
/*--/--*/
void downVol(short int * samples,int sample_count,float factor){
	int i;
	double convert;
	float swap;
	for(i=0;i<sample_count;i++){
		convert =  samples[i]*factor;
		swap = convert;

		samples[i]=swap;
	}
}

int main(int argc,char *argv[]){
	riffHeader wavData;
	short int * samples;
	int data_size=sizeof(short int),
		sample_count;
	FILE *input;
	FILE *output;
	

	input=fopen(argv[1],"r");
	output=fopen(argv[2],"w");
	
	if(input==NULL){
		return -1;
	}

	readWavHeader(input,&wavData);
	writeWavHeader(output,&wavData);

	sample_count = (wavData.subChunk2Size/2);

	samples = (short int *)malloc(sample_count*data_size);

	fread(samples,data_size,sample_count,input);

	downVol(samples,sample_count,0.5);

	fwrite(samples,data_size,sample_count,output);



	fclose(input);
	fclose(output);

}