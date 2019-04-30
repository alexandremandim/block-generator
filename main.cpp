#include <cstdlib>
#include <string.h>
#include <stdio.h>  
#include <unistd.h> 
#include "./libs/generator/generator.h"
#include <iostream>

using namespace std;
using namespace GeneratorN;

int main(int argc, char *argv[]){

    clock_t tStart = clock();
    
    static string optString = "r:w:s:n:a:p:h?";
    
     /* Inicilizar variaveis */
    unsigned int blockSize = 4096, blocosAGerar = 1000, percentage_unique_blocks_analyze = 5, percentagem_compressao_entre_blocos = 50;
    string readPath = "./input.txt";
    string writePath = "~/Desktop/gerado/testes/";

    /* Args parser */
    int opt = getopt( argc, argv, optString.c_str() );
    while( opt != -1 ) {
        switch(opt)  
        {
            case 'r':  /* read path */
                readPath = optarg;
                cout << "Read path: " << readPath << endl;  
                break; 
            case 'w':  /* write path */
                writePath = optarg;
                cout << "Write path: " << writePath<< endl;    
                break; 
            case 's':  /* block size */
                blockSize = atoi(optarg);
                cout << "Block size: " << blockSize << endl; 
                /* todo: tem q ser multiplo de 4096 */
                break;  
            case 'n':  /* nr of blocks to generate */
                blocosAGerar = atoi(optarg);
                cout << "Nr blocks: " << blocosAGerar << endl; 
                break; 
            case 'a':  /* percentage to analyze (inter compression) */
                percentage_unique_blocks_analyze = atoi(optarg);
                cout << "Percentage to analyze: " << percentage_unique_blocks_analyze << endl;  
                break;  
            case 'p':   /* percentage to compress inter blocks */ 
                percentagem_compressao_entre_blocos = atoi(optarg);
               cout << "Percentage to compress inter blocks: " << percentagem_compressao_entre_blocos << endl;  
                /* todo: must be between 1-100 */
                break;  
            case 'h':   /* fall-through is intentional */
            case '?':
                /* TODO: Mostrar help */
                break;
                 
            default:
                /* You won't actually get here. */
                break; 
        }     
        opt = getopt( argc, argv, optString.c_str() );
    }
    
    GeneratorN::Generator generator(blockSize, blocosAGerar, percentage_unique_blocks_analyze, percentagem_compressao_entre_blocos, writePath, readPath);

    if (generator.initialize() == 1) {
        unsigned char *buffer = new unsigned char[blockSize]();
        FILE *write_ptr = fopen(writePath.c_str(),"wb+");

        if(write_ptr == NULL) {cout << "Cannot open file to write." << endl; return -1;}
 
        cout << "Start generating data..." << endl;
        for(unsigned int i = 0; i < blocosAGerar; i++){
            generator.nextBlock(buffer);
            fwrite(buffer, 1, blockSize, write_ptr);
        }
        cout << "Data generated with success." << endl;  
    }
    else {
        cout << "Error" << endl;
        return -1;
    }
    printf("Time taken: %.2fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);
    return 1;
}