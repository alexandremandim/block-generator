#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <vector>
#include <random>
#include <string> 
#include <tuple>

using namespace std;

class Linha{
    public:
        int nrCopies, nrBlocks, nrBase;
        vector<double> compression; 
};

class Generator {

    private:
        int blockSize = 4096, total_blocks = 0, total_unique_blocks = 0; /* bytes */
        tuple<double, double> compressions_inter_blocks_interval;
        vector<Linha> linhas;
        vector<double> weights;
        vector<unsigned char*> modelos;

        /* Returns random block ID from a line */
        int getRandomBlockFromLine(Linha l) {

            if (l.nrCopies == 0) {
                int r = rand();
                return (r);
            } else {
                random_device rd; //Will be used to obtain a seed for the random number engine
                mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
                uniform_int_distribution<> dis(1, l.nrBlocks);

                return dis(gen);
            }
        }

        /* Returns random block ID from a line */
        int getRandomBlockFromLine2(Linha l) {

            random_device rd; //Will be used to obtain a seed for the random number engine
            mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
            uniform_int_distribution<> dis(1, l.nrBlocks);

            return dis(gen);
        }

        /* Returns random line from inputfile */
        Linha getLinha() {

            unsigned seedDistr = std::chrono::system_clock::now().time_since_epoch().count();
            std::default_random_engine generator(seedDistr);

            discrete_distribution<> distribution(weights.begin(), weights.end());

            return linhas.at(distribution(generator));
        }

        /* Returns compression index from a block */
        int giveMyCompression(Linha linhaAleatoria, int randomBlockID) {

            /* Como o ID é completamente aleatório, também a compressão vai ser com base nas percentagens */
            if(linhaAleatoria.nrCopies == 0){
                /* Get random compression */
                unsigned seedDistr = std::chrono::system_clock::now().time_since_epoch().count();
                std::default_random_engine generator(seedDistr);
                discrete_distribution<> distribution(linhaAleatoria.compression.begin(), linhaAleatoria.compression.end());
                /* Devolve um indice (0-9)*/
                int r = distribution(generator);
                return r;
            }
            
            /* Cada bloco tem uma compressão ligada a ele */
            else{
                double myPercent = ((double) randomBlockID) / linhaAleatoria.nrBlocks * 100;
                double aux = 0;

                for (int i = 0; i < 10; i++) {
                    aux += linhaAleatoria.compression.at(i);
                    if (myPercent <= aux) {
                        return i;
                    }
                }
                return 9;
            }
        }

        /* Returns compression index from a block */
        int giveMyCompression2(Linha linhaAleatoria, int randomBlockID) {

            double myPercent = ((double) randomBlockID) / linhaAleatoria.nrBlocks * 100;
            double aux = 0;

            for (int i = 0; i < 10; i++) {
                aux += linhaAleatoria.compression.at(i);
                if (myPercent <= aux) {
                    return i;
                }
            }
            return 9;
        }
        
        /* Fill buffer with data with a blockID and compression */
        void generate_data(unsigned char* buffer, int blockKey, int compression){
            
            unsigned char* bufferModelo = modelos.at(compression);
            
            /* Copiar o buffer (Posso otimizar este passo para ser mais rapido).
             * Em vez de copiar insiro o blockKey no bufferModelo e retorno-o */
            for(int i = 0; i < blockSize; i ++)
                buffer[i] = bufferModelo[i];
            
            /* alterar os primeiros 4 bytes */
            for (size_t i = 0; i < sizeof(blockKey); ++i)
                buffer[i] = *((unsigned char *)&blockKey + i);
            
        }
        
        /* Auxiliar Function to Generate Block Models*/
        static void generate4096BytesBlock(unsigned char* buffer, double seed){
            
            double c = 1;
            
            /* Gerar o bloco */
            for(int contador = 0; contador < 4096/sizeof(c); contador++){ 
                for (size_t i = 0; i < sizeof(c); ++i) 
                  buffer[(sizeof(c)*contador)+i] = *((unsigned char *)&c + i);
                c = c * seed;
            }
        }

        /* Returns a block with 0% compression */
        unsigned char* generateBlockWithNoCompression(int blockSize, double seed){
            if(blockSize < 4096) return NULL;

            int nrBlocos4096 = blockSize/4096;
            int tamanhoTotal = nrBlocos4096*4096;

            unsigned char *buffer = (unsigned char *)malloc(sizeof(unsigned char)*(tamanhoTotal));

            /* Gerar alguma aleatoriedade */
            for(int i = 0; i< nrBlocos4096; i++)
                generate4096BytesBlock(&(buffer[i*4096]), seed);

            return buffer;
        }

        /* This function transforms a block with no compression to compress 'rate'%
         * rate is the rate compression (1 - 100) */
        void transformBlockToCompress(unsigned char* buffer, int rate, int tamanhoBuffer){

            if(rate < 1 || rate > 100 || tamanhoBuffer <= 0 || buffer == NULL) return;
            rate = rate + 0;
            /* Tamanho do semi-bloco a ser repetido (1% do tamanho total) */
            int nrBlocosADuplicar = tamanhoBuffer/100, posicaoBuffer = 0, contador = 0;

            /* Começa na posicao 0 do buffer, vai andar de 1 em 1 % do buffer. 
             * Vai copiar os primeiros 1% dos bytes até à taxa pretendida. Ex: se a taxa for 40% vai copiar
             os primeiros 1% 40 vezes
            */

            while(contador < rate && posicaoBuffer < (tamanhoBuffer-nrBlocosADuplicar)){
                for(int posicaoAuxiliar = 0; posicaoAuxiliar < nrBlocosADuplicar; posicaoAuxiliar++)
                    buffer[posicaoBuffer + posicaoAuxiliar] = buffer[posicaoAuxiliar];

                posicaoBuffer = posicaoBuffer + nrBlocosADuplicar;
                contador++;
            }

        }

        /* Returns a block with a given size and compression. 
         * Compression must be between 1 and 99. 
         * The blockSize must be multiple of 4096
         */
        unsigned char* blockModel(int blockSize, int compression, double seed){

            if(compression < 1 || compression > 99) return NULL;
            int size = blockSize/4096; size = size * 4096; /* blockSize must be multiple of 4096 */

            unsigned char* buffer = generateBlockWithNoCompression(size, seed);
            if(buffer != NULL) transformBlockToCompress(buffer,compression,size);

            return buffer;
        }
        
        /* 
         * different_nc_ratio: ratio of blocks with different non-compressive areas needed
         *                     to achive wanted media
         * return: 1 -> ok, -1 -> error 
         */
        int loadModels(int different_nc_ratio){

           /* TODO: Gerar outros modelos com ares de não compressão diferentes, 
           de forma a tornar different_nc_ratio do dataset diferente. */
            

            cout << "Creating models..." << endl;
            srand (time ( NULL));

            /* Gerar um valor random (double) que vai ser utilizado para gerar um bloco sem compressão, 
            através da multiplicação sucessivada deste valor */
            /* Neste caso, como geramos apenas uma seed, todos os modelos vão ser iguais na parte que não comprime */

            double max = 1.1; double min = 1.9;
            double seed =  min + (rand() / (RAND_MAX / (max - min)));

            for(int compression = 5; compression <= 95; compression=compression+10){        

                unsigned char *buffer = blockModel(this->blockSize, compression, seed);
                
                if(buffer != NULL)
                    this->modelos.push_back(buffer);
                else {
                    cout << "Error loading models..." << endl;
                    return -1;
                }
            }
            cout << "Models created with success." << endl << endl;
            return 1;
        }

        /* return block compression or -1 (error) */
        int get_block_compression_by_id(int block_id){

            Linha l;
            int min_id_line, max_id_line, id_line;

            for (vector<Linha>::iterator it = linhas.begin() ; it != linhas.end(); ++it){
                min_id_line = it->nrBase + 1;
                max_id_line = it->nrBase + it->nrBlocks;
                id_line = block_id - it->nrBase;

                if(block_id >= min_id_line && block_id <= max_id_line){ /* É esta a linha do bloco */
                    int x = giveMyCompression2(*it, id_line);
                    return x * 10;
                }
            }
            return -1;
        }

        /* return tuple (min_media, max_media) */
        tuple<double, double> get_media_inter(int percentage_interval){
            
            int id_block_analyze = 1, previous_compression = -1, current_compression = -1, nr_pairs = 0;
            int new_media_max = 0, new_media_min = 0, total_media_min = 0, total_media_max = 0;
            int minimum = 0, maximum = 0;

            if(!(percentage_interval > 0 && percentage_interval < 100))   return make_tuple(-1,-1);

            for(int i = 0; i < this->total_unique_blocks*percentage_interval/100; i++){
                /* Generate new block */
                random_device rd;
                mt19937 gen(rd());
                uniform_int_distribution<> dis(1, this->total_unique_blocks);
                id_block_analyze = dis(gen);

                /* Get compression */
                current_compression = get_block_compression_by_id(id_block_analyze);
                //cout << "Block: " << id_block_analyze << "\tCompress: " << current_compression << endl;
            
                /* Calculate media */
                if(previous_compression == -1){
                    previous_compression = current_compression;           /* É o 1º elemento do par para calcular a media */
                }
                else{
                    new_media_max = (current_compression + previous_compression + min((100-current_compression),(100-previous_compression)))/2;
                    new_media_min = (current_compression + previous_compression)/2;
                    nr_pairs++; previous_compression = -1;
                    minimum += new_media_min; maximum += new_media_max;
                }
            }
            cout << "Values estimated based on " << nr_pairs << " pairs." << endl;

            return make_tuple(minimum/nr_pairs, maximum/nr_pairs);
        }
        
    public:
                
        void nextBlock(unsigned char* buffer) {

            Linha linhaAleatoria = getLinha();
            int myCompression = 0;

            int randomBlockID = getRandomBlockFromLine2(linhaAleatoria);
            myCompression = giveMyCompression2(linhaAleatoria, randomBlockID);
            
            int blockKey = randomBlockID + linhaAleatoria.nrBase;
            generate_data(buffer, blockKey, myCompression);
        }

        /* Read input file for duplicate and compression distribution.
            Generate Models to generate data
         * path: path to input file
         * block_size: size of each block
         * nrBlocksToGenerate: number of blocks generated
         * percentage: percetage of unique blocks used to estimate compression inter blocks
         * Return 1:ok -1:error
        */
        int initialize(string path, int block_Size, int nrBlocksToGenerate, int percentage, int wanted_media_compression_inter_blocks) {
            if(block_Size < 4096) return -1;

            int nrBlocos4096 = block_Size/4096;
            this->blockSize = nrBlocos4096*4096;
            this->total_blocks = nrBlocksToGenerate;
             
            /* Reading Input File */
            int nrBaseAux = 0;
            istringstream iss;
            string line;

            ifstream fin(path);
            if (!fin) {
                cout << "Can't open file " << path << endl;
                return -1;
            }
            
            /* Reading line by line */
            cout << endl << "Reading file..." << path << endl;
            while (getline(fin, line)) {
                Linha newLine;

                newLine.nrBase = nrBaseAux;
                stringstream ss(line);
                string token;
                double probabilidadeLinha;

                if (getline(ss, token, ' ')) {
                    newLine.nrCopies = stoi(token);
                    if (getline(ss, token, ' ')) {
                        probabilidadeLinha = stof(token);

                        newLine.nrBlocks = (int) (nrBlocksToGenerate * (probabilidadeLinha / 100) / (newLine.nrCopies + 1));
                        if(newLine.nrBlocks == 0) newLine.nrBlocks = 1;
                        nrBaseAux = nrBaseAux + newLine.nrBlocks;
                    }
                }

                /* restantes eltos da linha (compressoes) */
                while (getline(ss, token, ' ')){
                    newLine.compression.push_back(stof(token));
                }
                
                this->linhas.push_back(newLine);
                this->weights.push_back(probabilidadeLinha);
            }
            cout << "File readed with success." << endl << endl;
            
            /* Blocos únicos */
            for (vector<Linha>::iterator it = linhas.begin() ; it != linhas.end(); ++it){
                this->total_unique_blocks += it->nrBlocks;
            }

            cout << "Total blocks:\t" << this->total_blocks << endl;
            cout << "Unique blocks:\t" << this->total_unique_blocks << endl << endl;

            /* Calcular o intervalo esperado de compressão entre blocos*/
            cout << "Estimating interval of compression between two blocks" << endl;
            compressions_inter_blocks_interval = get_media_inter(percentage);
            cout << "Estimated Compression Interval: [" << get<0>(compressions_inter_blocks_interval) 
                << ", " << get<1>(compressions_inter_blocks_interval) << "]" << endl << endl;

            /* Check if wanted media belongs to inverval */
            double infLimit = get<0>(this->compressions_inter_blocks_interval);
            double supLimit = get<1>(this->compressions_inter_blocks_interval);
            if(wanted_media_compression_inter_blocks < infLimit){       /* Wanted media not in interval */
                wanted_media_compression_inter_blocks = infLimit;
            }
            else if(wanted_media_compression_inter_blocks > supLimit){  /* Wanted media not in interval */
                wanted_media_compression_inter_blocks = supLimit;
            }

            /* Calculating different_ratio used to change model blocks */
            int different_ratio = (wanted_media_compression_inter_blocks - infLimit) / (supLimit - infLimit); /* ratio of different non-compression areas needed to achive wanted media */
            /* Generating Block Models */
            int returnLoadModels = loadModels(different_ratio);

            /* TODO: Por fim, associar modelos aos blocos., alterar geração */
    
            return(returnLoadModels);
        }
        
};

int main(int argc, char ** argv){
    
    const int blockSize = 4096, blocosAGerar = 416830;
    const int wanted_media = 50, percentage_unique_blocks_analyze = 5;
    Generator generator;
    string pathToWrite = "/home/alexandre/Desktop/gerado/data", pathToRead = "./input.txt";

    if (generator.initialize(pathToRead, blockSize, blocosAGerar, 
        percentage_unique_blocks_analyze, wanted_media) == 1) {
        unsigned char *buffer = new unsigned char[blockSize]();
        FILE *write_ptr = fopen(pathToWrite.c_str(),"wb+");

        if(write_ptr == NULL) {cout << "Cannot open file to write." << endl; return -1;}
 
        cout << "Start generating data..." << endl;

        for(int i = 0; i < blocosAGerar; i++){
            generator.nextBlock(buffer);
            fwrite(buffer,1,blockSize,write_ptr);
        }

        cout << "Data generated with success." << endl;  
        }
    else {
        cout << "Error" << endl;
        return -1;
    }

    return 1;
}
