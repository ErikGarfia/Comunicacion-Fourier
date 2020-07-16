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
		w.im=(-1)*sin((2*PI*k)/N);

		spectrum[k].re=A[k].re	+w.re*B[k].re	+w.im*B[k].im;
		spectrum[k].im=A[k].im	+w.re*B[k].im	-w.im*B[k].re;

		spectrum[k+(N/2)].re=A[k].re	-w.re*B[k].re	-w.im*B[k].im;
		spectrum[k+(N/2)].im=A[k].im	-w.re*B[k].im	+w.im*B[k].re;		

	}
	return spectrum;
}


int main(int argc, char *argv[]){
	int i,k,padding,dtmf_index[2][4],sample[2];
	double nTs,max;
	wavHeader inputHeader;
	wavSamples inputSamples;
	complex * signal, * spectrum;
	long double scale;
	char symbols[4][4]={
		{'1','2','3','A'},
		{'4','5','6','B'},
		{'7','8','9','C'},
		{'*','0','#','D'}
	};

	FILE * input;

	input = fopen(argv[1],"r");

	read_wavHeader(input,&inputHeader);

	inputSamples.count = get_sampleCount(&inputHeader);
	padding=getPadding(&inputSamples);

	inputSamples.left = (short int *)malloc((inputSamples.count+padding)*data_size);
	
	memset(inputSamples.left,0x00,inputSamples.count+padding);
	
	fread(inputSamples.left,data_size,inputSamples.count,input);
	fclose(input);
	
	inputSamples.count+=padding;


	/*
		|	1209Hz	|	1336Hz	|	1477Hz	|	1633Hz	|
		|	697Hz	|	770Hz	|	852Hz	|	941Hz	|
	*/

	nTs=inputSamples.count*(1.0/44100.0);
	printf("Input samples:%d\tnTs:%f\n",inputSamples.count,nTs);
	
	dtmf_index[0][0]=floor((697)*nTs);
	dtmf_index[0][1]=floor((770)*nTs);
	dtmf_index[0][2]=floor((852)*nTs);
	dtmf_index[0][3]=floor((941)*nTs);

	dtmf_index[1][0]=floor((1209)*nTs);
	dtmf_index[1][1]=floor((1336)*nTs);
	dtmf_index[1][2]=floor((1477)*nTs);
	dtmf_index[1][3]=floor((1633)*nTs);

	sample[0]=0;
	sample[1]=0;

	signal=(complex *)malloc(inputSamples.count*sizeof(complex));
	scale=(long double)1/inputSamples.count;

	for(i=0;i<inputSamples.count;i++){
		signal[i].re=(double)inputSamples.left[i];
		signal[i].im=0.0;
	}

	spectrum=fft(signal,inputSamples.count);

	for(i=0;i<2;i++){
		for(k=0;k<4;k++){
			printf("%.0f\t",dtmf_index[i][k]/nTs);
		}
	}
	printf("\n");
	for(i=0;i<2;i++){
		max=0;
		for(k=0;k<4;k++){
			printf("%.2Lf\t",spectrum[dtmf_index[i][k]].re*scale);
			if(spectrum[dtmf_index[i][k]].re*scale>max){
				max=spectrum[dtmf_index[i][k]].re*scale;
				sample[i]=k;
			}
		}
	}

	printf("\n\nfreqs(%.0f,%.0f)=%c\n",
		dtmf_index[0][sample[0]]/nTs,
		dtmf_index[1][sample[1]]/nTs,
		symbols[sample[0]][sample[1]]);

	return 0;
}