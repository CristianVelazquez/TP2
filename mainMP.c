#include <stdio.h>
#include "simple_bmp.h"
#include <math.h>
#include <time.h>
#include <omp.h>
#define TAM 120


/* Variables globales */
void kernel_setup(uint16_t **kern, int16_t ksize);

char path1[] = "/home/cristian/Documentos/TP2/base.bmp";
//char path1[] = "/home/SO2/Alumno26/base.bmp";
char path2[]="/home/cristian/Imágenes/testeo.bmp";
//char path2[]="/home/SO2/Alumno26/TP2CRISTIAN/bin/testeo.bmp";

uint16_t sumatoria = 0;
uint32_t alto = 0, ancho = 0;//0 valor por defecto
int kk = 1, l = 0;
int16_t SIZE_K = 0;
u_int32_t radio = 0;

/* Declaración de funciones */
uint64_t rdtsc();

void recvDatosUsuario();

int main() {
    //int fila = 0, columna = 0;
    int centro1, centro2;
    uint32_t blue = 0, green = 0, red = 0;
    uint32_t valorker;
    sbmp_image imgOld = {0};
    sbmp_image imgNew = {0};

    recvDatosUsuario();
    centro1 = (int) alto / 2;
    centro2 = (int) ancho / 2;

    rdtsc();
    uint16_t **kernel = calloc((unsigned long) SIZE_K, sizeof(int *));
    for (int k = 0; k < SIZE_K; k++)
        kernel[k] = calloc((unsigned long) SIZE_K, sizeof(uint16_t));
    kernel_setup(kernel, SIZE_K);

    if (sbmp_load_bmp(path1, &imgOld) != SBMP_OK) {
        exit(-1);
    }

    int32_t check = sbmp_initialize_bmp(&imgNew, (uint32_t) alto, (uint32_t) ancho);
    if (SBMP_OK != check) {
        perror("No se pudo crear la imagen ");
        exit(-1);
    }

    int i, j;
    int a, b;

    uint32_t blue1 = 0, green1 = 0, red1 = 0;
    clock_t cl = clock();
    #pragma omp parallel for collapse(2) reduction(+:blue1) reduction(+:green1) reduction(+:red1)
    for ( i = 0; i < imgNew.info.image_height - 1; ++i) {
        for ( j = 0; j < imgNew.info.image_width - 1; ++j) {
            if((i<=centro1*2 && j<=centro2*2) && (pow(( i - centro1), 2) + pow((j - centro2), 2) <= pow(radio, 2))){
                red1= (uint32_t) (imgOld.data[i][j].red * kk + l);
                blue1= (uint32_t) (imgOld.data[i][j].blue * kk + l);
                green1= (uint32_t) (imgOld.data[i][j].green * kk + l);
                if(blue1>255){
                    blue1=255;
                }
                if(red1>255){
                    red1=255;
                }
                if(green1>255){
                    green1=255;
                }
                imgNew.data[i][j] = (sbmp_raw_data) {(u_int8_t) blue1,(u_int8_t) green1,(u_int8_t) red1};
            }
           else{
                if (i <= imgNew.info.image_height - (SIZE_K)) {
                    if (j <= imgNew.info.image_width - (SIZE_K)) {

#pragma omp parallel for collapse(2) reduction(+:blue) reduction(+:green) reduction(+:red) private(valorker)
                    for (a = i; a < SIZE_K + i; ++a) {
                        for (b = j; b < j + SIZE_K; ++b) {

                                valorker= (uint8_t) kernel[a-i][b-j];
                                blue=  (blue+(uint32_t)(imgOld.data[a][b].blue * valorker));
                                green= (green+(uint32_t) (imgOld.data[a][b].green * valorker));
                                red= (red+(uint32_t)(imgOld.data[a][b].red * valorker));
                        }
                    }
                    imgNew.data[i][j] = (sbmp_raw_data) {(u_int8_t) (blue/sumatoria),(u_int8_t) (green/sumatoria),(u_int8_t) (red/sumatoria)};
                    blue=0;
                    red=0;
                    green=0;
                }
                }
            }

        }
    }

printf("%ld",(clock()-cl)*1000/CLOCKS_PER_SEC);
    int32_t check2 = sbmp_save_bmp(path2, &imgNew);
    if (SBMP_OK != check2) {
        perror("No se puedo guardar la imagen");
        exit(-1);
    }
    return 0;
}

enum sbmp_codes sbmp_initialize_bmp(sbmp_image *image, uint32_t height, uint32_t width) {

    // Falta chequeo > 32bits, of
    if (image == NULL || height == 0 || width == 0) {
        return SBMP_ERROR_PARAM;
    }

    // Headerd
    image->type.file_type = TIFF_MAGIC_NUMBER;
    image->type.data_offset = sizeof(sbmp_ftype_data) + BITMAPINFOHEADER; // arreglar sizeof + size of
    image->type.file_size = (uint32_t) image->type.data_offset;
    image->type.file_size += (((uint32_t) sizeof(sbmp_raw_data)) * width + width % 4) * height;
    image->type.reserved = 0;

    image->info.header_size = BITMAPINFOHEADER;
    image->info.image_width = (int32_t) width;
    image->info.image_height = (int32_t) height;
    image->info.planes = 1;
    image->info.bit_per_pixel = 24;
    image->info.compression = 0;
    image->info.image_size = 0;
    image->info.xpix_per_meter = 0;
    image->info.ypix_per_meter = 0;
    image->info.total_colors = 0;
    image->info.important_colors = 0;

    image->data = calloc(height, sizeof(sbmp_raw_data *));
    if (image->data == NULL) {
        fprintf(stderr, "Error alocando memoria");
        exit(EXIT_FAILURE);
    }

    for (uint32_t i = 0; i < height; i++) {
        image->data[i] = calloc(width, sizeof(sbmp_raw_data));
        if (image->data[i] == NULL) /* Meeh ?*/
        {
            fprintf(stderr, "Error alocando memoria");
            exit(EXIT_FAILURE);
        }
    }

    return SBMP_OK;
}

enum sbmp_codes sbmp_load_bmp(const char *filename, sbmp_image *image) {

    FILE *fd = fopen(filename, "r");
    if (fd == NULL) {
        fprintf(stderr, "Error: %s\n", strerror(errno));
        return SBMP_ERROR_FILE;
    }

    size_t result1=fread(&image->type, sizeof(image->type), 1, fd);
    if (result1 > 0){

    }
    size_t result2= fread(&image->info, sizeof(image->info), 1, fd);
    if (result2 > 0){

    }
    image->data = calloc((size_t) image->info.image_height, sizeof(sbmp_raw_data *));
    if (image->data == NULL) {
        fprintf(stderr, "Error: %s\n", strerror(errno));
        return SBMP_ERROR_FILE;
    }

    for (int32_t i = image->info.image_height - 1; i >= 0; i--) {
        image->data[i] = calloc((size_t) image->info.image_width, sizeof(sbmp_raw_data));
        size_t result=fread(image->data[i], sizeof(sbmp_raw_data), (uint32_t) image->info.image_width, fd);
        if (result > 0){

        }
    }
    fclose(fd);
    return SBMP_OK;
}

enum sbmp_codes sbmp_save_bmp(const char *filename, const sbmp_image *image) {

    FILE *fd = fopen(filename, "w");
    if (fd == NULL) {
        fprintf(stderr, "Error: %s\n", strerror(errno));
        return SBMP_ERROR_FILE;
    }

    // Write the headers
    fwrite(&image->type, sizeof(image->type), 1, fd);
    fwrite(&image->info, sizeof(image->info), 1, fd);

    // Padding is necessary?
    size_t padd_size = ((size_t) image->info.image_width * sizeof(sbmp_raw_data)) % PADDINGSIZE;
    uint8_t *zero_pad = NULL;

    if (0 != padd_size) { // Yes
        padd_size = PADDINGSIZE - padd_size;
        zero_pad = (uint8_t *) calloc(PADDINGSIZE, sizeof(uint8_t));
    }

    for (int32_t i = image->info.image_height - 1; i >= 0; i--) {
        fwrite(image->data[i],
               sizeof(sbmp_raw_data),
               (uint32_t) image->info.image_width,
               fd);
        if (NULL != zero_pad)
            fwrite(zero_pad, sizeof(uint8_t), padd_size, fd);
    }

    if (!zero_pad)
        free(zero_pad);

    return SBMP_OK;
}

void kernel_setup(uint16_t **kern, int16_t ksize) {
    uint16_t st_val = 1;

    for (int j = 0; j < ksize; j++)
        kern[0][j] = st_val;

    for (int i = 1; i < ksize / 2 + 1; i++) {
        for (int j = 0; j < ksize; j++) {
            if (j >= i && j < (ksize - i))
                kern[i][j] = (uint16_t) (kern[i - 1][j] + (uint16_t) 1);
            else
                kern[i][j] = kern[i - 1][j];
        }

    }
    for (int i = 1; i < ksize / 2; i++) {
        for (int j = 0; j < ksize; j++) {
            kern[i + ksize / 2][j] = kern[ksize / 2 - i][j];
        }

    }

    for (int i = 0; i < ksize; i++) {
        for (int j = 0; j < ksize; j++) {
            printf("%3hu ", kern[i][j]);
            sumatoria = (uint16_t) (kern[i][j] + sumatoria);
        }
        printf("\n");
    }
}

uint64_t rdtsc() {
    printf("HOLAAA");
    unsigned int lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t) hi << 32) | lo;

}

void recvDatosUsuario() {
    char buffer[TAM];
    char *token;
    const char ident[] = ",";
    int num = 0;
    printf("Bienvenido!!\n");
    printf("Ingrese la altura,ancho y radio para la imagen de salida:\n");
    printf("ejemplo: 500,1000,600>");
    if (fgets(buffer, TAM - 1, stdin) == 0) {
        printf("Interrupción\n");
        exit(-1);
    }
    token = strtok(buffer, ident);
    while (token != NULL) {
        switch (num) {
            case 0:
                alto = (uint32_t) atoi(token);
                break;
            case 1:
                ancho = (uint32_t) atoi(token);
                break;
            case 2:
                radio = (uint32_t) atoi(token);
                break;
            default:
                break;
        }
        token = strtok(NULL, ident);
        num++;
    }
    num = 0;
    memset(buffer, '\0', TAM);
    printf("\nIngrese el contraste, brillo y tamaño de la matriz kernel deseada\n");
    printf("ejemplo: 1,2,42>");
    if (fgets(buffer, TAM - 1, stdin) == 0) {
        printf("Interrupción\n");
        exit(-1);
    }
    token = strtok(buffer, ident);
    while (token != NULL) {
        switch (num) {
            case 0:
                kk = (int) atoi(token);
                break;
            case 1:
                l = (int) atoi(token);
                break;
            case 2:
                SIZE_K = (int16_t) atoi(token);
                break;
            default:
                break;
        }
        token = strtok(NULL, ident);
        num++;
    }
}
