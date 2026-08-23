//
// Created by Ashish on 21-08-2026.
//

#include<bits/stdc++.h>
#include "../Model/GenericRubiksCube.h"
//#include "../Model/PatternDatabase/PatternDatabase.h"
#include "../PatternDatabases/CornerPatternDatabase.h"
#include <cassert>


#ifndef RUBIKS_IDASTARSOLVER_H
#define RUBIKS_IDASTARSOLVER_H

template<typename T, typename H>
class IDAstarSolver {
private:
    CornerPatternDatabase cornerDB;
    vector<GenericRubiksCube::MOVE> moves;
    unordered_map<T, GenericRubiksCube::MOVE, H> move_done;
    unordered_map<T, bool, H> visited;

    struct Node {
        T cube;
        int depth;
        int estimate;

        Node(T _cube, int _depth, int _estimate) : cube(_cube), depth(_depth), estimate(_estimate) {};
    };

    struct compareCube {
        bool operator()(pair<Node, int> const &p1, pair<Node, int> const &p2) {
            auto n1 = p1.first, n2 = p2.first;
            if (n1.depth + n1.estimate == n2.depth + n2.estimate) {
                return n1.estimate > n2.estimate;
            } else return n1.depth + n1.estimate > n2.depth + n2.estimate;
        }
    };

    void resetStructure() {
        moves.clear();
        move_done.clear();
        visited.clear();
    }

// returns {solved cube, bound}: if the cube was solved
// returns {rubiksCube, next_bound}, if the cube was not solved
    pair<T, int> IDAstar(int bound, chrono::steady_clock::time_point start_time, std::atomic<bool>* cancel_flag = nullptr) {
//        priority_queue contains pair(Node, move done to reach that)
        priority_queue<pair<Node, int>, vector<pair<Node, int>>, compareCube> pq;
        Node start = Node(rubiksCube, 0, cornerDB.getNumMoves(rubiksCube));
        pq.push(make_pair(start, 0));
        int next_bound = 100;
        int nodes_visited = 0;
        while (!pq.empty()) {
            nodes_visited++;
            if (nodes_visited % 10000 == 0) {
                if (cancel_flag && cancel_flag->load()) {
                    return make_pair(rubiksCube, -1); // Canceled by user
                }
                auto curr_time = chrono::steady_clock::now();
                if (chrono::duration_cast<chrono::seconds>(curr_time - start_time).count() > 30) {
                    return make_pair(rubiksCube, -1); // -1 signifies timeout
                }
            }
            auto p = pq.top();
            Node node = p.first;
            pq.pop();

            if (visited[node.cube]) continue;

            visited[node.cube] = true;
            move_done[node.cube] = GenericRubiksCube::MOVE(p.second);

            if (node.cube.isSolved()) return make_pair(node.cube, bound);
            node.depth++;
            for (int i = 0; i < 18; i++) {
                auto curr_move = GenericRubiksCube::MOVE(i);
                node.cube.move(curr_move);
                if (!visited[node.cube]) {
                    node.estimate = cornerDB.getNumMoves(node.cube);
                    if (node.estimate + node.depth > bound) {
                        next_bound = min(next_bound, node.estimate + node.depth);
                    } else {
                        pq.push(make_pair(node, i));
                    }
                }
                node.cube.invert(curr_move);
            }

        }
        return make_pair(rubiksCube, next_bound);
    }

public:
    T rubiksCube;

    IDAstarSolver(T _rubiksCube, string fileName) {
        rubiksCube = _rubiksCube;
        cornerDB.fromFile(fileName);
    }

    vector<GenericRubiksCube::MOVE> solve(std::atomic<bool>* cancel_flag = nullptr) {
        int bound = 1;
        auto start_time = chrono::steady_clock::now();
        auto p = IDAstar(bound, start_time, cancel_flag);
        while (p.second != bound) {
            if (p.second == -1) return vector<GenericRubiksCube::MOVE>(); // Timeout

            resetStructure();
            bound = p.second;
            p = IDAstar(bound, start_time, cancel_flag);
        }
        T solved_cube = p.first;
        if (!solved_cube.isSolved()) return vector<GenericRubiksCube::MOVE>(); // Failsafe
        
        T curr_cube = solved_cube;
        while (!(curr_cube == rubiksCube)) {
            GenericRubiksCube::MOVE curr_move = move_done[curr_cube];
            moves.push_back(curr_move);
            curr_cube.invert(curr_move);
        }
        rubiksCube = solved_cube;
        reverse(moves.begin(), moves.end());
        return moves;
    }
};

#endif //RUBIKS_IDASTARSOLVER_H