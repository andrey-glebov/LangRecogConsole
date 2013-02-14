#ifndef RECOGNEURALNET_HPP_INCLUDED
#define RECOGNEURALNET_HPP_INCLUDED

#include <cstdlib>
#include <ctime>
#include <string>

using namespace std;

class NeuralNet{

    private:

        FILE *Net;
        int InNumber, HiddenNumber, OutNumber;
        double *InHiddenCoefs;
        double *HiddenOutCoefs;
        double *HiddenLayer;
        string FileName;
        double sigma(double arg){
            return 1.0 / ( 1.0 + exp(-arg) );
        }
        double frand(double delta){
            return delta * (1.0 - 2.0 * rand() / (double) RAND_MAX);
        }

    public:

	    NeuralNet(char *File) {
	        FileName = File;
	    }

	    void GetParams(int &InNum, int &HiddenNum, int &OutNum){
	        InNum = InNumber;
	        HiddenNum = HiddenNumber;
	        OutNum = OutNumber;
	    }

        void Reset(int InNum, int HiddenNum, int OutNum) {
            InNumber = InNum;
            HiddenNumber = HiddenNum;
            OutNumber = OutNum;
            if (InHiddenCoefs != NULL)
                delete [] InHiddenCoefs;
            if (HiddenOutCoefs != NULL)
                delete [] HiddenOutCoefs;
            InHiddenCoefs = new double[InNum * HiddenNum];
            HiddenOutCoefs = new double[HiddenNum * OutNum];
            srand(time(0) ^ clock());
            for(int i = 0; i < InNum; i ++)
                for(int j = 0; j < HiddenNum; j ++)
                    InHiddenCoefs[i * HiddenNum + j] = frand(0.1);
            for(int i = 0; i < HiddenNum; i ++)
                for(int j = 0; j < OutNum; j ++)
                    HiddenOutCoefs[i * OutNum + j] = frand(0.1);
        }

        void LoadFromFile() {
            Net = fopen(FileName.c_str() , "r");
            fscanf(Net, "%d%d%d", &InNumber, &HiddenNumber, &OutNumber);
            if (InHiddenCoefs != NULL)
                delete [] InHiddenCoefs;
            if (HiddenOutCoefs != NULL)
                delete [] HiddenOutCoefs;
            InHiddenCoefs = new double[InNumber * HiddenNumber];
            HiddenOutCoefs = new double[HiddenNumber * OutNumber];
            for(int i = 0; i < InNumber; i ++)
                for(int j = 0; j < HiddenNumber; j ++)
                    fscanf(Net, "%lf", &InHiddenCoefs[i * HiddenNumber + j]);
            for(int i = 0; i < HiddenNumber; i ++)
                for(int j = 0; j < OutNumber; j ++)
                    fscanf(Net, "%lf", &HiddenOutCoefs[i * OutNumber + j]);
            fclose(Net);
        }

        void SaveToFile() {
            Net = fopen(FileName.c_str() , "w");
            fprintf(Net, "%d %d %d\n", InNumber, HiddenNumber, OutNumber);
            for(int i = 0; i < InNumber; i ++){
                for(int j = 0; j < HiddenNumber; j ++)
                    fprintf(Net, "%lf ", InHiddenCoefs[i * HiddenNumber + j]);
                fprintf(Net, "\n");
            }
            for(int i = 0; i < HiddenNumber; i ++){
                for(int j = 0; j < OutNumber; j ++)
                    fprintf(Net, "%lf ", HiddenOutCoefs[i * OutNumber + j]);
                fprintf(Net, "\n");
            }
            fflush(Net);
            fclose(Net);
        }

        void ComputeResult(double *InLayer, double *OutLayer){
            HiddenLayer = new double[HiddenNumber];
            for(int HidIdx = 0; HidIdx < HiddenNumber; HidIdx ++){
                HiddenLayer[HidIdx] = InHiddenCoefs[HidIdx];
                for(int InIdx = 1; InIdx < InNumber; InIdx ++)
                    HiddenLayer[HidIdx] += InLayer[InIdx - 1] * InHiddenCoefs[InIdx * HiddenNumber + HidIdx];
                HiddenLayer[HidIdx] = sigma(HiddenLayer[HidIdx]);
            }

            for(int OutIdx = 0; OutIdx < OutNumber; OutIdx ++){
                OutLayer[OutIdx] = HiddenOutCoefs[OutIdx];
                for(int HidIdx = 1; HidIdx < HiddenNumber; HidIdx ++)
                    OutLayer[OutIdx] += HiddenLayer[HidIdx - 1] * HiddenOutCoefs[HidIdx * OutNumber + OutIdx];
                OutLayer[OutIdx] = sigma(OutLayer[OutIdx]);
            }
        }

        void Learn(double *Ins, double *Outs) { }

};

#endif // RECOGNEURALNET_HPP_INCLUDED
