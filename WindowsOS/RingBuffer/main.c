#include "RingBuffer.h"

#include <stdio.h>
#include <string.h>
#define BUF_ORDER 1
#define THREAD_NUM 40

typedef struct {
	RingBuf *rb;
	int     iNum;
} testParamRingBuf;

static DWORD WINAPI writeThreadFunction(LPVOID lpParam)
{
	int i = 0;
	testParamRingBuf *param = (testParamRingBuf*) lpParam;
	RingBuf *rb = param->rb;
	char test[100] = "[      ]C:\\Users\\Valera\\Documents\\Visual Studio 2012\\Projects\\RingBuffer\\RingBuffer\\ringBuffer.log\n";

	sprintf(&test[1], "%d", param->iNum);
	for (;i < 101;++i) {
		if (i % 10 == 0) {
			flushRingBuf(rb);
		}
		
		writeInRingBuf(rb, test, sizeof(test));
	}
	return 0;
}
int main(int argc, char *argv[])
{
	int i = 0;
	const char path[] = "C:\\Users\\Valera\\Documents\\Visual Studio 2012\\Projects\\RingBuffer\\RingBuffer\\ringBuffer.log";
	HANDLE hThread[THREAD_NUM];
	RingBuf *rb = constructRingBuf(BUF_ORDER, path);
	testParamRingBuf testParam[THREAD_NUM];

	for (i = 0; i < THREAD_NUM ; ++i) {
		testParam[i].iNum = i;
		testParam[i].rb = rb;
		hThread[i] = CreateThread(  NULL,                  
                                    0,                      
                                    writeThreadFunction,   
                                    &testParam[i],			 
                                    0,                       
                                    NULL	);  
	}
	
	WaitForMultipleObjects(THREAD_NUM, hThread, TRUE, INFINITE);
	destructRingBuf(rb);
	system("pause");

	return 0;
}