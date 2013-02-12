#ifndef LANGUAGERECOGNIZER_HPP_INCLUDED
#define LANGUAGERECOGNIZER_HPP_INCLUDED

//#include <include/PlayThread.h>
#include <sndfile.h>
#include <cstdlib>
#include <fftw3.h>
#include <climits>
#include <cmath>
#include <cstring>
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

        LanguageRecognizer() { }

        ~LanguageRecognizer() { }

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

            short *Data = (short*) calloc(FileInfo.frames * FileInfo.channels, sizeof(short));
            sf_readf_short( Input, Data, FileInfo.frames);

            int WindowLength = 2048;
            double *ToProcess = (double *) calloc(WindowLength, sizeof(double));
            fftw_complex *Frequencies = (fftw_complex *) calloc(WindowLength, sizeof(fftw_complex));
            fftw_plan Fft = fftw_plan_dft_r2c_1d( WindowLength, ToProcess, Frequencies, FFTW_MEASURE);

            FILE *out = fopen("out", "w");
            int BaseFrequency = 65;
            int MelBucketsNum = 64;
            int *MelTable = (int *) calloc(WindowLength, sizeof(int));
            for(int i = 0; i < WindowLength; i++)
                MelTable[i] = (int) ( 1127.0 * log(1.0 + i / 700.0) );
            double *MelBuckets = (double *) calloc(MelBucketsNum, sizeof(double));
            double *MelCepstrum = (double *) calloc(MelBucketsNum, sizeof(double));
            double *MelAmplitudes = (double *) calloc(MelBucketsNum / 2, sizeof(double));
            fftw_plan MelFft = fftw_plan_r2r_1d(MelBucketsNum, MelBuckets, MelCepstrum, FFTW_REDFT10, FFTW_MEASURE);

            // 0-130 65-195 ...
            //  0      1

            //for(int StartFrame = WindowLength / 2; StartFrame < FileInfo.frames - WindowLength / 2; StartFrame ++){
            int StartFrame = WindowLength / 2;
                //printf("SF: %d\n",StartFrame);
                for(int i = 0; i < WindowLength; i ++){
                    ToProcess[i] = (double) Data[StartFrame - WindowLength / 2 + i] / (SHRT_MAX + 1.0);
                    //printf("%d %lf\n", i, ToProcess[i]);
                }
                fftw_execute(Fft);
                for(int i = 0; i < MelBucketsNum; i ++)
                    MelBuckets[i] = 0;

                for(int i = 0; i < WindowLength; i++){
                    int MelFrequency = MelTable[i];
                    int MelBucketIndex = MelFrequency / BaseFrequency;

                    int Distance = abs(MelFrequency - MelBucketIndex * BaseFrequency);
                    double Koefficient = 1.0 - Distance / BaseFrequency;
                    double Power = sqr(Frequencies[i][0]) + sqr(Frequencies[i][1]);
                    double MelPower = Koefficient * Power;
                    MelBuckets[MelBucketIndex] += MelPower;

                    MelBucketIndex ++;
                    if (MelBucketIndex >= MelBucketsNum)
                        continue;

                    Distance = abs(MelFrequency - MelBucketIndex * BaseFrequency);
                    Koefficient = 1.0 - Distance / BaseFrequency;
                    Power = sqr(Frequencies[i][0]) + sqr(Frequencies[i][1]);
                    MelPower = Koefficient * Power;
                    MelBuckets[MelBucketIndex] += MelPower;
                }
                for(int i = 0; i < MelBucketsNum; i ++){
                    //fprintf(out, "%d %lf ", i, MelBuckets[i]);
                    MelBuckets[i] = log(MelBuckets[i] + 1e-12);
                    //fprintf(out, "%d %lf\n", i, MelBuckets[i]);
                }
                fftw_execute(MelFft);
                MelAmplitudes[0] = fabs(MelCepstrum[0]);
                MelAmplitudes[MelBucketsNum / 2 - 1] = fabs(MelCepstrum[MelBucketsNum - 1]);
                for(int i = 1; i < MelBucketsNum / 2 - 1; i ++)
                    MelAmplitudes[i] = sqrt(sqr(MelCepstrum[i]) + sqr(MelCepstrum[MelBucketsNum - i]));
                for(int i = 0; i < MelBucketsNum / 2; i++)
                    fprintf(out, "%d %lf\n", i, MelAmplitudes[i]);
            //}
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
            system("cat out");
            //system("gnuplot\nplot out\n");
            return 0;
        }

        int RunningState() { }

        int Stop() { }
};



#endif // LANGUAGERECOGNIZER_HPP_INCLUDED
