#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>

int readcnt = 0; // Счётчик активных читателей

// Все семафоры инициализируются единицей
sem_t resourceAccess;  // Контролирует доступ на запись/чтение к общей памяти
sem_t readCountAccess; // Отвечает за синхронизацию изменений общей переменной readcnt
sem_t serviceQueue;    // Отвечает за честное распределение ресурсов, сохраняет порядок запросов (FIFO)

char sharedString[256] = { 'H', 'e', 'l', 'l', 'o', '!', 0 };

void write(char *string)
{
	strcpy(sharedString, string);
}

void* writer(void *string)
{
	printf("Writer is working!\n");

	sem_wait(&serviceQueue);   // Ждём в очереди на обслуживание
	sem_wait(&resourceAccess); // Запрашиваем эксклюзивный доступ к общей памяти
	sem_post(&serviceQueue);   // Разрешаем следующим в очереди быть обслужеными

	write((char*)string); // Запись в общую память

	sem_post(&resourceAccess); // Освобобождаем доступ к памяти на чтение/запись
}

void read()
{
	printf("%s\n", sharedString);
}

void* reader()
{
	printf("Reader is working!\n");

	sem_wait(&serviceQueue);    // Ждём в очереди на обслуживание
	sem_wait(&readCountAccess); // Запрашиваем эксклюзивный доступ к readcnt

	if (readcnt == 0)				// Если в данный момент читателй больше нет, то 
		sem_wait(&resourceAccess);  // Запрашиваем доступ к памяти для читателей (писатели заблокированы)
	readcnt++;						// Обновляем счётчик активных читателей

	sem_post(&serviceQueue);		// Разрешаем следующим в очереди быть обслужеными
	sem_post(&readCountAccess);		// Освобождаем доступ к readcnt

	read();							// Производим чтение

	sem_wait(&readCountAccess);		// Запрашиваем эксклюзивный доступ к readcnt
	readcnt--;						// Обновляем счётчик активных читателей
	if (readcnt == 0)				// Если читателей больше не осталось, то
		sem_post(&resourceAccess);	// Освободить доступ к памяти для всех
	sem_post(&readCountAccess);		// Освободить доступ к readcnt
}

int main()
{
	sem_init(&resourceAccess, 0, 1);  // Инициализация семафоров
	sem_init(&readCountAccess, 0, 1); //
	sem_init(&serviceQueue, 0, 1);	  //

	pthread_t thread_writer1; // Идентификаторы потоков
	pthread_t thread_writer2; //
	pthread_t thread_reader1; //
	pthread_t thread_reader2; //

	pthread_create(&thread_writer1, NULL, writer, "Bye!");		// Инициализация потоков
	pthread_create(&thread_reader1, NULL, reader, NULL);		//
	pthread_create(&thread_writer2, NULL, writer, "Goodbye!");	//
	pthread_create(&thread_reader2, NULL, reader, NULL);		//

	pthread_join(thread_writer1, NULL); // Ожидание завершения всех созданных потоков 
	pthread_join(thread_reader1, NULL); // во избежания преждевременного завершения работы программы
	pthread_join(thread_writer2, NULL);	//
	pthread_join(thread_reader2, NULL);	//

	return 0;
}