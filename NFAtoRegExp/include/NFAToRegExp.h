#ifndef NFATOREGEXP_H
#define NFATOREGEXP_H

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <algorithm>

using namespace std;

class NFAToRegExp
{
    public:
        NFAToRegExp(string &automataPath);
        virtual ~NFAToRegExp();
        void generateRegularExpression();
        void printAutomata(string outputPath);
        string getRegExp();

    protected:

    private:
        void addInitialState();
        void addFinalState();
        void deleteEpsilonTransitions();
        void adjustRepeatedHeights();
        void removeState(int state);
        void checkAndRemoveBidirectional(int stateToRemove);
        void modifyInitStateHeight(string weight, int newDestiny, int stateToRemove, bool isKleene);
        vector< pair<int, vector<pair<string, int> > > > automata;
        vector< pair<int, vector<pair<string, int> > > > initialAutomata;
        int finalState;
        int initialState;
        string regularExpression;

};

#endif // NFATOREGEXP_H
