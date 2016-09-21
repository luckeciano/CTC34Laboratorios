#include "NFAToRegExp.h"


//Construtor: Lê o arquivo e gera a estrutura correspondente ao autômato
//Adicionalmente, já adiciona estados final e inicial
NFAToRegExp::NFAToRegExp(string &automataPath)
{
    ifstream myFile;
    myFile.open(automataPath.c_str());
    if (myFile.is_open()) {
        int state;
        myFile >> state;
        this->initialState = state;
        myFile >> state;
        this->finalState = state;
        addInitialState();
        while (!myFile.eof()) {
            int origin; int destiny; string weight;
            myFile >> origin >> destiny >> weight;
            bool insideAutomata = false;
            for (int k = 0; k < automata.size() && !insideAutomata; k++) {
                if (automata[k].first == origin) {
                        bool repeatedHeight = false;
                        for (int t = 0; t < automata[k].second.size() && !repeatedHeight; t++) {
                            if (automata[k].second[t].second == destiny) {
                                automata[k].second[t].first = "(" + automata[k].second[t].first + "+" + weight + ")";
                                repeatedHeight = true;
                            }
                        }
                        if (!repeatedHeight)
                            automata[k].second.push_back(make_pair(weight, destiny));
                    insideAutomata = true;
                }
            }

            if (!insideAutomata) {
                vector <pair <string, int> > aux; aux.push_back(make_pair(weight, destiny));
                automata.push_back(make_pair(origin,aux));
            }
        }

        addFinalState();
        initialAutomata = automata;
    } else cout << "Unable to open " << automataPath << endl;
}

//Adiciona estado inicial ao autômato, criando uma transição épsilon entre este
//e o antigo estado inicial
void NFAToRegExp::addInitialState() {
    vector <pair <string, int> > aux;
    aux.push_back(make_pair("&", this->initialState));
    automata.push_back(make_pair(-1, aux));
    this->initialState = -1;

}

//Adiciona o estado final ao autômato, criando uma transição épsilon do estado final
//anterior até este novo
void NFAToRegExp::addFinalState() {
    bool existsInAutomata = false;
    for (int i = 0; i < automata.size() && !existsInAutomata; i++) {
        if (automata[i].first == this->finalState) {
            existsInAutomata = true;
        }
    }
    if (!existsInAutomata) {
        vector <pair <string, int> > aux;
        automata.push_back(make_pair(this->finalState, aux));
    }
    vector <pair<string, int> > aux;
    automata.push_back(make_pair(-2, aux));
    for (int i = 0; i < automata.size(); i++) {
        if (automata[i].first == this->finalState) {
            automata[i].second.push_back(make_pair("&", -2));
        }
    }
    this->finalState = -2;

}

//Imprime o autômato antes e depois da remoção de estados
//imprime também a expressão regular
void NFAToRegExp::printAutomata(string outputPath) {
    ofstream output(outputPath);
    output << "Initial Automata:" << endl;
    for (int i = 0; i < initialAutomata.size(); i++) {
        output << "State " << initialAutomata[i].first << ":" << endl;
        for (int k = 0; k < initialAutomata[i].second.size(); k++) {
            output << initialAutomata[i].second[k].second << " " << initialAutomata[i].second[k].first << endl;
        }
    }
    output << endl;
    output << "Final Automata:" << endl;
    for (int i = 0; i < automata.size(); i++) {
        output << "State " << automata[i].first << ":" << endl;
        for (int k = 0; k < automata[i].second.size(); k++) {
            output << automata[i].second[k].second << " " << automata[i].second[k].first << endl;
        }
    }

    output << endl;
    output << "Regular Expression: " << regularExpression << endl;

}
void NFAToRegExp::adjustRepeatedHeights() {
    vector<pair<string, int> >::iterator it1;
    vector<pair<string, int> >::iterator it2;
    vector<pair<string, int> >::iterator aux;
    for (it1 = automata[0].second.begin(); it1 != automata[0].second.end(); it1++) {
        for (it2 = it1 + 1; it2!=automata[0].second.end(); it2++) {
            if (it1->second == it2->second){
                it1->first = "(" + it1->first + "+" + it2->first + ")";
                aux = it2 + 1;
                if (aux == automata[0].second.end()) {
                    automata[0].second.erase(it2);
                    break;
                }
                else {
                    automata[0].second.erase(it2);
                    it2 = aux - 1;
                }
            }
        }
    }

}

//Responsável por criar fechamentos e novas transições para remover estados
void NFAToRegExp::modifyInitStateHeight(string weight, int newDestiny, int stateToRemove, bool isKleene){


    if (isKleene) {
        for (int i = 0; i < automata[0].second.size(); i++) {
            if (automata[0].second[i].second == stateToRemove){
                        automata[0].second[i].first += weight + "*";

            }
        }
    }
    else {
        string weightFromStateToRemove;
        vector<pair<string, int> >::iterator it;
        for (it = automata[0].second.begin(); it != automata[0].second.end(); it++) {
            if (it->second == stateToRemove) {
                weightFromStateToRemove = it->first;
                break;
            }
        }

        bool isRepeatedHeight = false;
        for (int i = 0; i < automata[0].second.size() && !isRepeatedHeight; i++) {
            if (automata[0].second[i].second == newDestiny) {
                automata[0].second[i].first = "(" + automata[0].second[i].first + "+" + weightFromStateToRemove + weight + ")";
                isRepeatedHeight = true;
            }
        }
        if (!isRepeatedHeight) automata[0].second.push_back(make_pair(weightFromStateToRemove+weight, newDestiny));
        for (it = automata[0].second.begin(); it != automata[0].second.end(); it++){
            if (it->second == stateToRemove){
                automata[0].second.erase(it);
                break;
            }
        }
    }
}
//Método que gerencia a atividade de remoção de estado
void NFAToRegExp::removeState(int stateToRemove) {
    checkAndRemoveBidirectional(stateToRemove);
    vector< pair<int, vector<pair<string, int> > > >::iterator it;
    for (it = automata.begin(); it!=automata.end(); it++) {
        if (it->first == stateToRemove) {

            vector<pair<string, int> >::iterator itHeights;
            //check klenee
            for (itHeights = it->second.begin(); itHeights != it->second.end(); itHeights++) {
               if (itHeights->second == stateToRemove) {
                    modifyInitStateHeight(itHeights->first, itHeights->second, stateToRemove, true);
               }
            }
            //no klenee
            for (itHeights = it->second.begin(); itHeights != it->second.end(); itHeights++) {
               if (itHeights->second != stateToRemove)
                    modifyInitStateHeight(itHeights->first, itHeights->second, stateToRemove, false);
            }
            automata.erase(it);
        }
    }
}

//Método que checa e remove bidirecionalidade no autômato
void NFAToRegExp::checkAndRemoveBidirectional(int stateToRemove) {
    vector< pair<int, vector<pair<string, int> > > >::iterator itStateToRemove;
    vector< pair<int, vector<pair<string, int> > > >::iterator itDestinyBidirectional;
    for (itStateToRemove = automata.begin(); itStateToRemove != automata.end(); itStateToRemove++) {
        if (itStateToRemove->first == stateToRemove) {
            for (itDestinyBidirectional = automata.begin(); itDestinyBidirectional != automata.end(); itDestinyBidirectional++){
                if (itDestinyBidirectional != itStateToRemove) {
                    for (int i = 0; i < itStateToRemove->second.size(); i++) {
                        if (itStateToRemove->second[i].second == itDestinyBidirectional->first){
                            for (int j = 0; j < itDestinyBidirectional->second.size(); j++) {
                                if (itDestinyBidirectional->second[j].second == itStateToRemove->first) {
                                    //yes, its bidirectional (finally!)
                                    string weightNewDestiny = "(" + itStateToRemove->second[i].first +
                                            itDestinyBidirectional->second[j].first+ ")";
                                    for (int k = 0; k < itDestinyBidirectional->second.size(); k++) {
                                            if (itDestinyBidirectional->second[k].second == stateToRemove) {
                                                itStateToRemove->second.push_back(make_pair(weightNewDestiny,
                                                                                    itDestinyBidirectional->second[k].second));
                                            }
                                            else itStateToRemove->second.push_back(make_pair(itDestinyBidirectional->second[k].first,
                                                                                             itDestinyBidirectional->second[k].second));
                                    }
                                    itStateToRemove->second.erase(itStateToRemove->second.begin() + i);
                                    automata.erase(itDestinyBidirectional);
                                }
                            }

                        }
                    }
                }
            }
            break;
        }

    }

}
//Retira as transições epsilons para resultar na expressão regular
void NFAToRegExp::deleteEpsilonTransitions() {
    for (int i = 0; i < regularExpression.size(); i++) {
        regularExpression.erase (std::remove(regularExpression.begin(), regularExpression.end(), '&'), regularExpression.end());
    }
}

//Gera expressão regular, ajustanto as transições em paralelo e removendo estado por estado
void NFAToRegExp::generateRegularExpression() {
    regularExpression = "";
    while (true) {
        adjustRepeatedHeights();
        if (automata[0].second.size() == 1 && automata[0].second[0].second == this->finalState)
            break;
        else {
            removeState(automata[0].second[0].second);
        }
    }
    regularExpression = automata[0].second[0].first;
    deleteEpsilonTransitions();
    cout << regularExpression << endl;
}
NFAToRegExp::~NFAToRegExp()
{
    //dtor
}
//Main com casos de teste
int main() {
    string path = "automata.txt";
    string outputPath = "automataOutput.txt";
    NFAToRegExp nfaToRegExp(path);
    nfaToRegExp.generateRegularExpression();
    nfaToRegExp.printAutomata(outputPath);

    path = "automataTest1.txt";
    outputPath = "automataOutputTest1.txt";
    NFAToRegExp testOne(path);
    testOne.generateRegularExpression();
    testOne.printAutomata(outputPath);

    path = "automataTest2.txt";
    outputPath = "automataOutputTest2.txt";
    NFAToRegExp testTwo(path);
    testTwo.generateRegularExpression();
    testTwo.printAutomata(outputPath);

    path = "automataTest3.txt";
    outputPath = "automataOutputTest3.txt";
    NFAToRegExp testThree(path);
    testThree.generateRegularExpression();
    testThree.printAutomata(outputPath);

    path = "automataTest4.txt";
    outputPath = "automataOutputTest4.txt";
    NFAToRegExp testFour(path);
    testFour.generateRegularExpression();
    testFour.printAutomata(outputPath);

    path = "automataTestFinal.txt";
    outputPath = "automataOutputTestFinal.txt";
    NFAToRegExp testFinal(path);
    testFinal.generateRegularExpression();
    testFinal.printAutomata(outputPath);








    return 0;
}
