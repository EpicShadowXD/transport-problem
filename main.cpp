#include <iostream>
#include <vector>
#include <limits>
#include <queue>

using namespace std;

const double INF = numeric_limits<double>::infinity();

struct TransportProblem {
    int m, n;
    vector<double> supply;
    vector<double> demand;
    vector<vector<double>> cost;
    vector<vector<double>> solution;

    TransportProblem(int m, int n) : m(m), n(n), supply(m), demand(n), cost(m, vector<double>(n)), solution(m, vector<double>(n, 0)) {}

    void north_west_corner_rule() {
        int i = 0, j = 0;
        while (i < m && j < n) {
            double allocation = min(supply[i], demand[j]);
            solution[i][j] = allocation;
            supply[i] -= allocation;
            demand[j] -= allocation;

            if (supply[i] == 0) {
                i++;
            } else {
                j++;
            }
        }
    }

    bool find_cycle(int start_i, int start_j, vector<vector<bool>> &visited, vector<pair<int, int>> &cycle) {
        int i = start_i, j = start_j;
        cycle.push_back({i, j});
        visited[i][j] = true;
        
        while (true) {
            bool found = false;
            for (int k = 0; k < n; k++) {
                if (solution[i][k] > 0 && !visited[i][k]) {
                    j = k;
                    cycle.push_back({i, j});
                    visited[i][j] = true;
                    found = true;
                    break;
                }
            }
            if (!found) return false;
            for (int k = 0; k < m; k++) {
                if (solution[k][j] > 0 && !visited[k][j]) {
                    i = k;
                    cycle.push_back({i, j});
                    visited[i][j] = true;
                    if (i == start_i && j == start_j) return true;
                    break;
                }
            }
            if (i == start_i && j == start_j) return true;
        }
        return false;
    }

    void solve() {
        north_west_corner_rule();

        while (true) {
            vector<double> u(m, INF), v(n, INF);
            u[0] = 0;

            bool updated = true;
            while (updated) {
                updated = false;
                for (int i = 0; i < m; ++i) {
                    for (int j = 0; j < n; ++j) {
                        if (solution[i][j] > 0) {
                            if (u[i] != INF && v[j] == INF) {
                                v[j] = cost[i][j] - u[i];
                                updated = true;
                            } else if (v[j] != INF && u[i] == INF) {
                                u[i] = cost[i][j] - v[j];
                                updated = true;
                            }
                        }
                    }
                }
            }

            double min_reduced_cost = 0;
            int min_i = -1, min_j = -1;
            for (int i = 0; i < m; ++i) {
                for (int j = 0; j < n; ++j) {
                    if (solution[i][j] == 0) {
                        double reduced_cost = cost[i][j] - u[i] - v[j];
                        if (reduced_cost < min_reduced_cost) {
                            min_reduced_cost = reduced_cost;
                            min_i = i;
                            min_j = j;
                        }
                    }
                }
            }

            if (min_reduced_cost >= 0) break;

            vector<vector<bool>> visited(m, vector<bool>(n, false));
            vector<pair<int, int>> cycle;
            if (!find_cycle(min_i, min_j, visited, cycle)) break;

            double theta = INF;
            for (int k = 1; k < cycle.size(); k += 2) {
                theta = min(theta, solution[cycle[k].first][cycle[k].second]);
            }

            for (int k = 0; k < cycle.size(); ++k) {
                int i = cycle[k].first;
                int j = cycle[k].second;
                if (k % 2 == 0) {
                    solution[i][j] += theta;
                } else {
                    solution[i][j] -= theta;
                }
            }
        }
    }

    void print_solution() {
        cout << "Optimal transport plan:" << endl;
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < n; j++) {
                cout << solution[i][j] << " ";
            }
            cout << endl;
        }
    }
};

int main() {
    int m = 3, n = 3;
    TransportProblem problem(m, n);

    problem.supply = {20, 30, 25};
    problem.demand = {10, 25, 40};
    problem.cost = {{8, 6, 10},
                    {9, 12, 13},
                    {14, 9, 16}};

    problem.solve();
    problem.print_solution();

    return 0;
}
