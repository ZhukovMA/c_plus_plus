#include <iostream>
#include <string>
#include <vector>
#include <random>

using namespace std;

const string TARGET = "methinksitislikeaweasel";
const int STRING_LENGTH = 23;
const int OFFSPRING_COUNT = 100;
const double MUTATION_PROBABILITY = 0.05;

string generateInitialString(default_random_engine& engine) {
    uniform_int_distribution<int> letterDist(0, 25);

    string s;
    s.reserve(STRING_LENGTH);

    for (int i = 0; i < STRING_LENGTH; ++i) {
        s += static_cast<char>('a' + letterDist(engine));
    }

    return s;
}

string mutateString(const string& parent, default_random_engine& engine) {
    uniform_real_distribution<double> realDist(0.0, 1.0);

    string child = parent;

    for (char& c : child) {
        double p = realDist(engine);

        if (p < MUTATION_PROBABILITY) {
            int letterIndex = static_cast<int>(realDist(engine) * 26.0);
            if (letterIndex == 26) {
                letterIndex = 25;
            }
            c = static_cast<char>('a' + letterIndex);
        }
    }

    return child;
}

int distanceToTarget(const string& s) {
    int distance = 0;

    for (size_t i = 0; i < TARGET.size(); ++i) {
        if (s[i] != TARGET[i]) {
            ++distance;
        }
    }

    return distance;
}

int main() {
    random_device rd;
    default_random_engine engine(rd());

    string parent = generateInitialString(engine);
    int generation = 0;

    cout << "Target: " << TARGET << '\n';
    cout << "Generation " << generation << ", parent: " << parent << ", metric: " << distanceToTarget(parent) << "\n\n";

    while (true) {
        ++generation;

        vector<string> offspring;
        offspring.reserve(OFFSPRING_COUNT);

        int bestMetric = STRING_LENGTH + 1;
        string bestString;

        cout << "Generation " << generation << ":\n";

        for (int i = 0; i < OFFSPRING_COUNT; ++i) {
            string child = mutateString(parent, engine);
            int metric = distanceToTarget(child);

            offspring.push_back(child);

            cout << i + 1 << ") " << child << " metric = " << metric << '\n';

            if (metric < bestMetric) {
                bestMetric = metric;
                bestString = child;
            }

            if (metric == 0) {
                cout << "\nTarget reached on generation " << generation << ":\n";
                cout << child << '\n';
                return 0;
            }
        }

        parent = bestString;

        cout << "Chosen parent for next generation: " << parent << " metric = " << bestMetric << "\n\n";
    }

    return 0;
}