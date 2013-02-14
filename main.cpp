#include <cstdio>
#include "LanguageRecognizer.hpp"

char c = 'n';
char file[] = "last.wav";
char net[] = "net";
LanguageRecognizer *Recognize = new LanguageRecognizer(net);
int InNum, HiddenNum, OutNum;

int main()
{
    printf("LangRecogConsole\n");
    while(1){
        printf("> ");
        do
            c = getchar();
        while(isspace(c));
        switch(c){
            case 'r':
                Recognize->Record();
            break;
            case 'p':
                Recognize->Play();
            break;
            case 'a':
                Recognize->Analyze(file);
            break;
            case 't':
                printf("Input sizes:\n");
                scanf("%d%d%d", &InNum, &HiddenNum, &OutNum);
                Recognize->Net->Reset(InNum, HiddenNum, OutNum);
		        Recognize->Net->SaveToFile();
            break;
	        case 'l':
                Recognize->Net->LoadFromFile();
            break;
            case 's':
                Recognize->Net->SaveToFile();
            break;
        }
    }
    return 0;
}
