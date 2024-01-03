#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

#define LIST_SIZE 9999
#define FILENAME "numbers_small.txt"

pthread_mutex_t file_lock;

struct file_section{
        FILE *input_file;
        long  start;
        long  stop;
};

int GLOBAL_LIST[LIST_SIZE];

/* Function to read the numbers in file for a particular section */
void * get_unique_numbers(void *args);

/* function returns the file size in bytes*/
long   get_file_size(FILE *input_file);

/* print global unique numbers*/
void   print_global_list();

/* move pointer to beginning on number*/
long   check_index(FILE *input_file, long ptr);

int main(){

        long index = 0;
        long file_size = 0;
        int  thread_count = 4;
        pthread_t read_thread[thread_count];

        if (pthread_mutex_init(&file_lock, NULL) < 0){
                printf("mutex init failed.\n");
                exit (EXIT_FAILURE);
        }

        // Opening the file for reading numbers
        // if file is not successfully opened print
        // error message and return
        FILE *input_file = fopen(FILENAME, "r");
        if (input_file == NULL){
                perror("Error opening file");
                exit (EXIT_FAILURE);
        }

        for (int i = 0; i < thread_count; i++){
                // Update the start and stop positions in each iteration
                file_size  = get_file_size(input_file);
                long start = index;
                long stop  = check_index(input_file, index + (file_size/thread_count));
                index = stop;

                if ( i == (thread_count - 1)){
                        stop = get_file_size(input_file);
                }

                struct file_section fs = {input_file, start, stop};
                pthread_create(read_thread + i, NULL, get_unique_numbers, (void *)&fs);
        }

        // wait for thread to join
        for (int i = 0; i < thread_count; i++){
                pthread_join(read_thread[i], NULL);
        }

        // cleanup
        pthread_mutex_destroy(&file_lock);
        fclose(input_file);

        // Print the global unique numbers
        print_global_list();
        return 0;
}

void * get_unique_numbers(void *args){

        struct file_section *fs = (struct file_section *)args;

        FILE *input_file = fs->input_file;
        long start = fs->start;
        long stop  = fs->stop;

        pthread_mutex_lock(&file_lock);

        fseek(input_file, start, SEEK_SET);
        int temp = 0;
        fscanf (input_file, "%d", &temp);
        while (!feof(input_file)){
                GLOBAL_LIST[temp] = 1;
                fscanf (input_file, "%d", &temp);
                if ( ftell(input_file) >= stop ){
                        break;
                }
        }

        // reset to start of file
        fseek(input_file, 0, SEEK_SET);
        pthread_mutex_unlock(&file_lock);

        return NULL;
}

long   get_file_size(FILE *input_file){
        // Move the cursor to the end of file
        // then count the offset
        fseek(input_file, 0, SEEK_END);
        long size = ftell(input_file);
        // reset to start of file
        fseek(input_file, 0, SEEK_SET);
        return size;
}

void   print_global_list(){
        // Only print the indexes that are set
        for ( int i=0; i < LIST_SIZE; i++){
                if (GLOBAL_LIST[i]){
                        printf("%d \n", i);
                }
        }
}

long   check_index(FILE *input_file, long ptr){

        pthread_mutex_lock(&file_lock);
        fseek(input_file, ptr, SEEK_SET);
        char ch = fgetc(input_file);
        while ( ch != ' '){
                fseek(input_file, --ptr, SEEK_SET);
                ch = fgetc(input_file);
        }
        ptr++;
        fseek(input_file, 0, SEEK_SET);
        pthread_mutex_unlock(&file_lock);
        return ptr;
}