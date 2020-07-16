#include "wav_format.h"

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
		w.im=sin((2*PI*k)/N);

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


int main(int argc, char *argv[]){
	int i,padding;
	wavHeader inputHeader, outputHeader;
	wavSamples inputSamples, outputSamples;
	complex * signal, * spectrum, sample;

	FILE * input, * output;

	input = fopen(argv[1],"r");
	output = fopen(argv[2],"w");

	read_wavHeader(input,&inputHeader);
	print_wavHeader(&inputHeader);

	inputSamples.count = get_sampleCount(&inputHeader);
	padding=getPadding(&inputSamples);

	inputSamples.left = (short int *)malloc((inputSamples.count+padding)*data_size);
	inputSamples.right = (short int *)malloc((inputSamples.count+padding)*data_size);
	
	memset(inputSamples.left,0x00,inputSamples.count+padding);
	memset(inputSamples.right,0x00,inputSamples.count+padding);
	
	read_samples(&inputSamples,input,2);
	fclose(input);
	
	inputSamples.count+=padding;

	memcpy(&outputHeader,&inputHeader,sizeof(inputHeader));

	outputSamples.count = inputSamples.count*2;
	outputSamples.left = (short int *)malloc(inputSamples.count*data_size);
	outputSamples.right = (short int *)malloc(inputSamples.count*data_size);

	outputHeader.subChunk2Size=outputSamples.count*2;

	print_wavHeader(&outputHeader);
	fwrite(&outputHeader,44,1,output);

	signal=(complex *)malloc(inputSamples.count*sizeof(complex));

	for(i=0;i<inputSamples.count;i++){
		signal[i].re=(double)inputSamples.left[i];
		signal[i].im=(double)inputSamples.right[i];
//		printf("[%Lf,%Lf]\n",signal[i].re,signal[i].im);
	}

	spectrum=fft(signal,inputSamples.count);

	for(i=0;i<inputSamples.count;i++){
		sample.re=spectrum[i].re;
		sample.im=spectrum[i].im;
		
		if(sample.re>32767) sample.re=32767;
		if(sample.im>32767) sample.re=32767;
		if(sample.re<(-32768)) sample.re=-32768;
		if(sample.im<(-32768)) sample.im=-32768;

		outputSamples.left[i]=(short int)sample.re;
		outputSamples.right[i]=(short int)sample.im;
		printf("%Lf,%Lf,%d,%d\n",sample.re,sample.im,outputSamples.left[i],outputSamples.right[i]);

	}

	write_stereoSamples(&outputSamples,output);	

	fclose(output);
	return 0;
}
