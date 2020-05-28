#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>

int readcnt = 0; // ������� �������� ���������

// ��� �������� ���������������� ��������
sem_t resourceAccess;  // ������������ ������ �� ������/������ � ����� ������
sem_t readCountAccess; // �������� �� ������������� ��������� ����� ���������� readcnt
sem_t serviceQueue;    // �������� �� ������� ������������� ��������, ��������� ������� �������� (FIFO)

char sharedString[256] = { 'H', 'e', 'l', 'l', 'o', '!', 0 };

void write(char *string)
{
	strcpy(sharedString, string);
}

void* writer(void *string)
{
	printf("Writer is working!\n");

	sem_wait(&serviceQueue);   // ��� � ������� �� ������������
	sem_wait(&resourceAccess); // ����������� ������������ ������ � ����� ������
	sem_post(&serviceQueue);   // ��������� ��������� � ������� ���� �����������

	write((char*)string); // ������ � ����� ������

	sem_post(&resourceAccess); // ������������� ������ � ������ �� ������/������
}

void read()
{
	printf("%s\n", sharedString);
}

void* reader()
{
	printf("Reader is working!\n");

	sem_wait(&serviceQueue);    // ��� � ������� �� ������������
	sem_wait(&readCountAccess); // ����������� ������������ ������ � readcnt

	if (readcnt == 0)				// ���� � ������ ������ �������� ������ ���, �� 
		sem_wait(&resourceAccess);  // ����������� ������ � ������ ��� ��������� (�������� �������������)
	readcnt++;						// ��������� ������� �������� ���������

	sem_post(&serviceQueue);		// ��������� ��������� � ������� ���� �����������
	sem_post(&readCountAccess);		// ����������� ������ � readcnt

	read();							// ���������� ������

	sem_wait(&readCountAccess);		// ����������� ������������ ������ � readcnt
	readcnt--;						// ��������� ������� �������� ���������
	if (readcnt == 0)				// ���� ��������� ������ �� ��������, ��
		sem_post(&resourceAccess);	// ���������� ������ � ������ ��� ����
	sem_post(&readCountAccess);		// ���������� ������ � readcnt
}

int main()
{
	sem_init(&resourceAccess, 0, 1);  // ������������� ���������
	sem_init(&readCountAccess, 0, 1); //
	sem_init(&serviceQueue, 0, 1);	  //

	pthread_t thread_writer1; // �������������� �������
	pthread_t thread_writer2; //
	pthread_t thread_reader1; //
	pthread_t thread_reader2; //

	pthread_create(&thread_writer1, NULL, writer, "Bye!");		// ������������� �������
	pthread_create(&thread_reader1, NULL, reader, NULL);		//
	pthread_create(&thread_writer2, NULL, writer, "Goodbye!");	//
	pthread_create(&thread_reader2, NULL, reader, NULL);		//

	pthread_join(thread_writer1, NULL); // �������� ���������� ���� ��������� ������� 
	pthread_join(thread_reader1, NULL); // �� ��������� ���������������� ���������� ������ ���������
	pthread_join(thread_writer2, NULL);	//
	pthread_join(thread_reader2, NULL);	//

	return 0;
}