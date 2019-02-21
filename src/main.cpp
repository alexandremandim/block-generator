#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <vector>
#include <random>
#include "../libs/lzdatagen-master/lzdatagen.h"
#include "../libs/lzdatagen-master/pcg_basic.h"

const int tamanhoBloco = 4096; /* bytes */
const int blocosAGerar = 2400000;

using namespace std;

class Linha{
    public:
        int nrCopies, nrBlocks, nrBase;
        vector<double> compression; 
};

class Generator {
	private:

	    vector<Linha> linhas;
	    vector<double> weights;

	    /* Acho que posso apagar esta funcao */
	    int getCompression(Linha linhaAleatoria) {
	        unsigned seedDistr = std::chrono::system_clock::now().time_since_epoch().count();
	        default_random_engine generator(seedDistr);
	        discrete_distribution<> distributionCompression(linhaAleatoria.compression.begin(), linhaAleatoria.compression.end());
	        return distributionCompression(generator)*10;
	    }

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

	    Linha getLinha() {

	        unsigned seedDistr = std::chrono::system_clock::now().time_since_epoch().count();
	        std::default_random_engine generator(seedDistr);

	        discrete_distribution<> distribution(weights.begin(), weights.end());

	        return linhas.at(distribution(generator));
	    }

	    double giveMyCompression(Linha linhaAleatoria, int randomBlockID) {
	        
	        double res = 1.0;
	        
	        /* Como o ID é completamente aleatório, também a compressão vai ser com base nas percentagens */
	        if(linhaAleatoria.nrCopies == 0){
	            /* Get random compression */
	            unsigned seedDistr = std::chrono::system_clock::now().time_since_epoch().count();
	            std::default_random_engine generator(seedDistr);
	            discrete_distribution<> distribution(linhaAleatoria.compression.begin(), linhaAleatoria.compression.end());
	        
	           int compressionPosition = distribution(generator);
	           double compressionPercentage = (compressionPosition + 0) * 10;
	           res = (((double)1)/((((double)100.0)-compressionPercentage)/((double)100)));
	           return res;
	        }
	        
	        /* Cada bloco tem uma compressão ligada a ele */
	        else{
	            double myPercent = ((double) randomBlockID) / linhaAleatoria.nrBlocks;
	            double aux = 0;
	            double compression, res = 1.0;

	            for (int i = 0; i < 10; i++) {
	                aux += linhaAleatoria.compression.at(i) / 100.0;
	                if (myPercent <= aux) {
	                    compression = (i + 1) * 10;
	                    res = (((double)1)/((((double)100.0)-compression)/((double)100)));
	                    break;
	                }
	            }
	            return res;
	        }
	    }
	 
	public:

	    void nextBlock(char* buffer) {

	        Linha linhaAleatoria = getLinha();
	        double myCompression = 1.0;

	        int randomBlockID = getRandomBlockFromLine(linhaAleatoria);
	        myCompression = giveMyCompression(linhaAleatoria, randomBlockID);
	        uint64_t seedLZ = randomBlockID + linhaAleatoria.nrBase;

	        pcg32_srandom(seedLZ, 0xC0FFEE);
	        lzdg_generate_data(buffer, tamanhoBloco, myCompression, 3.0, 3.0);
	    }

	    void readFile(string path) {

	        int nrBaseAux = 0;
	        istringstream iss;
	        string line;

	        ifstream fin(path);
	        if (!fin) {
	            cout << "Can't open file " << path << endl;
	            exit(-1);
	        }

	        cout << "Reading file " << path << endl;

	        /* Reading line by line */
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

	                    newLine.nrBlocks = (int) (blocosAGerar * (probabilidadeLinha / 100) / (newLine.nrCopies + 1));

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

	        cout << "File readed with success" << endl;
	    }
};

static void gerar(Generator data, string path){
    ofstream outfile (path,std::ofstream::binary);
    uint64_t resultado;
    char* buffer = new char[tamanhoBloco];

    cout << "Generating " << blocosAGerar << " blocks with " << tamanhoBloco << " to " << path << endl;
            
    for(int i = 0; i < blocosAGerar; i++){
            
            data.nextBlock(buffer);
            outfile.write(buffer, tamanhoBloco);
    }
    
    outfile.close();
    
    cout << "Generated with success" << endl;
}

//*
int main(int argc, const char **argv) {
    
    Generator generator;
    string pathToWrite = "/Users/alexandresilva/Desktop/data";
    string pathToRead = "/Users/alexandresilva/Desktop/ex.txt";

    generator.readFile(pathToRead);
    gerar(generator, pathToWrite);

    return 1;
}
//*/
