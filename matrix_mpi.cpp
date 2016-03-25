#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <cfloat>

#include <cstdlib>
#include <fstream>
#include <string>
#include <complex>
#include <vector>
#include <cstring>

#include <climits>
#include <sys/time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

using std::vector;
using std::string;
using std::complex;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::pair;
using std::ios;
using std::cout;

string nameMatrix;
int h, w;                                     // height, width
vector< vector < complex< double > > > Matrix;

 
int main (int argc, char* argv[])
{
	struct timeval start, finish;
	ofstream fout;
	fout.open("mpi_output.txt");	

	FILE * pFile;
	size_t lSize;
	size_t result;
	int zeroStep = 6+2*sizeof(int);
	char nameMatrix[] = "ololol";

	pFile = fopen ( "out_new_matrix.bin" , "rb" );
	if (pFile==NULL) {fputs ("File error",stderr); exit (1);}
	
	lSize = 6;
	result = fread (nameMatrix, 1, lSize, pFile);
	if (result != lSize) {fputs ("Reading error",stderr); exit (3);}

	result = fread (&h, sizeof(int), 1, pFile);
	result = fread (&w, sizeof(int), 1, pFile);

	if(strcmp(nameMatrix, "MATRIX") || !(h && w) )
	{
		fout << "bad file" << endl;
		return 0 ;
	}
	gettimeofday(&start, NULL);
//============================================clock_start

	int size, rank;
	MPI_Init (&argc, &argv);
	
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	MPI_Comm_size( MPI_COMM_WORLD, &size );

	double moduleMaxAbs, realMaxAbs, imagMaxAbs;
	vector < int > idMaxAbs;
	double moduleMinAbs, realMinAbs, imagMinAbs;
	vector < int > idMinAbs;

	double moduleMaxZ, realMaxZ, imagMaxZ;
	vector < int > idMaxZ;
	double moduleMinZ, realMinZ, imagMinZ;
	vector < int > idMinZ;

	if( rank == 0 )
	{	
// 0 proc divides tasks between other (size-1) proc and rest take itself
		int taskPerProc = 0;
		int rest = h*w; // (x2) because real and imag
		if( size != 1 )
		{
			taskPerProc = h*w/(size-1);
			rest = h*w % (size-1);
		}

		for( int i=1; i<size; i++)
// send tasks other process
		{
			MPI_Send( &taskPerProc, 1, MPI_INT, i, i, MPI_COMM_WORLD );
		}

// lets do it now
		
//		int endFile = zeroStep+2*h*w*sizeof(double);
// init of max/min
		fseek(pFile, zeroStep+2*(h*w-rest)*sizeof(double), SEEK_SET);

 		double tmpReal, tmpImag, module;
		result = fread( &tmpReal, sizeof(double), 1, pFile);
		result = fread( &tmpImag, sizeof(double), 1, pFile);

		realMaxZ = tmpReal;
		imagMaxZ = tmpImag;

		realMinZ = tmpReal;
		imagMinZ = tmpImag;
		
		module = tmpReal*tmpReal + tmpImag*tmpImag;
		moduleMaxZ = module;
		moduleMinZ = module;
// find min/max in rest of matrix

		fseek(pFile, zeroStep+2*(h*w-rest)*sizeof(double), SEEK_SET);
		for(int i = h*w-rest; i < h*w; i++ )//                      h*w-rest OR h*w-rest+1
		{
			result = fread( &tmpReal, sizeof(double), 1, pFile);
			result = fread( &tmpImag, sizeof(double), 1, pFile);

			module = tmpReal*tmpReal + tmpImag*tmpImag;

			if(module == moduleMaxZ)
			{
				idMaxZ.push_back( i/w );
				idMaxZ.push_back( i%w );
			}
			if(module > moduleMaxZ)
			{
				realMaxZ = tmpReal;
				imagMaxZ = tmpImag;
				moduleMaxZ = module;
				idMaxZ.clear();
				idMaxZ.push_back( i/w );
				idMaxZ.push_back( i%w );
			}
			if(module == moduleMinZ)
			{
				idMinZ.push_back( i/w );
				idMinZ.push_back( i%w );
			}
			if(module < moduleMinZ)
			{
				realMinZ = tmpReal;
				imagMinZ = tmpImag;
				moduleMinZ = module;
				idMinZ.clear();
				idMinZ.push_back( i/w );
				idMinZ.push_back( i%w );
			}		
		}

//inic abs
		fseek(pFile, zeroStep, SEEK_SET);

 		tmpReal, tmpImag, module;
		result = fread( &tmpReal, sizeof(double), 1, pFile);
		result = fread( &tmpImag, sizeof(double), 1, pFile);

		realMaxAbs = tmpReal;
		imagMaxAbs = tmpImag;

		realMinAbs = tmpReal;
		imagMinAbs = tmpImag;
		
		module = tmpReal*tmpReal + tmpImag*tmpImag;
		moduleMaxAbs = module;
		moduleMinAbs = module;		

//abs vs Z
		if( rest )
		{
			if( moduleMaxAbs == moduleMaxZ )
			{
				for(int i=0; i<idMaxZ.size(); i++)
					idMaxAbs.push_back( idMaxZ[i] );
			}
			else if( moduleMaxAbs < moduleMaxZ )
			{
				moduleMaxAbs = moduleMaxZ;
				realMaxAbs = realMaxZ;
				imagMaxAbs = imagMaxZ;
				idMaxAbs.clear();
				idMaxAbs = idMaxZ;
			}
			if( moduleMinAbs == moduleMinZ )
			{
				for(int i=0; i<idMinZ.size(); i++)
					idMinAbs.push_back( idMinZ[i] );
			}
			else if( moduleMinAbs > moduleMinZ )
			{
				moduleMinAbs = moduleMinZ;
				realMinAbs = realMinZ;
				imagMinAbs = imagMinZ;
				idMinAbs.clear();
				idMinAbs = idMinZ;
			}
		}	
		
//abs vs rest
		for( int i=1; i<size; i++ )
		{
			double moduleMax, realMax, imagMax;
			int sizeMax;
			double moduleMin, realMin, imagMin;
			int sizeMin;
				
			int num_amount;
			MPI_Status status;

			MPI_Probe(i, i, MPI_COMM_WORLD, &status);
			MPI_Get_count(&status, MPI_DOUBLE, &num_amount);
			
			double arr[num_amount];

			MPI_Recv(arr, num_amount, MPI_DOUBLE, i, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		        moduleMax = arr[0];
			realMax = arr[1];
			imagMax = arr[2];
			sizeMax = (int)arr[3];
			int idMaxArray[sizeMax];		

			for(int k=4; k<sizeMax+4; k++)
			{
				idMaxArray[k-4] = (int)arr[k];
			}

		        moduleMin = arr[4+sizeMax];
			realMin = arr[5+sizeMax];
			imagMin = arr[6+sizeMax];
			sizeMin = (int)arr[7+sizeMax];
			int idMinArray[sizeMin];

			for(int k=8+sizeMax; k<num_amount; k++)
			{
				idMinArray[k-8-sizeMax] = (int)arr[k];
			}
			

			if( moduleMaxAbs == moduleMax )
			{
				for(int i=0; i<sizeMax; i++)
					idMaxAbs.push_back( idMaxArray[i] );
			}
			else if( moduleMaxAbs < moduleMax )
			{
				moduleMaxAbs = moduleMax;
				realMaxAbs = realMax;
				imagMaxAbs = imagMax;
				idMaxAbs.clear();
				for(int i=0; i<sizeMax; i++)
					idMaxAbs.push_back( idMaxArray[i] );
			}
			if( moduleMinAbs == moduleMin )
			{
				for(int i=0; i<sizeMin; i++)
					idMinAbs.push_back( idMinArray[i] );
			}
			else if( moduleMinAbs > moduleMin )
			{
				moduleMinAbs = moduleMin;
				realMinAbs = realMin;
				imagMinAbs = imagMin;
				idMinAbs.clear();
				for(int i=0; i<sizeMin; i++)
					idMinAbs.push_back( idMinArray[i] );
			}	
		}

		fout << "Max item: " << realMaxAbs << " + i " <<
						imagMaxAbs << endl;
		fout << "ID of Max ( height, width ): " << endl;
		for(int i=0; i<idMaxAbs.size(); i+=2)
		{
			fout << idMaxAbs[i] << "   " << 
				idMaxAbs[i+1] << endl;
		}

		fout << "Min item: " << realMinAbs << " + i " <<
						imagMinAbs << endl;
		fout << "ID of Min ( height, width ): " << endl;
		for(int i=0; i<idMinAbs.size(); i+=2)
		{
			fout << idMinAbs[i] << "   " << 
				idMinAbs[i+1] << endl;
		}	
//=============================================clock_stop
		gettimeofday(&finish, NULL);

		fout << "Time of work (sec): " << 
			(double)(finish.tv_sec-start.tv_sec)*1000000+(double)(finish.tv_usec-start.tv_usec) << endl;
		
	}


	if( rank != 0 )
	{
		int taskPerProc;
		MPI_Recv( &taskPerProc, 1, MPI_INT, 0, rank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
					
		for(int j=1; j<size; j++)
		if( rank == j )
		{
			double moduleMax, realMax, imagMax;
			vector< int > idMax;   

			double moduleMin, realMin, imagMin;
			vector< int > idMin; 

			fseek(pFile, zeroStep+2*(j-1)*taskPerProc*sizeof(double), SEEK_SET);//   !!!!!!!

	 		double tmpReal, tmpImag, module;
			result = fread( &tmpReal, sizeof(double), 1, pFile);
			result = fread( &tmpImag, sizeof(double), 1, pFile);

			realMax = tmpReal;
			imagMax = tmpImag;

			realMin = tmpReal;
			imagMin = tmpImag;
		
			module = tmpReal*tmpReal + tmpImag*tmpImag;
			moduleMax = module;
			moduleMin = module;
	
			fseek(pFile, zeroStep+2*(j-1)*taskPerProc*sizeof(double), SEEK_SET);
			for(int i = (j-1)*taskPerProc; i < j*taskPerProc; ++i)
			{
				result = fread( &tmpReal, sizeof(double), 1, pFile);
				result = fread( &tmpImag, sizeof(double), 1, pFile);

				module = tmpReal*tmpReal + tmpImag*tmpImag;

				if(module == moduleMax)
				{
					idMax.push_back( i/w );
					idMax.push_back( i%w );
				}
				if(module > moduleMax)
				{
					realMax = tmpReal;
					imagMax = tmpImag;
					moduleMax = module;
					idMax.clear();
					idMax.push_back( i/w );
					idMax.push_back( i%w );
				}
				if(module == moduleMin)
				{
					idMin.push_back( i/w );
					idMin.push_back( i%w );
				}
				if(module < moduleMin)
				{
					realMin = tmpReal;
					imagMin = tmpImag;
					moduleMin = module;
					idMin.clear();
					idMin.push_back( i/w );
					idMin.push_back( i%w );
				}
			}
			int idMaxSize = idMax.size();
			int idMinSize = idMin.size(); 
			double Array[8+idMax.size()+idMin.size()];

		        Array[0] = moduleMax;
			Array[1] = realMax;
			Array[2] = imagMax;
			Array[3] = (double)idMaxSize;		

			for(size_t k=4; k<idMax.size()+4; k++)
			{
				Array[k] = (double)idMax[k-4];
			}

		        Array[0+idMax.size()+4] = moduleMin;
			Array[1+idMax.size()+4] = realMin;
			Array[2+idMax.size()+4] = imagMin;
			Array[3+idMax.size()+4] = (double)idMinSize;

			for(size_t k=8+idMax.size(); k<idMin.size()+idMax.size()+8; k++)
			{
				Array[k] = idMin[k-8-idMax.size()];
			}	
			MPI_Send(Array, (int)(idMin.size()+idMax.size()+8), MPI_DOUBLE, 0, j, MPI_COMM_WORLD);
/*/========================================================
cout << "MIN " << realMin << " " << imagMin << endl;
//========================================================*/

		}		
	}
	MPI_Finalize();
		
	fout.close();
	fclose ( pFile );
	return 0;
}
