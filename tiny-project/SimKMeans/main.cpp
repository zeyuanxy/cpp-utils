#include <string>
#include <time.h>
#include <Windows.h>
#include <vector>
#include "Platform.h"
#include <mpi.h>

using namespace std;

Platform *platform;

struct AppArgs
{	    
	int dim;
	int centroidNum;
	int iterationNum;
	int threadNum;
	string centroidsPath;
	string resultPath;
};


void ParseArgs(int argc, char **argv, AppArgs *pAppArgs)
{
	// Parse input args.
	for(int i = 0; i < argc; ++i)
	{	
		if(strcmp(argv[i], "-dim") == 0)
		{
			pAppArgs->dim = atoi(argv[i+1]);
			++i;
		}
		else if(strcmp(argv[i], "-K") == 0)
		{
			pAppArgs->centroidNum = atoi(argv[i+1]);
			++i;
		}
		else if (strcmp(argv[i], "-iteration") == 0)
		{
			pAppArgs->iterationNum = atoi(argv[i+1]);
			++i;
		}
		else if(strcmp(argv[i], "-Z") == 0)
		{
			pAppArgs->threadNum = atoi(argv[i+1]);
			++i;
		}
		else if(strcmp(argv[i], "-centroids") == 0)
		{
			pAppArgs->centroidsPath = argv[i+1];
			++i;
		}
		else if(strcmp(argv[i], "-output") == 0)
		{
			pAppArgs->resultPath = argv[i+1];
			++i;
		}
	}
}


void LoadCentroids(AppArgs *pAppArgs, double *centroids)
{
	FILE *pFile;
    errno_t err = fopen_s(&pFile, pAppArgs->centroidsPath.c_str(), "rb");
    if (err != 0)
    {
		abort();
    }
    int centroidNum;
    int dim;
    int ret = fscanf_s(pFile, "%d", &centroidNum);
    if (ret != 1)
    {
        abort();
    }
    ret = fscanf_s(pFile, "%d", &dim);
    if (ret != 1)
    {
        abort();
    }    
    for (int i = 0; i < centroidNum; ++i)
    {
        for (int j = 0; j < dim; ++j)
        {
            ret = fscanf_s(pFile, "%lf", (centroids + i * dim + j));
            if (ret != 1)
            {
                if (ret != dim)
                {
                    abort();
                }
            }
        }
    }
    fclose(pFile);
}


void UpdateCentroids(AppArgs *pAppArgs, double *pWeightVecs, int *pCounts, double *centroids)
{
	for (int i = 0; i < pAppArgs->centroidNum; ++i)
	{
		double *curCentroid = centroids + i * pAppArgs->dim;
		double *curWeightVec = pWeightVecs + i * pAppArgs->dim;
		int curCount = pCounts[i];
		for (int j = 0; j < pAppArgs->dim; ++j)
		{
			curCentroid[j] = curWeightVec[j] / curCount;
		}
	}
}


double GetDistance(int dim, float *p1, double *p2)
{
	double dis = 0.0;
	for (int i = 0; i < dim; ++i)
	{
		double f1 = (double)p1[i];
		double f2 = p2[i];
		dis += (f1 - f2) * (f1 - f2);
	}
	return sqrt(dis);
}


/* HTK File Header */
struct HTKhdr
{              
	int nSamples;
	int sampPeriod;
	short sampSize;
	short sampKind;
};


/* SwapInt32: swap byte order of int32 data value *p */
void Swap32(int *p)
{
   char temp,*q;
   
   q = (char*) p;
   temp = *q; *q = *(q+3); *(q+3) = temp;
   temp = *(q+1); *(q+1) = *(q+2); *(q+2) = temp;
}

/* SwapShort: swap byte order of short data value *p */
void Swap16(short *p)
{
   char temp,*q;
   
   q = (char*) p;
   temp = *q; *q = *(q+1); *(q+1) = temp;
}


struct WorkerThreadArg
{
	AppArgs *pAppArgs;
	double *pWeightAcc;
	int *pCountAcc;
	double *pCentroids;
	int threadIndex;
};


DWORD WINAPI WorkerThread(PVOID para)
{
	WorkerThreadArg *pArg = (WorkerThreadArg*)para;
	float *featureVec = new float[pArg->pAppArgs->dim];
	while (true)
	{
		FILE *fp = platform->getNextFile();
		if (!fp)
			break;
        HTKhdr header;
		fread_s(&header, sizeof(header), sizeof(header), 1, fp);
		Swap32(&header.nSamples);
		Swap32(&header.sampPeriod);
		Swap16(&header.sampSize);
		Swap16(&header.sampKind);
	
		for (int iSample = 0; iSample < header.nSamples; ++iSample)
		{
			// 1. Read a sample's feature vector.
			fread_s(featureVec, pArg->pAppArgs->dim * sizeof(float), sizeof(float), pArg->pAppArgs->dim, fp);
            int *tmp = (int *)(&featureVec[0]);
            for (int iDim = 0; iDim < pArg->pAppArgs->dim; ++iDim)
            {
                Swap32(tmp + iDim);
            }
			
			// 2. Calculate distance between all centroids and choose nearest one.
			double minDis = 0.0;
			int bestId = 0;
			for (int i = 0; i < pArg->pAppArgs->centroidNum; ++i)
			{
				double *centroid = pArg->pCentroids + i * pArg->pAppArgs->dim;
				double dis = GetDistance(pArg->pAppArgs->dim, featureVec, centroid);
				if (i == 0 || dis < minDis)
				{
					minDis = dis;
					bestId = i;
				}
			}

			// 3. Accumulate.
			for (int i = 0; i < pArg->pAppArgs->dim; ++i)
			{
				pArg->pWeightAcc[bestId * pArg->pAppArgs->dim + i] += featureVec[i];
			}
			pArg->pCountAcc[bestId] += 1;
		}

		fclose(fp);
	}
	delete []featureVec;
	return 0;
}


void OutputResult(AppArgs *pAppArgs, double *centroids)
{
	FILE *pFile;
	errno_t err = fopen_s(&pFile, pAppArgs->resultPath.c_str(), "w");
	if (err != 0)
	{
		abort();
	}
	int centroidNum = pAppArgs->centroidNum;
	int dim = pAppArgs->dim;
	fprintf(pFile, "%d\n", centroidNum);
	fprintf(pFile, "%d\n", dim);	
	for (int i = 0; i < centroidNum; ++i)
	{
        for (int j = 0; j < dim; ++j)
        {
            if (j > 0)
            {
                fprintf(pFile, " ");
            }
            fprintf(pFile, "%lf", *(centroids + i * dim + j));
        }
        fprintf(pFile, "\n");
	}
	fclose(pFile);
}

template <class T>
void reduce(T *acc, int size) {
	T* buffer = NULL;
	if (platform->getRank() == 0)
	{
		buffer = new T[platform->getNProc() * size];
	}
	platform->gather(acc, buffer, sizeof(T) * size);
	if (platform->getRank() == 0)
	{
		int nproc = platform->getNProc();
		for (int i = 1; i < nproc; ++i)
		{
			T *rAcc = buffer + i * size;
			for (int j = 0; j < size; ++j)
				acc[j] += rAcc[j];
		}
	}
}

int main(int argc, char **argv)
{
	AppArgs appArgs;
	ParseArgs(argc, argv, &appArgs);

	double *centroids = new double[appArgs.centroidNum * appArgs.dim];
	int centroidsSize = appArgs.centroidNum * appArgs.dim * sizeof(double);

	platform = new Platform(argc, argv);
	printf("hello from rank = %d\n", platform->getRank());
	if (platform->getRank() == 0) 
	{
		printf("loading...\n");
		LoadCentroids(&appArgs, centroids);
	}

	if (platform->getRank() == 0)
		printf("start iterations ...\n");

	for (int iterationId = 0; iterationId < appArgs.iterationNum; ++iterationId)
	{
		// New iteration
		if (platform->getRank() == 0)
			printf("iteration #%d\n", iterationId);
		platform->newIteration();

		// Broadcast centroids to all processes.
		if (platform->getRank() == 0)
			printf("broadcasting...\n");
		platform->broadcast(centroids, centroidsSize);

		// Create worker threads and run them.        
		double *weightAcc = new double[appArgs.centroidNum * appArgs.dim];
		memset(weightAcc, 0, sizeof(double) * appArgs.centroidNum * appArgs.dim);
		int *countAcc = new int[appArgs.centroidNum];
		memset(countAcc, 0, sizeof(int)* appArgs.centroidNum);

		if (platform->getRank() == 0)
			printf("running...\n");
		HANDLE *hThreads = new HANDLE[appArgs.threadNum];
		for (int i = 0; i < appArgs.threadNum; ++i)
		{
			WorkerThreadArg arg;
			arg.pWeightAcc = weightAcc;
			arg.pCountAcc = countAcc;
			arg.pAppArgs = &appArgs;
			arg.pCentroids = centroids;
			arg.threadIndex = i;

			hThreads[i] = ::CreateThread(NULL, 0, WorkerThread, &arg, 0, NULL);
		}
		::WaitForMultipleObjects(appArgs.threadNum, hThreads, TRUE, INFINITE);
		delete []hThreads;

		// Aggregate partial accumulator among all processes.
		if (platform->getRank() == 0)
			printf("reducing...\n");
		reduce(weightAcc, appArgs.centroidNum * appArgs.dim);
		reduce(countAcc, appArgs.centroidNum);

		// Update centroids.
		if (platform->getRank() == 0)
		{
			printf("updating...\n");
			UpdateCentroids(&appArgs, weightAcc, countAcc, centroids);
		}

		delete [] weightAcc;
		delete [] countAcc;

		platform->endIteration();
	}

	if (platform->getRank() == 0) {
		printf("the program has completed successfully\n");
        OutputResult(&appArgs, centroids);    
	}

	delete []centroids;
	platform->finish();
	return 0;
}