/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   arrayVersion.cpp
 * Author: alex
 *
 * Created on 8 de Maio de 2018, 22:36
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <vector>
#include <random>
#include <math.h> 

using namespace std;

const int nrBlocksToGenerate = 100000;

class Linha{
    public:
        int nrCopies, nrBlocks, nrBase;
};

class GenerateBlocks {
private:
    vector<int> idVector;
    int totalBlocks;
    Linha linhaZero;
    
    int getRandomPositionVetor()
    {
        /* Generate random position in vetor plus blocks with 0 copies */
        random_device rd;  //Will be used to obtain a seed for the random number engine
        mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
        uniform_int_distribution<> dis(0, (linhaZero.nrBlocks + idVector.size() - 1));
        
        int randomPositionVetor = dis(gen);
        /* Outside Vector : 0 copie */
       
        if(randomPositionVetor >= idVector.size()){
            int totalyRandomID = rand();
            return(totalyRandomID);
        }
        else{
            return idVector.at(randomPositionVetor);
        }
    }

public:

    void readFile(string filepath) {
        int nrBaseAux = 0, aux, idBlock;
        istringstream iss;
        string line;

        ifstream fin(filepath);
        if (!fin) {
            cout << "Can't open file " << filepath << endl;
            exit(-1);
        }

        /* Reading line by line */
        while (getline(fin, line)) {
            Linha newLine;
            newLine.nrBase = nrBaseAux;
            stringstream ss(line);
            string token;
            double probabilidadeLinha;
            vector<int>::iterator it;
            it = this->idVector.begin();

            /* Criar o objeto Linha*/
            if (getline(ss, token, ' ')) {

                newLine.nrCopies = stoi(token);

                if (getline(ss, token, ' ')) {

                    probabilidadeLinha = stof(token);
                    
                    newLine.nrBlocks = (int) ceil((this->totalBlocks * (probabilidadeLinha / 100) / (newLine.nrCopies + 1)));
                   
                    nrBaseAux = nrBaseAux + newLine.nrBlocks;
                }
            }
            /* (compressoes), vamos ignorar nesta versao*/
            while (getline(ss, token, ' ')) {
            }

            /* Construir vetor*/
            /* Caso seja 0 nao precisamos de inserir*/
            if(newLine.nrCopies == 0){
                this->linhaZero = newLine;
            }
            
            /* Vamos inserir no vetor*/
            else{
                for (aux = 1; aux <= newLine.nrBlocks; aux++) {
                    idBlock = aux + newLine.nrBase;
                    it = idVector.insert(it, newLine.nrCopies + 1, idBlock);
                }
            }
        }
    }

    GenerateBlocks(int nrBlocks) {
        this->totalBlocks = nrBlocks;
    }

    uint64_t nextBlock() {
      
        /* Pegar id aleatorio */
        uint64_t result = getRandomPositionVetor();
        return result;
    }

    vector<int> getIdVector() const {
        return idVector;
    }

};

static void gerar(GenerateBlocks g, string path){
    ofstream outfile (path,std::ofstream::binary);
    uint64_t resultado;

    for(int i = 0; i < nrBlocksToGenerate; i++){
            resultado = g.nextBlock();
            outfile.write(reinterpret_cast<const char *>(&resultado), sizeof(resultado));
    }

    outfile.close();
}

/*
int main(int argc, char** argv) {
    
    string pathToWrite = "/home/alex/repos/lei/analyzer/data/data";
    string pathToRead = "./example.txt";
    
    GenerateBlocks newGeneration(nrBlocksToGenerate);
    
    newGeneration.readFile(pathToRead);
    
    gerar(newGeneration, pathToWrite);

    return 0;
}
//*/

