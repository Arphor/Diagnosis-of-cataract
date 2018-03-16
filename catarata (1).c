
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "biblioteca.h"

int gauss(pixel **matriz, pixel **matriz_aux, int a, int l);

int sobel_bin(pixel **matriz, pixel **matriz_aux, int a, int l);

int maior(int ***matriz, int x, int y, int b, int alt, int larg);

int hough(pixel **matriz, pixel **aux, pixel **colorida, pixel **cinza, pixel **final, int alt, int larg);

int teste(pixel **matriz, int alt, int larg);

int main(int argc,char*argv[]){

	FILE *inicial;
	FILE *final;
	char* nome_inicial;
	char* nome_final;
	char* formato;
	char codigo[3];
	char linha[50];
	int i, j, a, l, max;
	pixel **matriz;
	pixel **matriz_cinza;
	pixel **matriz_gauss;
	pixel **matriz_bin;
	pixel **matriz_hough;
	pixel **matriz_final;

	nome_inicial = argv[2];
	nome_final = argv[4];
	formato = argv[6];


	//Garantimos que o arquivo foi aberto com sucesso.

	inicial = fopen(nome_inicial , "r");
	if(inicial == NULL){
		printf("Erro ao abrir arquivo \n");
		return 0;
	}

	//Conferimos se ele é PPM

	fscanf(inicial, "%s ", codigo);
	if(strcmp (codigo,"P3") != 0){
		printf("Arquivo nao e PPM \n");
		fclose(inicial);
		return 0;
	}

	fgets(linha, 50 ,inicial);

	//Agora lemos o arquivo e o colocamos em uma matriz chamada "matriz".

	fscanf(inicial, "%d", &l);
	fscanf(inicial, "%d", &a);
	fscanf(inicial, "%d", &max);

	matriz = malloc (a * sizeof (pixel *));
	for (i = 0; i < a; i++){
		matriz[i]= malloc (l * sizeof (pixel));
	}


	for(i=0; i<a; i++){
		for(j=0; j<l; j++){
			fscanf(inicial, "%d", &matriz[i][j].r);
			fscanf(inicial, "%d", &matriz[i][j].g);
			fscanf(inicial, "%d", &matriz[i][j].b);
		}
	}

	//Aqui aplicamos o filtro cinza em cada pixel da imagem.

	matriz_cinza = malloc (a * sizeof (pixel *));
	for (i = 0; i < a; i++){
		matriz_cinza[i]= malloc (l * sizeof (pixel));
	}

	for(i=0; i<a; i++){
		for(j=0; j<l; j++){
			matriz_cinza[i][j].r = ((0.3 * matriz[i][j].r) + (0.59 * matriz[i][j].g) + (0.11 * matriz[i][j].b));
			matriz_cinza[i][j].g = matriz_cinza[i][j].r;
			matriz_cinza[i][j].b = matriz_cinza[i][j].r;
		}
	}

	//Agora criamos uma nova matriz para podermos aplicar gauss nela.

	matriz_gauss = malloc (a * sizeof (pixel *));
	for (i = 0; i < a; i++){
		matriz_gauss[i]= malloc (l * sizeof (pixel));
	}

	//Aplicamos o filtro de gauss.


	gauss(matriz_cinza, matriz_gauss, a, l);

	//Criamos uma nova matriz para aplicarmos sobel e a binarização.

	matriz_bin = malloc (a * sizeof (pixel *));
	for (i = 0; i < a; i++){
		matriz_bin[i]= malloc (l * sizeof (pixel));
	}

	//Aplicamos os dois filtros.

	sobel_bin(matriz_gauss, matriz_bin, a, l);

	//Criamos 2 matrizes, com calloc para que elas fiquem zeradas, para identificarmos a pupila na imagem colorida e na com tons de cinza.

	matriz_hough = calloc (a, sizeof (pixel *));
		for (i = 0; i < a; i++){
			matriz_hough[i]= calloc (l, sizeof (pixel));
		}

	matriz_final = calloc (a, sizeof (pixel *));
		for (i = 0; i < a; i++){
			matriz_final[i]= calloc (l, sizeof (pixel));
		}

	//Fazemos a identificação atraves de hough.

	hough(matriz_bin, matriz, matriz_hough, matriz_cinza, matriz_final, a, l);

	//Testamos se o paciente possui catarata ou nao.

	teste(matriz_final, a, l, nome_final);



	final = fopen(nome_final , "w");


	fprintf(final, "P3 \n");
	fprintf(final, "%d \n", l);
	fprintf(final, "%d \n", a);
	fprintf(final, "%d \n", max);

	for(i=0; i<a; i++){
		for(j=0; j<l; j++){
			fprintf(final, "%d \n", matriz_hough[i][j].r);
			fprintf(final, "%d \n", matriz_hough[i][j].g);
			fprintf(final, "%d \n", matriz_hough[i][j].b);
		}
	}

	texto = fopen(nome_final , "w");




	fclose(inicial);
	fclose(final);

	return 0;
}

int gauss(pixel **matriz, pixel **matriz_aux, int a, int l){
	int A, B;
	int x, y;

	//Criamos uma matriz 5 por 5 com os valores dados no PDF.

	int F[5][5]={{2, 4, 5, 4, 2}, {4, 9, 12, 9, 4}, {5, 12, 15, 12, 5}, {4, 9, 12, 9, 4}, {2, 4, 5, 4, 2}};

	//Aplicamos a formula de gauss.

	for(x=0; x<a ; x++){
		for(y=0; y<l; y++){
			int aux=0;
			for(A=-2; A<=2; A++){
				for(B=-2; B<=2; B++){
					if(x+A<a && y+B<l && x+A>0 && y+B>0){
						aux += (F[A+2][B+2]*matriz[x+A][y+B].r);
					}
				}
			}
			matriz_aux[x][y].r = aux/159;
			matriz_aux[x][y].g = aux/159;
			matriz_aux[x][y].b = aux/159;
		}
	}

	return 0;
}

int sobel_bin(pixel **matriz, pixel **matriz_aux, int a, int l){
	int FX[3][3]={{-1, 0, +1}, {-2, 0, +2}, {-1, 0, +1}};
	int FY[3][3]={{+1, +2, +1}, {0, 0, 0}, {-1, -2, -1}};
	int aux;
	int x, y, i, j;
	int gx, gy;

	//Calculamos sobel com base nas duas matrizes 3 por 3.

	for(x=0; x<a; x++){
		for(y=0; y<l; y++){
			aux=0;
			gx=0;
			gy=0;
			for(i= -1; i<=1; i++){
				for(j= -1; j<=1; j++){
					if(x+i<a && y+j<l && x+i>0 && y+j>0){
						gx += FX[i+1][j+1]*matriz[x+i][y+j].r;
						gy += FY[i+1][j+1]*matriz[x+i][y+j].r;
					}
				}
			}

			//Usamos os valores de sobel para aplicarmos a binarização.

			aux=sqrt((gx*gx)+(gy*gy));
			if(aux>36){
				matriz_aux[x][y].r=250;
				matriz_aux[x][y].g=250;
				matriz_aux[x][y].b=250;
			}
			else{
				matriz_aux[x][y].r=0;
				matriz_aux[x][y].g=0;
				matriz_aux[x][y].b=0;
			}
		}
	}

	return 0;
}

int hough(pixel **matriz, pixel **aux, pixel **colorida, pixel **cinza, pixel **final, int alt, int larg){


	int x, y, r, t, a, b;
	int rmin, rmax;
	int ***Hough;
	int i, j;

	//Criamos o raio minimo e maximo com base no menor lado da imagem.


	if(larg<alt){
		rmin=larg/8;
		rmax=larg/3.5;
	}
	else{
		rmin=alt/8;
		rmax=alt/3.5;
	}

	//Criamos uma matriz auxiliar tridimensional.

	Hough = malloc (alt * sizeof (pixel *));
	for (i = 0; i < alt; i++){
		Hough[i]= malloc (larg * sizeof (pixel));
		for(j=0; j<larg; j++){
			Hough[i][j]= malloc (rmax * sizeof (pixel));
		}
	}

	//Aplicamos Hough.

	for(x=alt/4; x<alt - alt/4; x++){
		for(y=larg/4; y<larg-larg/4; y++){
			//Começamos desses valores para diminuir o tamanho da imagem e agilizar o processamento.
			if(matriz[x][y].r == 250){
				for(r=rmin; r<rmax; r++){
					for(t=0; t<360; t++){
						a = x-r*cos((t*3.14)/180);
						b = y-r*sin((t*3.14)/180);
						if(a>0 && a<x && b>0 && b<y){
							Hough[a][b][r] += 1;
						}

					}
				}
			}
		}
	}

	//Agora descobrimos qual o ponto que possui maior valor, quais suas coordenadas e qual o raio.

	int maior, maior_x, maior_y, raio;
	maior=Hough[0][0][0];
	maior_x=0;
	maior_y=0;
	raio=0;

	maior=Hough[0][0][0];

	for(r=rmin; r<rmax; r++){
		for(x=0; x<alt; x++){
			for(y=0; y<larg; y++){
				if(maior<Hough[x][y][r]){
					maior=Hough[x][y][r];
					maior_x=x;
					maior_y=y;
					raio=r;
				}
			}
		}
	}

	printf("x=%d, y=%d, r=%d \n", maior_x, maior_y, raio);

	//Agora criamos 2 matrizes apenas com a pupila, uma colorida para saida e uma cinza para a analise de catarata.

	for(x=0; x<alt; x++){
		for(y=0; y<larg; y++){
			if(sqrt((maior_x-x)*(maior_x-x)+(maior_y-y)*(maior_y-y))<raio){
				colorida[x][y].r = aux[x][y].r;
				colorida[x][y].g = aux[x][y].g;
				colorida[x][y].b = aux[x][y].b;

				final[x][y].r = cinza[x][y].r;
				final[x][y].g = cinza[x][y].g;
				final[x][y].b = cinza[x][y].b;
			}
		}
	}



	return 0;
}

int teste(pixel **matriz, int alt, int larg, char nome){

	FILE *texto;
	int x, y;
	float soma=0, pixels, media;

	printf("Dentro \n");

	//Fazemos a media dos pixels da imagem.

	for(x=0; x<alt; x++){
		for(y=0; y<larg; y++){
			if(matriz[x][y].r>0){
				soma= soma + matriz[x][y].r;
				pixels++;
			}
		}
	}

	media = (soma/pixels)/100.0;

	printf("%f \n", media);

	//Atraves dessa media descobrimos se a imagem é de um olho com catarata ou não.

	texto = fopen(nome , "w");

	if(media>1.5){
		fprintf(final, "Erro na analise\n", );
	}
	else{
		if(media>0.8){
			fprintf(final, "O paciente possui catarata de %f % \n", media);
		}
		else{
			fprintf(final, "O paciente não possui catarata \n");
		}
	}



	return 0;
}
