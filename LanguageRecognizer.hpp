#ifndef LANGUAGERECOGNIZER_HPP_INCLUDED
#define LANGUAGERECOGNIZER_HPP_INCLUDED

//#include <include/PlayThread.h>
#include <sndfile.h>
#include <cstdlib>
#include <fftw3.h>
#include <climits>
#include <cmath>
#include <cstring>
#include "RecogNeuralNet.hpp"

double sqr(double x){
    return x*x;
}

enum RunState {STOPPED, PLAYING, RECORDING};

class LanguageRecognizer{

    /*private:

        wxThread *RecordThread;

        PlayThread *Player;

        RunState State;*/

    public:

        NeuralNet *Net;

        LanguageRecognizer(char *NetName) {
            Net = new NeuralNet(NetName);
        }

        ~LanguageRecognizer() {
            delete Net;
        }

        int Record() {
            return system("arecord -f S16_LE -v -r 44100 -c 1 last.wav"); //-d 7
        }

        int Play() {
            return system("aplay -f S16_LE -v -r 44100 -c 1 last.wav");
        }

        int Analyze(char *FileName) {
            SF_INFO FileInfo;
            SNDFILE *Input;
            memset( &FileInfo, 0, sizeof(FileInfo));
            Input = sf_open( FileName, SFM_READ, &FileInfo);
            if( (FileInfo.format & SF_FORMAT_SUBMASK) != SF_FORMAT_PCM_16 )
                return -1;

            short *Data = (short*) malloc(FileInfo.frames * FileInfo.channels * sizeof(short));
            sf_readf_short( Input, Data, FileInfo.frames);

            int WindowLength = 2048;
            double *ToProcess = (double *) malloc(WindowLength * sizeof(double));
            fftw_complex *Frequencies = (fftw_complex *) calloc(WindowLength, sizeof(fftw_complex));
            fftw_plan Fft = fftw_plan_dft_r2c_1d( WindowLength, ToProcess, Frequencies, FFTW_MEASURE);

            FILE *out = fopen("out", "w");
            int BaseFrequency = 65;
            int MelBucketsNum = 64;
            int *MelTable = (int *) malloc(WindowLength * sizeof(int));
            for(int i = 0; i < WindowLength; i++)
                MelTable[i] = (int) ( 1127.0 * log(1.0 + i * FileInfo.samplerate / WindowLength / 700.0) );
            double *MelBuckets = (double *) malloc(MelBucketsNum * sizeof(double));
            double *MelCepstrum = (double *) malloc(MelBucketsNum * sizeof(double));
            double *MelAmplitudes = (double *) malloc(MelBucketsNum / 2 * sizeof(double));
            fftw_plan MelFft = fftw_plan_r2r_1d(MelBucketsNum, MelBuckets, MelCepstrum, FFTW_REDFT10, FFTW_MEASURE);

            // 0-130 65-195 ...
            //  0      1
            FILE *NetResult = fopen("netres", "w");

            for(int StartFrame = WindowLength / 2; StartFrame < FileInfo.frames - WindowLength / 2; StartFrame += WindowLength / 2){
                for(int i = 0; i < WindowLength; i ++)
                    ToProcess[i] = (double) Data[StartFrame - WindowLength / 2 + i] / (SHRT_MAX + 1.0);
                fftw_execute(Fft);
                for(int i = 0; i < MelBucketsNum; i ++)
                    MelBuckets[i] = 0;
                for(int i = 0; i < WindowLength; i++){
                    int MelFrequency = MelTable[i];
                    int MelBucketIndex = MelFrequency / BaseFrequency;

                    int Distance = abs(MelFrequency - MelBucketIndex * BaseFrequency);
                    double Coefficient = 1.0 - Distance / BaseFrequency;
                    double Power = sqr(Frequencies[i][0]) + sqr(Frequencies[i][1]);
                    Power /= WindowLength * 1.0;
                    double MelPower = Coefficient * Power;
                    MelBuckets[MelBucketIndex] += MelPower;

                    MelBucketIndex ++;
                    if (MelBucketIndex >= MelBucketsNum)
                        continue;

                    Distance = abs(MelFrequency - MelBucketIndex * BaseFrequency);
                    Coefficient = 1.0 - Distance / BaseFrequency;
                    Power = sqr(Frequencies[i][0]) + sqr(Frequencies[i][1]);
                    Power /= WindowLength * 1.0;
                    MelPower = Coefficient * Power;
                    MelBuckets[MelBucketIndex] += MelPower;
                }
                for(int i = 0; i < MelBucketsNum; i ++)
                    MelBuckets[i] = log(MelBuckets[i] + 1e-12);
                fftw_execute(MelFft);

                int MelAmplsToUse = 15;
                for(int i = 0; i < MelBucketsNum / 2; i ++)
                    MelAmplitudes[i] = fabs(MelCepstrum[i]) / sqrt(2 * MelBucketsNum);
                for(int i = 1; (i < MelBucketsNum / 2) && (i <= MelAmplsToUse); i ++)
                    fprintf(out, "%d %d %4.4lf\n", StartFrame / (WindowLength / 2), i, MelAmplitudes[i]);
                fprintf(out, "\n");

                int NetInNum, NetHidNum, NetOutNum;
                Net->LoadFromFile();
                Net->GetParams(NetInNum, NetHidNum, NetOutNum);
                double *NetIn = new double[MelAmplsToUse];
                double *NetOut = new double[NetOutNum];
                for(int i = 0; i < MelAmplsToUse; i ++)
                    NetIn[i] = MelAmplitudes[i + 1];
                Net->ComputeResult(NetIn, NetOut);
                fprintf(NetResult, "%d ", StartFrame / (WindowLength / 2));
                for(int i = 0; i < NetOutNum; i ++)
                    fprintf(NetResult, "%lf ", NetOut[i]);
                fprintf(NetResult, "\n");
                delete [] NetIn;
                delete [] NetOut;
            }
            fflush(NetResult);
            fclose(NetResult);
            fclose(out);
            sf_close(Input);
            fftw_destroy_plan(Fft);
            fftw_destroy_plan(MelFft);
            free(Data);
            free(ToProcess);
            free(MelTable);
            free(Frequencies);
            free(MelBuckets);
            free(MelCepstrum);
            free(MelAmplitudes);
            return 0;
        }

        int RunningState() { }

        int Stop() { }
};



#endif // LANGUAGERECOGNIZER_HPP_INCLUDED
