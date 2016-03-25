#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <complex>
#include <string>
#include <cstring>
#include <time.h> 

using std::ios;
using std::cin;
using std::cout;
using std::string;
using std::complex;
using std::ifstream;
using std::ofstream;
using std::endl;

int main(int argc, char** argv)
{
	srand (time(NULL));
	FILE * pFile;
	int w, h;      // width, height, Max for module

	pFile = fopen ( "out_new_matrix.bin" , "wb" );
	if (pFile==NULL) {fputs ("File error",stderr); exit (1);}

	if( argc == 3 )
	{
		h = atoi( argv[1] );
		w = atoi( argv[2] );
	}
	else
	{
		char BadSign[] = "CAN NOT READ HEIGHT AND WIDE OF MATRIX";
		fwrite(BadSign, sizeof(char), sizeof(BadSign), pFile );
		return 0;
	}	

	char Matrix[] = "MATRIX";

	fwrite(Matrix, sizeof(char), 6, pFile);
	fwrite(&h, sizeof(int), 1, pFile);
	fwrite(&w, sizeof(int), 1, pFile);

//	cout << "MATRIX " << h << " " << w << endl;

	for(int i=0; i<h; i++)
	{
		for(int j=0; j<w; j++)
		{
			double realTemp, imagTemp;
			realTemp = rand()%10+(double)rand()/(RAND_MAX);
//			realTemp = 1;
			imagTemp = rand()%10+(double)rand()/(RAND_MAX);
//			imagTemp = 1;
			fwrite(&realTemp, sizeof(double), 1, pFile);
			fwrite(&imagTemp, sizeof(double), 1, pFile);
//			cout << realTemp << " i" << imagTemp << " | ";

		}	
//			cout << endl;
	}
	
	fclose(pFile);	
/*
	char nameMatrix[] = "      ";
	pFile = fopen ( "out_new_matrix.bin" , "rb" );
	if (pFile==NULL) {fputs ("File error",stderr); exit (1);}

	size_t lSize = 6;
	size_t result;
	result = fread (nameMatrix, sizeof(char), 6, pFile);
	if (result != lSize) {fputs ("Reading error",stderr); exit (3);}

	fread (&h, sizeof(int), 1, pFile);
	fread (&w, sizeof(int), 1, pFile);

	cout << nameMatrix << " " << h << " " << w << endl;

	for(int i=0; i<h; i++)
	{
		for(int j=0; j<w; j++)
		{
			double realTemp, imagTemp;

			fread(&realTemp, sizeof(double), 1, pFile);
			fread(&imagTemp, sizeof(double), 1, pFile);
			cout << realTemp << " i" << imagTemp << " ";

		}	
			cout << endl;
	}

	fclose(pFile);
*/
	return 0;
}
