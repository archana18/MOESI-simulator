#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void process(int cache_number, char command, int line_number, char* source);
void broadcast(int cache_number, char command, int line_number);

FILE *input_file;
char *input;

FILE *output_file;
char *output; 

int CACHE_COUNT = 3;
int CACHE_LINE_COUNT = 4;

typedef struct cache {
    char *cache_lines;
} cache_t;

cache_t *new_cache();

typedef struct memory {
    cache_t **caches;
} memory_t;

memory_t *new_memory();

void init_dirty_bits();

memory_t *mem;
int *dirty_bits;

int main(int argc, char *argv[]){
     if(argc != 1){
        //check for correct program usage
        printf("usage: p1 < input_filename > output_filename\n");
        exit(1);
    }

    input_file = stdin;
    output_file = stdout;  

    off_t input_filesize;

    if(fseeko(input_file, 0L, SEEK_END) == -1){
        perror("<fseek() failed unexpectedly");
        exit(1);
    }

    input_filesize = ftello(input_file);
    if(input_filesize == -1){
        perror("<ftello() failed unexpectedly>");
        exit(1);
    }

    if(fseeko(input_file, 0L, SEEK_SET) == -1){
        perror("<fseek() failed unexpectedly>");
        exit(1);
    };
   
    //allocate space for input
    input = (char*)malloc(input_filesize*sizeof(char)+1);
    if(input == NULL){
        perror("<malloc() failed unexpectedly>");
        exit(1);
    }

    off_t OUTPUT_LINESIZE = 6;

    //allocate space for output
    output = (char*)malloc(OUTPUT_LINESIZE*sizeof(char)+1);
    if(output == NULL){
        perror("<malloc() failed unexpectedly>");
        exit(1);
    }
    //read-in input
    int nread;

    nread = fread(input, sizeof(char), input_filesize, input_file);
    if(nread != input_filesize){
        perror("<fread() failed unexpectedly>");
        exit(1);
    }       
    input[input_filesize]='\0';

    mem = new_memory();
    init_dirty_bits();
      
    char * parse;
    char * token;

    char cache_number;
    char command;
    char line_number;

    token = strtok_r(input, "\n", &parse);
 
    cache_number = token[0] - '0';
    command = token[1];
    line_number = token[2] - '0';

    fprintf(output_file, "%d%c%d\n", cache_number, command, line_number); 
    process(cache_number, command, line_number, "processor");           

    while(token = strtok_r(NULL, "\n", &parse)){
        cache_number = token[0] - '0';
        command = token[1];
        line_number = token[2] - '0';

        fprintf(output_file, "%d%c%d\n", cache_number, command, line_number); 
        process(cache_number, command, line_number, "processor");         
    }    
    
    //cleanup
    free(dirty_bits);

    int i, j;
    for(i = 0; i < CACHE_COUNT; i++){
        free(mem->caches[i]->cache_lines);
        free(mem->caches[i]);
    }
    free(mem->caches);
    free(mem);

    free(input);
    free(output);
    fclose(output_file);
    fclose(input_file);
    return 0;
}

cache_t *new_cache(){
    cache_t *cache;
    cache = (cache_t *)malloc(sizeof(cache_t));
    if(cache == NULL){
        perror("<malloc() failed unexpectedly>");
        exit(1);
    }
    
    cache->cache_lines = (char *)malloc(sizeof(char) * CACHE_LINE_COUNT);
    if(cache->cache_lines == NULL){
        perror("<malloc() failed unexpectedly>");
        exit(1);
    }

    int i;
    for(i = 0; i < CACHE_LINE_COUNT; i++){
        cache->cache_lines[i] = 'i';
    }

    return cache;    
}

memory_t *new_memory(){
    memory_t *memory;
    memory = (memory_t *)malloc(sizeof(memory_t));
    if(memory == NULL){
        perror("<malloc() failed unexpectedly>");
        exit(1);
    }
    
    memory->caches = (cache_t **)malloc(sizeof(cache_t *) * CACHE_COUNT);
    if(memory->caches == NULL){
        perror("<malloc() failed unexpectedly>");
        exit(1);
    }

    int i;
    for(i = 0; i < CACHE_COUNT; i++){
        memory->caches[i] = new_cache();
    }

    return memory;    
}

void init_dirty_bits(){
    dirty_bits = (int *)malloc(sizeof(int) * CACHE_LINE_COUNT);
    if(dirty_bits == NULL){
        perror("<malloc() failed unexpectedly>");
        exit(1);
    }

    int i;
    for(i = 0; i < CACHE_LINE_COUNT; i++){
        dirty_bits[i] = 0;
    }
}

void process(int cache_number, char command, int line_number, char* source){
    if(mem->caches[cache_number]->cache_lines[line_number] == 'm'){
        if(command == 'r'){
            if(source == "processor"){
                mem->caches[cache_number]->cache_lines[line_number] = 'm';
                fprintf(output_file, "Cache %d, Read %d\n", cache_number, line_number); 
                fprintf(output_file, "Hit\n"); 
                fprintf(output_file, "M -> M\n");
                fprintf(output_file, "\n");
            }
            else if(source == "bus"){
                mem->caches[cache_number]->cache_lines[line_number] = 'o';
                fprintf(output_file, "Cache %d, Probe Read %d\n", cache_number, line_number); 

                if(dirty_bits[line_number]){
                    fprintf(output_file, "Hit Dirty\n"); 
                }
                else{
                    fprintf(output_file, "Hit\n"); 
                }

                fprintf(output_file, "M -> O\n");
                fprintf(output_file, "End Read Probe\n");
                fprintf(output_file, "\n");
            }
        }
        else if(command == 'w'){
            if(source == "processor"){
                mem->caches[cache_number]->cache_lines[line_number] = 'm';
                fprintf(output_file, "Cache %d, Write %d\n", cache_number, line_number); 
                fprintf(output_file, "Hit\n"); 
                fprintf(output_file, "M -> M\n");
                fprintf(output_file, "\n");
                dirty_bits[line_number] = 1;
            }
            else if(source == "bus"){
                mem->caches[cache_number]->cache_lines[line_number] = 'i';
                fprintf(output_file, "Cache %d, Probe Write %d\n", cache_number, line_number); 
                fprintf(output_file, "Hit\n"); 
                fprintf(output_file, "M -> I\n");
                fprintf(output_file, "End Write Probe\n");
                fprintf(output_file, "\n");
            }
        }
    }
    else if(mem->caches[cache_number]->cache_lines[line_number] == 'o'){
        if(command == 'r'){
            if(source == "processor"){
                mem->caches[cache_number]->cache_lines[line_number] = 'o';
                fprintf(output_file, "Cache %d, Read %d\n", cache_number, line_number); 
                fprintf(output_file, "Hit\n"); 
                fprintf(output_file, "O -> O\n");
                fprintf(output_file, "\n");
            }
            else if(source == "bus"){
                mem->caches[cache_number]->cache_lines[line_number] = 'o';
                fprintf(output_file, "Cache %d, Probe Read %d\n", cache_number, line_number); 

                if(dirty_bits[line_number]){
                    fprintf(output_file, "Hit Dirty\n"); 
                }
                else{
                    fprintf(output_file, "Hit\n"); 
                }

                fprintf(output_file, "O -> O\n");
                fprintf(output_file, "End Read Probe\n");
                fprintf(output_file, "\n");
            }
        }
        else if(command == 'w'){
            if(source == "processor"){
                mem->caches[cache_number]->cache_lines[line_number] = 'm';
                fprintf(output_file, "Cache %d, Write %d\n", cache_number, line_number); 
                fprintf(output_file, "Hit\n"); 
                fprintf(output_file, "O -> M\n");
                fprintf(output_file, "\n");
                broadcast(cache_number, command, line_number);
                dirty_bits[line_number] = 1;
            }
            else if(source == "bus"){
                mem->caches[cache_number]->cache_lines[line_number] = 'i';
                fprintf(output_file, "Cache %d, Probe Write %d\n", cache_number, line_number); 
                fprintf(output_file, "Hit\n"); 
                fprintf(output_file, "O -> I\n");
                fprintf(output_file, "End Write Probe\n");
                fprintf(output_file, "\n");
            }
        }
    }
    else if(mem->caches[cache_number]->cache_lines[line_number] == 'e'){
        if(command == 'r'){
            if(source == "processor"){
                mem->caches[cache_number]->cache_lines[line_number] = 'e';
                fprintf(output_file, "Cache %d, Read %d\n", cache_number, line_number); 
                fprintf(output_file, "Hit\n"); 
                fprintf(output_file, "E -> E\n");
                fprintf(output_file, "\n");
            }
            else if(source == "bus"){
                mem->caches[cache_number]->cache_lines[line_number] = 's';
                fprintf(output_file, "Cache %d, Probe Read %d\n", cache_number, line_number); 

                if(dirty_bits[line_number]){
                    fprintf(output_file, "Hit Dirty\n"); 
                }
                else{
                    fprintf(output_file, "Hit\n"); 
                }

                fprintf(output_file, "E -> S\n");
                fprintf(output_file, "End Reaad Probe\n");
                fprintf(output_file, "\n");
            }
        }
        else if(command == 'w'){
            if(source == "processor"){
                mem->caches[cache_number]->cache_lines[line_number] = 'm';
                fprintf(output_file, "Cache %d, Write %d\n", cache_number, line_number); 
                fprintf(output_file, "Hit\n"); 
                fprintf(output_file, "E -> M\n");
                fprintf(output_file, "\n");
                dirty_bits[line_number] = 1;
            }
            else if(source == "bus"){
                mem->caches[cache_number]->cache_lines[line_number] = 'i';
                fprintf(output_file, "Cache %d, Probe Write %d\n", cache_number, line_number); 
                fprintf(output_file, "Hit\n"); 
                fprintf(output_file, "E -> I\n");
                fprintf(output_file, "End Write Probe\n");
                fprintf(output_file, "\n");
            }
        }
    }
    else if(mem->caches[cache_number]->cache_lines[line_number] == 's'){
        if(command == 'r'){
            if(source == "processor"){
                mem->caches[cache_number]->cache_lines[line_number] = 's';
                fprintf(output_file, "Cache %d, Read %d\n", cache_number, line_number); 
                fprintf(output_file, "Hit\n"); 
                fprintf(output_file, "S -> S\n");
                fprintf(output_file, "\n");
            }
            else if(source == "bus"){
                mem->caches[cache_number]->cache_lines[line_number] = 's';
                fprintf(output_file, "Cache %d, Probe Read %d\n", cache_number, line_number); 

                if(dirty_bits[line_number]){
                    fprintf(output_file, "Hit Dirty\n"); 
                }
                else{
                    fprintf(output_file, "Hit\n"); 
                }

                fprintf(output_file, "Hit\n"); 
                fprintf(output_file, "S -> S\n");
                fprintf(output_file, "End Read Probe\n");
                fprintf(output_file, "\n");
            }
        }
        else if(command == 'w'){
            if(source == "processor"){
                mem->caches[cache_number]->cache_lines[line_number] = 'm';
                fprintf(output_file, "Cache %d, Write %d\n", cache_number, line_number); 
                fprintf(output_file, "Hit\n"); 
                fprintf(output_file, "S -> M\n");
                fprintf(output_file, "\n");
                broadcast(cache_number, command, line_number);
                dirty_bits[line_number] = 1;
            }
            else if(source == "bus"){
                mem->caches[cache_number]->cache_lines[line_number] = 'i';
                fprintf(output_file, "Cache %d, Probe Write %d\n", cache_number, line_number); 
                fprintf(output_file, "Hit\n"); 
                fprintf(output_file, "S -> I\n");
                fprintf(output_file, "End Write Probe\n");
                fprintf(output_file, "\n");
            }
        }
    }
    else if(mem->caches[cache_number]->cache_lines[line_number] == 'i'){
        if(command == 'r'){
            if(source == "processor"){
                int exclusive = 1;
                
                int j;
                for(j = 0; j < CACHE_COUNT; j++){
                    if(j != cache_number && mem->caches[j]->cache_lines[line_number] != 'i'){
                        exclusive = 0;
                        break;
                    }
                }
                if(exclusive){
                    mem->caches[cache_number]->cache_lines[line_number] = 'e';
                    fprintf(output_file, "Cache %d, Read %d\n", cache_number, line_number); 
                    fprintf(output_file, "Miss\n"); 
                    fprintf(output_file, "I -> E\n");
                    fprintf(output_file, "\n");
                    broadcast(cache_number, command, line_number);
                }
                else{
                    mem->caches[cache_number]->cache_lines[line_number] = 's';
                    fprintf(output_file, "Cache %d, Read %d\n", cache_number, line_number); 
                    fprintf(output_file, "Miss\n"); 
                    fprintf(output_file, "I -> S\n");
                    fprintf(output_file, "\n");
                    broadcast(cache_number, command, line_number);
                }
       
            }
            else if(source == "bus"){
                mem->caches[cache_number]->cache_lines[line_number] = 'i';
                fprintf(output_file, "Cache %d, Probe Read %d\n", cache_number, line_number); 
                fprintf(output_file, "Miss\n"); 
                fprintf(output_file, "I -> I\n");
                fprintf(output_file, "End Read Probe\n");
                fprintf(output_file, "\n");
            }
        }
        else if(command == 'w'){
            if(source == "processor"){
                mem->caches[cache_number]->cache_lines[line_number] = 'm';
                fprintf(output_file, "Cache %d, Write %d\n", cache_number, line_number); 
                fprintf(output_file, "Miss\n"); 
                fprintf(output_file, "I -> M\n");
                fprintf(output_file, "\n");
                broadcast(cache_number, command, line_number);
                dirty_bits[line_number] = 0;
            }
            else if(source == "bus"){
                mem->caches[cache_number]->cache_lines[line_number] = 'i';
                fprintf(output_file, "Cache %d, Probe Write %d\n", cache_number, line_number); 
                fprintf(output_file, "Miss\n"); 
                fprintf(output_file, "I -> I\n");
                fprintf(output_file, "End Write Probe\n");
                fprintf(output_file, "\n");
            }
        }
    }
}

void broadcast(int cache_number, char command, int line_number){
    int i;
    for(i = 0; i < CACHE_COUNT; i++){
        if(i != cache_number){
            process(i, command, line_number, "bus");
        }
    }
}

