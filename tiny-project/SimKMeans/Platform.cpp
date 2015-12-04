#include "Platform.h"
#include "hdfs.h"
#include <mpi.h>
#include <fstream>
#include <string>

Platform::Platform(int argc, char** argv) {
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &m_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &m_nproc);
	getFiles(argc, argv);
	MPI_Barrier(MPI_COMM_WORLD);
};

Platform::~Platform() {
	MPI_Finalize();
}

void Platform::getFiles(int argc, char **argv) {
	char *filename = NULL;
	for (int i = 0; i < argc; ++i)
	{
		if (strcmp(argv[i], "-S") == 0)
		{
			filename = argv[i + 1];
			break;
		}
	}
	ifstream fin(filename);
	string t;
	while (getline(fin, t)) {
		size_t pos = t.rfind('\\');
		string sub = "";
		if (pos == string::npos) 
			sub = t;
		else
			sub = t.substr(pos + 1);
		/*if (m_rank == 0)
			uploadFile(t, sub);*/
		m_file.push_back(sub);
	}
	m_current = 0;
}

void Platform::uploadFile(string filename1, string filename2) {
	hdfsFS fs = hdfsConnect(HDFS_ADDRESS, HDFS_PORT);
	string writePath = "/data/" + filename2;
	hdfsFile writeFile = hdfsOpenFile(fs, writePath.c_str(), O_WRONLY|O_CREAT, 0, 0, 0);
    if (!writeFile) {
        fprintf(stderr, "Failed to open %s for writing!\n", writePath.c_str());
        exit(-1);
    }

	FILE *fp;
	errno_t err = fopen_s(&fp, filename1.c_str(), "rb");
	if (err)
		abort();
	char *buffer = new char[BUFFER_SIZE];
	while (!feof(fp)) {
		size_t count = fread(buffer, 1, BUFFER_SIZE, fp);
		tSize num_written_bytes = hdfsWrite(fs, writeFile, (void*)buffer, (tSize)count);
		if (hdfsFlush(fs, writeFile)) {
			fprintf(stderr, "Failed to 'flush' %s\n", writePath.c_str());
			exit(-1);
		}
	}

    hdfsCloseFile(fs, writeFile);
	hdfsDisconnect(fs);
}

int Platform::getRank() {
	return m_rank;
}

int Platform::getNProc() {
	return m_nproc;
}

void Platform::newIteration() {
	m_current = m_rank;
}

void Platform::endIteration() {
	MPI_Barrier(MPI_COMM_WORLD);
}

void Platform::broadcast(void *addr, int size) {
	MPI_Bcast(addr, size, MPI_CHAR, 0, MPI_COMM_WORLD);
}

void Platform::scatter(void *addr1, void *addr2, int size) {
	MPI_Scatter(addr1, size, MPI_CHAR, addr2, size, MPI_CHAR, 0, MPI_COMM_WORLD);
}

void Platform::gather(void *addr1, void *addr2, int size) {
	MPI_Gather(addr1, size, MPI_CHAR, addr2, size, MPI_CHAR, 0, MPI_COMM_WORLD);
}

void Platform::reduce(void *addr1, void *addr2, int type, int op, int size) {
	MPI_Datatype TYPE = MPI_DATATYPE_NULL;
	MPI_Op OP = MPI_OP_NULL;
	if (type == PPCHAR)
		TYPE = MPI_CHAR;
	else if (type == PPINT)
		TYPE = MPI_INT;
	else if (type == PPDOUBLE)
		TYPE = MPI_DOUBLE;
	if (op == PPMAX)
		OP = MPI_MAX;
	else if (op = PPMIN)
		OP = MPI_MIN;
	else if (op = PPSUM)
		OP = MPI_SUM;

	MPI_Reduce(addr1, addr2, size, TYPE, OP, 0, MPI_COMM_WORLD);
}


FILE *Platform::getNextFile() {
	if (m_current >= m_file.size())
		return NULL;
	hdfsFS fs = hdfsConnect(HDFS_ADDRESS, HDFS_PORT);
	string readPath = "/data/" + m_file[m_current];
	hdfsFile readFile = hdfsOpenFile(fs, readPath.c_str(), O_RDONLY, 0, 0, 0);
    if (!readFile) {
		fprintf(stderr, "Failed to open %s for reading!\n", readPath.c_str());
        exit(-1);
    }

	char *buffer = new char[BUFFER_SIZE];
	FILE *fp;
	errno_t err = fopen_s(&fp, m_file[m_current].c_str(), "wb");
	if (err)
		abort();
	while (true) {
		size_t count = hdfsRead(fs, readFile, (void*)buffer, BUFFER_SIZE);
		fwrite(buffer, 1, count, fp);
		if (count != BUFFER_SIZE)
			break;
	}
	fclose(fp);
	fopen_s(&fp, m_file[m_current].c_str(), "rb");
	printf("rank = %d gets file #%d\n", m_rank, m_current);
	m_current += m_nproc;
	return fp;
}

void Platform::finish() {
	MPI_Finalize();
}