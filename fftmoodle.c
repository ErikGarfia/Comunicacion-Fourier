#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define PI 3.1415926535

struct Cabecera {
    char riff[4];
    int tamArchivo;
    char wave[4];
    
    char fmt[4];
    int tamSubBloque;
    short tipoGrabacion;
    short numeroCanales;
    int frecuenciaMuestreo;
    int bps;
    short alineamiento;
    short bpm;

    char data[4];
    int tamSubBloque2;
};

typedef struct Cabecera Cabecera;

FILE* abrirArchivo(char *, int);
void cerrarArchivo(FILE *);

Cabecera *obtenerCabecera(Cabecera *, FILE *);
short *muestrearSenial(FILE *, int, short *);
char *obtenerPie(FILE *, char*, int);

void cabeceraStereo(Cabecera *, FILE *);
void colocarPie(FILE *, char *, int);

int numeroDeMuestras(int, int);
void preparacionFFT(char *, char *);
int esPotenciaDos(int);
Cabecera *modificarCabecera(Cabecera *, int, int);
short *rellenar(short *, int, int);
float *fft(float *, float *, int, float *);
void swap(float *a, float *b);

int main(int argc, char **argv) {
    if(argc != 3) {
        printf("Error en la entrada\n");
        printf("Ejemplo:\n");
        printf("./NombrePrograma Archivo1.wav Archivo2.wav\n");
        exit(EXIT_FAILURE);
    }

    preparacionFFT(argv[1], argv[2]);
}

FILE* abrirArchivo(char *archivo, int modo) {
    FILE* descriptor = NULL;

    if (modo == 1) 
        descriptor = fopen(archivo, "rb"); 
    else 
        descriptor = fopen(archivo, "wb");
    
    if(descriptor == NULL) { 
        printf("No se pudo abrir el archivo\n");
        exit(EXIT_FAILURE);
    }

    return descriptor;
}

void cerrarArchivo(FILE *dArchivo) {
    fclose(dArchivo);
}

Cabecera *obtenerCabecera(Cabecera* c, FILE* dArchivo) {
    fread(c, 44, 1, (FILE *) dArchivo);
    return c;
}

short *muestrearSenial(FILE* dArchivo, int numeroMuestras, short* signal) { 
    fread(signal, sizeof(short) * numeroMuestras, 1, (FILE *) dArchivo);
    return signal;
}

char *obtenerPie(FILE* dArchivo, char* pie, int bytesPie) {
    fread(pie, bytesPie, 1, (FILE *) dArchivo);
    return pie;
}

void cabeceraStereo(Cabecera* c, FILE* dArchivoSalida) {
    c -> tamArchivo = (c -> tamArchivo + c -> tamSubBloque2);
    c -> numeroCanales = 2;
    c -> bps *= 2;
    c -> alineamiento *= 2;
    c -> tamSubBloque2 = (c -> tamSubBloque2 * 2);

    fwrite(c, 44, 1, (FILE *) dArchivoSalida);
}

void colocarPie(FILE* dArchivoSalida, char *pie, int bytesPie) {
    fwrite(pie, sizeof(char) * bytesPie, 1, (FILE *) dArchivoSalida);
}

int numeroDeMuestras(int tamSubBloque2, int bpm) {
    return (tamSubBloque2 / (bpm / 8));
}

void preparacionFFT(char *archivo, char *archivoSalida) {
    FILE *dArchivo = NULL;
    FILE *dArchivoSalida = NULL;
    short* signal = NULL;
    float* signalFFT = NULL;
    Cabecera *c = NULL;
    int numeroMuestras = 0;
    int numeroMuestrasNuevo = 0;
    char* pie = NULL;
    int bytesPie = 0;
    short* muestras = NULL;
    int no = 0;
    float *xr = NULL;
    float *xi = NULL;

    c = malloc(sizeof(Cabecera));

    dArchivo = abrirArchivo(archivo, 1);
    c = obtenerCabecera(c, dArchivo);

    numeroMuestras = numeroDeMuestras(c -> tamSubBloque2, c -> bpm);
    bytesPie = (c -> tamArchivo) - (c -> tamSubBloque2) - 36;
    pie = malloc(sizeof(char) * bytesPie);

    if(!esPotenciaDos(numeroMuestras)) {
        int l = log2(numeroMuestras);
        l += 1;

        numeroMuestrasNuevo = pow(2, l);

        c = modificarCabecera(c, numeroMuestras, numeroMuestrasNuevo);

        signal = malloc(sizeof(short) * numeroMuestrasNuevo);
        signal = muestrearSenial(dArchivo, numeroMuestras, signal);

        signal = rellenar(signal, numeroMuestras, numeroMuestrasNuevo);

        signalFFT = malloc(sizeof(float) * (numeroMuestrasNuevo * 2));
        muestras = malloc(sizeof(short) * (numeroMuestrasNuevo * 2));
        no = numeroMuestrasNuevo;
    } else {
        signal = malloc(sizeof(short) * numeroMuestras);
        signal = muestrearSenial(dArchivo, numeroMuestras, signal);
        
        signalFFT = malloc(sizeof(float) * (numeroMuestras * 2));
        muestras = malloc(sizeof(short) * (numeroMuestras * 2));
        no = numeroMuestras;
    }

    pie = obtenerPie(dArchivo, pie, bytesPie);
    cerrarArchivo(dArchivo);

    xr = malloc(sizeof(float) * no);
    xi = malloc(sizeof(float) * no);

    for(int i = 0; i < no; i++) {
        xr[i] = (float) signal[i];
        xi[i] = 0.0;
    }

    signalFFT = fft(xr, xi, numeroMuestras, signalFFT);

    for(int i = 0; i < no * 2; i++)
        muestras[i] = signalFFT[i];

    dArchivoSalida = abrirArchivo(archivoSalida, 2);
    cabeceraStereo(c, dArchivoSalida);

    fwrite(muestras, c -> tamSubBloque2, 1, (FILE *) dArchivoSalida);
    fwrite(pie, bytesPie, 1, (FILE *) dArchivoSalida);

    cerrarArchivo(dArchivoSalida);

    free(c);
    free(signal);
    free(signalFFT);
    free(pie);
    free(muestras);
    free(xr);
    free(xi);
}

float *fft(float *xr, float *xi, int numeroMuestras, float *signalFFT) {
    int m, n, i, j, k, j1;
    float arg, s, c, tempr, tempi, w;
    
    m = log((float) numeroMuestras) / log(2.0);

    for(i = 0; i < numeroMuestras; ++i) {
        j = 0;
        for(k = 0; k < m; ++k) 
            j = (j << 1) | (1 & (i >> k));

        if(j < i) {
            swap(&xr[i], &xr[j]);
            swap(&xi[i], &xi[j]);
        }
    }

    for(i = 0; i < m; i++) {
        n = w = pow(2.0, (float) i);
        w = PI / n;

        k = 0;

        while(k < (numeroMuestras - 1)) {
            for(j = 0; j < n; j++) {
                arg = -j * w;
                c = cos(arg);
                s = sin(arg);
                j1 = k + j;
                tempr = xr[j1 + n] * c - xi[j1 + n] * s;
                tempi = xi[j1 + n] * c + xr[j1 + n] * s;
                xr[j1 + n] = xr[j1] - tempr;
                xi[j1 + n] = xi[j1] - tempi;
                xr[j1] = xr[j1] + tempr;
                xi[j1] = xi[j1] + tempi; 
            }
            k += 2 * n;
        }
    }
    arg = 1.0 / numeroMuestras;

    for(i = 0; i < numeroMuestras; i++) {
        xr[i] *= arg;
        xi[i] *= arg;

        signalFFT[2 * i] = xr[i];
        signalFFT[(2 * i) + 1] = xi[i];
    }

    return signalFFT;
}

void swap(float *a, float *b) {
    float aux;
    
    aux = *a;
    *a = *b;
    *b = aux; 
}

int esPotenciaDos(int numeroMuestras) { 
    return ((numeroMuestras != 0) && ((numeroMuestras & (numeroMuestras - 1)) == 0)) ? 1 : 0;
}

Cabecera *modificarCabecera(Cabecera *c, int numeroMuestras, int numeroMuestrasNuevo) {
    int m = numeroMuestrasNuevo - numeroMuestras;
    
    c -> tamArchivo += (m * 2);
    c -> tamSubBloque2 += (m * 2);

    return c;
}

short *rellenar(short *signal, int numeroMuestras, int numeroMuestrasNuevo) {   
    for(int i = numeroMuestras; i < numeroMuestrasNuevo; i++)
        signal[i] = 0;

    return signal;
}