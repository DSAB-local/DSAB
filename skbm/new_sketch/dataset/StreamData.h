#ifndef STREAM_DATA_H
#define STREAM_DATA_H

#include "stdio.h"
#include "stdlib.h"

class StreamData {
private:
	FILE *fp;
public:
	const int bytesPerStr;
	StreamData(const char *filePath, const int _bytesPerStr): bytesPerStr(_bytesPerStr) {
		if (!(fp = fopen(filePath, "rb"))) {
			printf("Wrong file path: %s\n", filePath);
			return;
		}
	}
	bool GetNext(char *str) {
		return fread(str, sizeof(char), bytesPerStr, fp) == bytesPerStr;
	}
	void Reset() {
		fseek(fp, 0, 0);
	}
	~StreamData() {
		fclose(fp);
	}
};

#endif