#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

/*
	network byte order -> big endian
	host byte order -> litle endian
*/

const 	double PI = 4. * atan(1.);
const 	int data_size = sizeof(short int);

typedef struct complex{
	long double re,im;
}complex;

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
	int				subChunk2Size;	/* Número de muestras * 2. Cada muestra es de 2 bytes*/
} wavHeader;


/* Canales*/

typedef struct wavSamples{
	short int
		* left,	
		* right;
	int count;
} wavSamples;


/*Lee los primeros 44 bytes del archivo .wav*/
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

/* Imprime arreglo de bytes como cadena */
void print_byteAsString(unsigned char src[], int count,char * end){
	int i;
	for(i=0;i<count;i++){
		printf("%c",src[i]);
	}
	printf("%s",end);
}

/* Imprime los primeros 44 bytes del .wav leído*/
void print_wavHeader(wavHeader * wavData){
	printf("  chunkId:");				print_byteAsString(wavData->chunkId,4,"\n");
	printf("  chunkSize: %d\n",			wavData->chunkSize);
	printf("  format: ");				print_byteAsString(wavData->format,4,"\n");
	printf("  subChunk1Id: ");			print_byteAsString(wavData->subChunk1Id,4,"\n");
	printf("  subChunk1Size: %d\n",		wavData->subChunk1Size);
	printf("  audioFormat: %d\n",		wavData->audioFormat);
	printf("  numChannels: %d\n",		wavData->numChannels);
	printf("  sampleRate: %d\n",		wavData->sampleRate);
	printf("  byteRate: %d\n",			wavData->byteRate);
	printf("  blockAlign: %d\n",		wavData->blockAlign);
	printf("  bitsPerSample: %d\n",		wavData->bitsPerSample);
	printf("  subChunk2Id: ");			print_byteAsString(wavData->subChunk2Id,4,"\n");
	printf("  subChunk2Size: %d\n\n",	wavData->subChunk2Size);
}

/*--Reserva memoria para las muestras--*/
void alloc_samples(wavSamples * sample, int numChannels){
	sample->left=(short int *)malloc(sample->count*data_size);
	if(numChannels==2)
		sample->right=(short int *)malloc(sample->count*data_size);
}
/* Escribe un conjunto de muestras en formato estéreo: [muestra canal izquierdo][muestra canal derecho] */
int write_stereoSamples(wavSamples * outputSamples, FILE * output){
	int i,N = outputSamples->count, count=0;
	for(i=0;i<N/2;i++){
		count+=fwrite(&outputSamples->left[i],data_size,1,output);
		count+=fwrite(&outputSamples->right[i],data_size,1,output);		
	}

	return count;
}

int write_samples(wavSamples * sample,FILE * output,int numChannels){
	int i,N = sample->count, count=0;
	if(numChannels==2){
		for(i=0;i<N;i++){
			count+=fwrite(&sample->left[i],data_size,1,output);
			count+=fwrite(&sample->right[i],data_size,1,output);
		//	printf("[%d][%d]\n",sample->left[i],sample->right[i]);
		}
	}
	else {
		for(i=0;i<N;i++){
			count+=fwrite(&sample->left[i],data_size,1,output);
		}
	}

	return count;
}

/* Lee archivo monoaural o esteréo */
int read_samples(wavSamples * sample,FILE * input,int numChannels){
	int i,N = sample->count, count=0;
	if(numChannels==2){
		for(i=0;i<N;i++){
			count+=fread(&sample->left[i],data_size,1,input);
			count+=fread(&sample->right[i],data_size,1,input);
		}
	}
	else {
		for(i=0;i<N;i++){
			count+=fread(&sample->left[i],data_size,1,input);
		}
	}

	return count;
}


/*
*	Recibe el header de un archivo wav mono y lo ocnvierte a esteréo
*	header: estructura de tipo wavHeader con la los primeros 44 bytes de un .wav leídos
*/

void convert_monoStereo(wavHeader * header){
	header->numChannels++;
	header->byteRate = (header->sampleRate*header->numChannels*header->bitsPerSample)/8;
	header->blockAlign =(header->numChannels*header->bitsPerSample)/8;
	header->subChunk2Size*=2;
}

/*	Dada la información leída del .wav regresa el número exacto de muestras */

int get_sampleCount(wavHeader * header){
	return (8*header->subChunk2Size)/(header->numChannels*header->bitsPerSample);
}

/* Lee un conjunto de muestras en formato estéreo: [muestra canal izquierdo][muestra canal derecho] */
int read_stereoSamples(wavSamples * inputSamples, FILE * input){
	int i,N = inputSamples->count, count=0;
	for(i=0;i<N/2;i++){
		count+=fread(&inputSamples->left[i],data_size,1,input);
		count+=fread(&inputSamples->right[i],data_size,1,input);
	}

	return count;
}

void fileName_setPrefix(char * proc,char * file,char * zname){
	char *chunk;

	chunk = strtok(proc,"/");
	chunk = strtok(NULL,"\0");

	strcpy(zname,chunk);
	strcat(zname,"_");

	chunk = strtok(file,"/");
	if(strcmp(chunk,file)==0){
		strcat(zname,chunk);
	}else {
		chunk = strtok(NULL,"\0");
		strcat(zname,chunk);
	}

}


/* Calcula el logaritmo base 2 de x */
double log2(double x){
	return (log(x))/(log(2));
}
/* Cálcula el padding para completar el número de muestras potencia 2 */
int getPadding(wavSamples * inputSamples){
	int N;
	long int padding=0;
	double n;

	N = inputSamples->count;

	n = ceil(log2(N));
	n = pow(2,n);

	if(N<n) padding=n-N;
	return padding;
}

void double_printArray(double * array,int begin,int end,char * format){
	int i;
	for(i=begin;i<end;i++){
		printf(format,i,array[i]);
	}
}

//FFT
void split_samples(complex * samples,complex * even,complex * odd, int N){
	int i;
	for(i=0;i<N;i++){
		even[i].re = samples[2*i].re;
		odd[i].re = samples[(2*i)+1].re;
		even[i].im = samples[2*i].im;
		odd[i].im = samples[(2*i)+1].im;
	}

}

complex * fft(complex * signal,int N){
	int k;
	complex w,even[N/2],odd[N/2],*A,*B,*spectrum;

	if(N==1) return signal;

	split_samples(signal,even,odd,N/2);
	A=(complex *)malloc((N/2)*sizeof(complex));
	B=(complex *)malloc((N/2)*sizeof(complex));
	spectrum=(complex *)malloc(N*sizeof(complex));

	A=fft(even,N/2);
	B=fft(odd,N/2);

	for(k=0;k<N/2;k++){
		w.re=cos((2*PI*k)/N);
		w.im=(-1)*sin((2*PI*k)/N);

		spectrum[k].re=A[k].re	+w.re*B[k].re	+w.im*B[k].im;
		spectrum[k].im=A[k].im	+w.re*B[k].im	-w.im*B[k].re;

		spectrum[k+(N/2)].re=A[k].re	-w.re*B[k].re	-w.im*B[k].im;
		spectrum[k+(N/2)].im=A[k].im	-w.re*B[k].im	+w.im*B[k].re;		

/*		printf("[%d:%d],[%Lf,%Lf]\t[%Lf,%Lf]\n",N,k,
			spectrum[k].re,spectrum[k].im,
			spectrum[k+(N/2)].re,spectrum[k+(N/2)].im
			);
*/	}
	return spectrum;
}


