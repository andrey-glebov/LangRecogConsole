#include <iostream>
#include "LanguageRecognizer.hpp"

using namespace std;

LanguageRecognizer *Recognize = new LanguageRecognizer();

int main()
{
    char c = 'n';
    char file[]="last.wav";

    cout << "LangRecogConsole\n";
    while(1){
        cout << "> ";
        cin >> c;
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
        }
    }
    return 0;
}
