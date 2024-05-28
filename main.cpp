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

// Funcția care printează matricea
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

// Metoda colțului de Nord-Vest
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

// Calculează costul total al unei soluții
int calculateTotalCost(const TransportProblem& problem, const vector<vector<int>>& allocation) {
    int totalCost = 0;
    for (int i = 0; i < allocation.size(); ++i) {
        for (int j = 0; j < allocation[i].size(); ++j) {
            totalCost += allocation[i][j] * problem.cost[i][j];
        }
    }
    return totalCost;
}

// Găsirea ciclului pentru ajustare
bool findCycle(int startI, int startJ, const vector<vector<int>>& allocation, vector<pair<int, int>>& cycle) {
    int m = allocation.size();
    int n = allocation[0].size();
    vector<vector<int>> visited(m, vector<int>(n, 0));
    vector<pair<int, int>> path;

    function<bool(int, int, int, int)> dfs = [&](int i, int j, int prevI, int prevJ) -> bool {
        if (visited[i][j] == 2) return false;
        if (visited[i][j] == 1) {
            // Ciclul este găsit
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
        // Calcularea potențialelor
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

        // Găsirea celei mai negative valori delta
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

        // Dacă nu există valori negative delta, soluția este optimă
        if (minDelta >= 0) break;

        // Găsirea ciclului în matricea de alocare
        vector<pair<int, int>> cycle;
        findCycle(minI, minJ, allocation, cycle);

        // Ajustarea alocării pe baza ciclului găsit
        int minValue = INF;
        for (int k = 1; k < cycle.size(); k += 2) {
            int i = cycle[k].first;
            int j = cycle[k].second;
            minValue = min(minValue, allocation[i][j]);
        }

        for (int k = 0; k < cycle.size(); ++k) {
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

    vector<vector<int>> allocation = northWestCorner(problem);

    cout << "Soluția inițială folosind metoda colțului de Nord-Vest:\n";
    printMatrix(allocation);
    cout << "Costul total inițial: " << calculateTotalCost(problem, allocation) << endl;

    MODI(problem, allocation);
    
    cout << "Soluția optimizată folosind metoda MODI:\n";
    printMatrix(allocation);
    cout << "Costul total minimizat: " << calculateTotalCost(problem, allocation) << endl;

    //  Numărul de iterații pentru benchmarking
    const int numIterations = 1000000;
    vector<double> executionTimes;

    for (int i = 0; i < numIterations; ++i) {
        TransportProblem problem = {
            {
                {20, 30, 10},
                {30, 40, 25},
                {35, 15, 20}
            },
            {100, 300, 100},
            {150, 125, 225}
        };
        // Începeți cronometrul
        auto start = high_resolution_clock::now();

        // Rezolvați problema de transport
        vector<vector<int>> allocation = northWestCorner(problem);
        MODI(problem, allocation);

        // Oprim cronometrul
        auto stop = high_resolution_clock::now();

        // Calculăm timpul de execuție pentru această iterație
        auto duration = duration_cast<microseconds>(stop - start);
        double durationSec = duration.count() / 1000000.0; // Convertim în secunde

        // Adăugăm timpul de execuție la lista de execuții
        executionTimes.push_back(durationSec);
    }

    // Calculăm timpul mediu de execuție
    double averageTime = accumulate(executionTimes.begin(), executionTimes.end(), 0.0) / executionTimes.size();

    // Afișăm rezultatele
    cout << "Timpul mediu de execuție pentru o iterație: " << averageTime << " secunde." << endl;

    return 0;
}
