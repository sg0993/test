#include <stdio.h>
#include <string.h>
#ifndef __APPLE_CC__
 #ifndef _TMS320C6X
  #include <malloc.h>
 #endif
#else
 #include <malloc/malloc.h>
#endif
#ifndef WIN32
  #define strcpy_s(a,b,c) strcpy(a,c)
#endif

#include <stdlib.h>
#include <stdio.h>
#include <common/include/adt_vfile.h>

static FileSystem_t *pFileSystem = 0;
static int MaxFiles = 0;
static int NFiles = 0;
typedef struct
{
	char FileName[100];
	short int *FileArray;
}		FileEntry_t;

void VF_initFileSystem(int _MaxFiles)
{
	MaxFiles = _MaxFiles;
	pFileSystem = malloc(MaxFiles*sizeof(FileEntry_t));
	if (pFileSystem == 0)
	{
		printf("Unable to allocate file system\n");
		exit(0);
	}
}
void VF_closeFileSystem()
{
	free(pFileSystem);
}

void VF_addFile(char *pFName, short int *pFileArray)
{
	if (NFiles < MaxFiles)
	{
		pFileSystem[NFiles].FileArray = pFileArray;
		strcpy_s(pFileSystem[NFiles].FileName,99, pFName);
		NFiles += 1;
	}
	else 
	{
		printf("File %s not added to file system - max file count exceeded\n");
		exit(0);
	}
}


short int *FileNameToArray(char *FileName)
{
	int i=0;
	int Done = 0;
	int Found = 0;
	short int *pReturnValue = 0;
	while (!Done)
	{
		Found = strstr(FileName, pFileSystem[i].FileName) != 0;
		if (Found)
			return(pFileSystem[i].FileArray);
		Done = Found || (pFileSystem[i].FileArray == 0);
		Done |= (i == NFiles);
		i += 1;
	}
	return(0);	
}

VFile_t *VF_open_array(short int *pFileArray, char *Disposition)
{
	unsigned long int WordCount;
	VFile_t *pFile;
	WordCount = ((long int) pFileArray[1] << 16) + (unsigned short int) pFileArray[0];
	pFile = malloc(sizeof(VFile_t));
	if (pFile == 0)
		return(0);
	pFile->NWords = WordCount;
	if (Disposition[0] == 'r')
	{
		pFile->ReadDispositionFlag = 1;
		pFile->pStart = &pFileArray[2];
	}
	else
	{
		pFile->ReadDispositionFlag = 0;
		pFile->pStart = malloc(WordCount*sizeof(short int));
		if (pFile->pStart == 0)
		{
			free(pFile);
			return(0);
		}
	}
	pFile->WordIndex = 0;
	return(pFile);
}
VFile_t *VF_openFileForWrite(char *pFileName, char *Disposition)
{
	VFile_t *pFile = malloc(sizeof(VFile_t));
	pFile->WriteFileHandle = fopen(pFileName, Disposition);
	return(pFile);
}

VFile_t *VF_open(char *pFileName, char *Disposition)
{
	short int *pArray;
	VFile_t *pVFile;
	if (Disposition[0] == 'w')
	{
		pVFile = VF_openFileForWrite(pFileName, Disposition);
		if (pVFile == 0)
			printf("Unable to open file %s with disposition %s\n",pFileName,Disposition);
		return(pVFile);
	}
	pArray = FileNameToArray(pFileName);
	if (pArray != 0)
	{
		pVFile = (VF_open_array(pArray, Disposition));
		if (pVFile == 0)
			printf("Unable to open virtual file %s\n",pFileName);
		pVFile->WriteFileHandle = 0;
		return(pVFile);
	}
	else
	{
		printf("Unable to find file %s in virtual file system\n",pFileName);
		return(0);
	}
}


void VF_close(VFile_t *pFile)
{
	if (pFile->WriteFileHandle != 0)
		fclose(pFile->WriteFileHandle);
	//	if ((pFile != 0) && (pFile->ReadDispositionFlag == 0) && (pFile->pStart != 0))
//		free(pFile->pStart);
	if (pFile > (VFile_t *) 1)	//0 is null, 1 is for a dummy output file
		free(pFile);
}

int VF_read_words(short int *pBuff, int WordCount, VFile_t *pFile)
{
	int i;
	for (i = 0; i < WordCount && pFile->WordIndex < pFile->NWords; i++)
		pBuff[i] = pFile->pStart[pFile->WordIndex++];
	return(i);
}
//WriteWords not implemented
int VF_write_words(short int *pBuff, int WordCount, VFile_t *pFile)
{
	int i=0;
#ifndef __APPLE__
	return((int) fwrite(pBuff, 2, WordCount, pFile->WriteFileHandle));
#endif
	return(i);
}

int VF_feof(VFile_t *pFile)
{
	return(pFile->WordIndex == pFile->NWords);
}


