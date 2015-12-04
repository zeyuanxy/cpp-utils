#pragma once
#include "include.h"
#include <iostream>
#include <vector>

using namespace std;

class Platform {
private:
	int m_rank, m_nproc, m_current;
	vector<string> m_file;
	void getFiles(int argc, char **argv);
	void uploadFile(string filename1, string filename2);
public:
	Platform(int argc, char **argv);
	~Platform();
	int getRank();
	int getNProc();
	void newIteration();
	void endIteration();
	void broadcast(void *addr, int size);
	void scatter(void *addr1, void *addr2, int size);
	void gather(void *addr1, void *addr2, int size);
	void reduce(void *addr1, void *addr2, int type, int op, int size);
	FILE *getNextFile();
	void finish();
};