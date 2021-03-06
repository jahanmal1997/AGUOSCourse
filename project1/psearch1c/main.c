#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/mman.h>

#include "../common.h"

#define READ 0
#define WRITE 1

sem_t* semaphores;
int p[2];
int main(int argc, char *argv[]) {

    /* will get commented */
    //char* a[6] = {"a", "a", "2", "b.txt", "a.txt", "output.txt"};
    //argv = a;
	/* will get commented */

	char *fileToRead;
	char *childOutput;
	pid_t pid;
	int i;
	char *searchString = argv[1];
	int numberOfFilesToSearch = atoi(argv[2]);
	pipe(p);

	semaphores = mmap(NULL, numberOfFilesToSearch*sizeof(sem_t), PROT_READ | PROT_WRITE,
				MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if (semaphores == MAP_FAILED)
		printf("omg"), exit(0);

	char *outputFilename = argv[numberOfFilesToSearch + 3];

	for (i = 0; i <= numberOfFilesToSearch; i++) {
		sem_init(&semaphores[i], numberOfFilesToSearch, 0);
	}

	sem_post(&semaphores[0]);

	/* Start children. */

	for (i = 0; i < numberOfFilesToSearch; ++i) {
	  fileToRead = argv[3+i];

	  if ((pid = fork()) < 0) {
	    perror("fork");
	    abort();
	  } else if (pid == 0) {

		  sem_wait(&semaphores[i]);

		  close(p[READ]);
		  childOutput = DoWorkInChild(searchString, fileToRead);
		  childOutput = concat(childOutput, "\n");
		  write(p[WRITE], childOutput, strlen(childOutput));

		  close(p[WRITE]);
		  if(i != numberOfFilesToSearch-1)
			  sem_post(&semaphores[i + 1]);
		  exit(0);
	  }

	}

	int j = numberOfFilesToSearch;

	char inputBuffer[100];
	char *totalOutput = (char *) malloc(1);
	int nbytes;
	close(p[WRITE]);

	for(j = 0; j < numberOfFilesToSearch; j++) {

		while((nbytes = read(p[READ],inputBuffer,100)) > 0)  {
			totalOutput = concat(totalOutput, inputBuffer);
			memset (inputBuffer, 0, sizeof(inputBuffer));
		}

	}
	close(p[READ]);

	FILE *outputFile = fopen(outputFilename, "w");
	fprintf(outputFile, "%s", totalOutput);
    fclose(outputFile);
	return EXIT_SUCCESS;
}
