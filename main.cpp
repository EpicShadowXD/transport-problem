#include <iostream>
#include <vector>
#include <limits>
#include <queue>
#include <utility>
#include <algorithm>
#include <functional>
#include <chrono>
#include <numeric>

using namespace std;
using namespace chrono;

const int INF = numeric_limits<int>::max();

struct TransportProblem {
    vector<vector<int>> cost;
    vector<int> supply;
    vector<int> demand;
};

// Functia care printeaza matricea
void printMatrix(const vector<vector<int>>& matrix) {
    for (const auto& row : matrix) {
        for (const auto& elem : row) {
            if (elem == INF) cout << "INF ";
            else cout << elem << " ";
        }
        cout << endl;
    }
    cout << endl;
}

// Metoda coltului de Nord-Vest
vector<vector<int>> northWestCorner(TransportProblem& problem) {
    int m = problem.supply.size();
    int n = problem.demand.size();
    vector<vector<int>> allocation(m, vector<int>(n, 0));

    int i = 0, j = 0;
    while (i < m && j < n) {
        int minVal = min(problem.supply[i], problem.demand[j]);
        allocation[i][j] = minVal;
        problem.supply[i] -= minVal;
        problem.demand[j] -= minVal;

        if (problem.supply[i] == 0) ++i;
        if (problem.demand[j] == 0) ++j;
    }

    return allocation;
}

// Calculeaza costul total al unei solutii
int calculateTotalCost(const TransportProblem& problem, const vector<vector<int>>& allocation) {
    int totalCost = 0;
    for (auto i = 0ul; i < allocation.size(); ++i) {
        for (auto j = 0ul; j < allocation[i].size(); ++j) {
            totalCost += allocation[i][j] * problem.cost[i][j];
        }
    }
    return totalCost;
}

// Gasirea ciclului pentru ajustare
bool findCycle(int startI, int startJ, const vector<vector<int>>& allocation, vector<pair<int, int>>& cycle) {
    int m = allocation.size();
    int n = allocation[0].size();
    vector<vector<int>> visited(m, vector<int>(n, 0));
    vector<pair<int, int>> path;

    function<bool(int, int, int, int)> dfs = [&](int i, int j, int prevI, int prevJ) -> bool {
        if (visited[i][j] == 2) return false;
        if (visited[i][j] == 1) {
            // Ciclul este gasit
            for (auto& p : path) {
                cycle.push_back(p);
                if (p == make_pair(i, j)) break;
            }
            cycle.push_back({i, j});
            return true;
        }

        visited[i][j] = 1;
        path.push_back({i, j});

        // Mergem pe linii
        for (int k = 0; k < n; ++k) {
            if (k != j && allocation[i][k] > 0 && !(i == prevI && k == prevJ)) {
                if (dfs(i, k, i, j)) return true;
            }
        }
        // Mergem pe coloane
        for (int k = 0; k < m; ++k) {
            if (k != i && allocation[k][j] > 0 && !(k == prevI && j == prevJ)) {
                if (dfs(k, j, i, j)) return true;
            }
        }

        path.pop_back();
        visited[i][j] = 2;
        return false;
    };

    return dfs(startI, startJ, -1, -1);
}

// Metoda MODI (Modified Distribution Method)
void MODI(TransportProblem& problem, vector<vector<int>>& allocation) {
    int m = problem.supply.size();
    int n = problem.demand.size();
    vector<int> u(m, INF), v(n, INF);
    vector<vector<int>> delta(m, vector<int>(n));

    while (true) {
        // Calcularea potentialelor
        u[0] = 0;
        bool updated = true;
        while (updated) {
            updated = false;
            for (int i = 0; i < m; ++i) {
                for (int j = 0; j < n; ++j) {
                    if (allocation[i][j] != 0) {
                        if (u[i] != INF && v[j] == INF) {
                            v[j] = problem.cost[i][j] - u[i];
                            updated = true;
                        } else if (u[i] == INF && v[j] != INF) {
                            u[i] = problem.cost[i][j] - v[j];
                            updated = true;
                        }
                    }
                }
            }
        }

        // Calcularea costurilor relative (delta)
        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < n; ++j) {
                if (allocation[i][j] == 0) {
                    delta[i][j] = problem.cost[i][j] - u[i] - v[j];
                } else {
                    delta[i][j] = INF;
                }
            }
        }

        // Gasirea celei mai negative valori delta
        int minDelta = 0, minI = -1, minJ = -1;
        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < n; ++j) {
                if (delta[i][j] < minDelta) {
                    minDelta = delta[i][j];
                    minI = i;
                    minJ = j;
                }
            }
        }

        // Daca nu exista valori negative delta, solutia este optima
        if (minDelta >= 0) break;

        // Gasirea ciclului in matricea de alocare
        vector<pair<int, int>> cycle;
        findCycle(minI, minJ, allocation, cycle);

        // Ajustarea alocarii pe baza ciclului gasit
        int minValue = INF;
        for (auto k = 1ul; k < cycle.size(); k += 2) {
            int i = cycle[k].first;
            int j = cycle[k].second;
            minValue = min(minValue, allocation[i][j]);
        }

        for (auto k = 0ul; k < cycle.size(); ++k) {
            int i = cycle[k].first;
            int j = cycle[k].second;
            if (k % 2 == 0) {
                allocation[i][j] += minValue;
            } else {
                allocation[i][j] -= minValue;
            }
        }
    }
}

void test(TransportProblem problem) {
    vector<vector<int>> allocation = northWestCorner(problem);

    cout << "Solutia initiala folosind metoda coltului de Nord-Vest:\n";
    printMatrix(allocation);
    cout << "Costul total initial: " << calculateTotalCost(problem, allocation) << endl;

    MODI(problem, allocation);
    
    cout << "Solutia optimizata folosind metoda MODI:\n";
    printMatrix(allocation);
    cout << "Costul total minimizat: " << calculateTotalCost(problem, allocation) << endl;
}

void benchmark(TransportProblem inputProblem, int numIterations) {
    vector<double> executionTimes;

    for (int i = 0; i < numIterations; ++i) {
        TransportProblem problem{inputProblem};

        // Pornim cronometrul
        auto start = high_resolution_clock::now();

        // Rezolvati problema de transport
        vector<vector<int>> allocation = northWestCorner(problem);
        MODI(problem, allocation);

        // Oprim cronometrul
        auto stop = high_resolution_clock::now();

        // Calculam timpul de executie pentru aceasta iteratie
        auto duration = duration_cast<microseconds>(stop - start);
        double durationSec = duration.count() / 1000000.0; // Convertim in secunde

        // Adaugam timpul de executie la lista de executii
        executionTimes.push_back(durationSec);
    }

    // Calculam timpul mediu de executie
    double averageTime = accumulate(executionTimes.begin(), executionTimes.end(), 0.0) / executionTimes.size();

    // Afisam rezultatele
    cout << "Timpul mediu de executie pentru o iteratie: " << averageTime << " secunde." << endl;
}

int main() {
    TransportProblem problem = {
        {
            {20, 30, 10},
            {30, 40, 25},
            {35, 15, 20}
        },
        {100, 300, 100},
        {150, 125, 225}
    };

    test(problem);

    benchmark(problem, 1000000);

    return 0;
}
