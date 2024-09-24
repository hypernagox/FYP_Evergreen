#pragma once
#include "pch.h"
#include <fstream>

// Vertex 구조체 (3차원 좌표)
struct Vertex {
    float x, y, z;
};

float CalculateDistance(const Vertex& v1, const Vertex& v2) {
    return std::sqrt(std::pow(v2.x - v1.x, 2) + std::pow(v2.y - v1.y, 2) + std::pow(v2.z - v1.z, 2));
}

struct Plane {
    float A, B, C, D;  

    Plane(const Vertex& v1, const Vertex& v2, const Vertex& v3) {
        Vertex v12 = { v2.x - v1.x, v2.y - v1.y, v2.z - v1.z };
        Vertex v13 = { v3.x - v1.x, v3.y - v1.y, v3.z - v1.z };

        // 법선 벡터를 계산
        A = v12.y * v13.z - v12.z * v13.y;
        B = v12.z * v13.x - v12.x * v13.z;
        C = v12.x * v13.y - v12.y * v13.x;
        D = -(A * v1.x + B * v1.y + C * v1.z);
    }

    float CalculateY(float x, float z) const {
        if (B != 0) {
            return -(A * x + C * z + D) / B;
        }
        else {
            return 0;
        }
    }
    float CalculateSlopeAngle() const {
        float normalLength = std::sqrt(A * A + B * B + C * C);
        float cosTheta = std::abs(B) / normalLength;  
        float theta = std::acos(cosTheta);  
        return theta;
    }
};

struct NaviCell {
    Vertex v1, v2, v3;     // 세 개의 Vertex로 구성된 삼각형
    Vertex center;         // 삼각형의 중점
    Vertex mid[3];         // 세 변의 중점
    Plane plane;           // 평면 방정식
    NaviCell* link[3];     // 이웃한 Cell
    float arrivalCost[3];  // 각 변의 중점까지의 거리

    NaviCell(Vertex v1, Vertex v2, Vertex v3) : v1(v1), v2(v2), v3(v3), plane(v1, v2, v3) {
        
        center = { (v1.x + v2.x + v3.x) / 3.0f, (v1.y + v2.y + v3.y) / 3.0f, (v1.z + v2.z + v3.z) / 3.0f };

      
        mid[0] = { (v1.x + v2.x) / 2.0f, (v1.y + v2.y) / 2.0f, (v1.z + v2.z) / 2.0f };
        mid[1] = { (v2.x + v3.x) / 2.0f, (v2.y + v3.y) / 2.0f, (v2.z + v3.z) / 2.0f };
        mid[2] = { (v3.x + v1.x) / 2.0f, (v3.y + v1.y) / 2.0f, (v3.z + v1.z) / 2.0f };

        link[0] = link[1] = link[2] = nullptr;

        for (int i = 0; i < 3; ++i) {
            arrivalCost[i] = CalculateDistance(center, mid[i]);
        }
    }


    

};


struct Grid {
    float cellSize; 
    std::unordered_map<int, std::vector<NaviCell*>> gridMap;

    Grid(float cellSize) : cellSize(cellSize) {}

   
    int GetCellIndex(const Vertex& point) const {
        int xIndex = static_cast<int>(point.x / cellSize);
        int zIndex = static_cast<int>(point.z / cellSize);
        return (xIndex << 16) | zIndex; 
    }


    void AddCell(NaviCell* cell) {
        int cellIndex = GetCellIndex(cell->center);
        gridMap[cellIndex].push_back(cell);
    }

    
    std::vector<NaviCell*> GetCellsForPoint(const Vertex& point) const {
        int cellIndex = GetCellIndex(point);
        auto it = gridMap.find(cellIndex);
        if (it != gridMap.end()) {
            return it->second;  
        }
        return {};  
    }
};

class NaviMesh {
public:
    std::vector<NaviCell*> cells;
    Grid* grid; 

    NaviMesh(float cellSize) {
        grid = new Grid(cellSize); 
    }

    ~NaviMesh() {
        delete grid;
        for (auto cell : cells) {
            delete cell;
        }
    }

    void LoadObj(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return;
        }

        std::vector<Vertex> tempVertices; 
        std::string line;

        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string prefix;
            iss >> prefix;

            if (prefix == "v") {
                Vertex v;
                iss >> v.x >> v.y >> v.z;
                tempVertices.push_back(v);
            }
            else if (prefix == "f") {
                int v1, v2, v3;
                iss >> v1 >> v2 >> v3;

                NaviCell* cell = new NaviCell(tempVertices[v1 - 1], tempVertices[v2 - 1], tempVertices[v3 - 1]);
                cells.push_back(cell);
                grid->AddCell(cell);  
            }
        }

        file.close();
        tempVertices.clear();
        LinkCells(); 
    }


    void LinkCells() {
        for (size_t i = 0; i < cells.size(); ++i) {
            for (size_t j = i + 1; j < cells.size(); ++j) {
                if (AreCellsNeighbors(cells[i], cells[j])) {
                    for (int k = 0; k < 3; ++k) {
                        if (cells[i]->link[k] == nullptr) {
                            cells[i]->link[k] = cells[j];
                            break;
                        }
                    }
                    for (int k = 0; k < 3; ++k) {
                        if (cells[j]->link[k] == nullptr) {
                            cells[j]->link[k] = cells[i];
                            break;
                        }
                    }
                }
            }
        }
    }

    bool AreCellsNeighbors(NaviCell* cell1, NaviCell* cell2) {
        Vertex edges1[3][2] = { {cell1->v1, cell1->v2}, {cell1->v2, cell1->v3}, {cell1->v3, cell1->v1} };
        Vertex edges2[3][2] = { {cell2->v1, cell2->v2}, {cell2->v2, cell2->v3}, {cell2->v3, cell2->v1} };

        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                if ((edges1[i][0].x == edges2[j][0].x && edges1[i][0].y == edges2[j][0].y && edges1[i][0].z == edges2[j][0].z &&
                    edges1[i][1].x == edges2[j][1].x && edges1[i][1].y == edges2[j][1].y && edges1[i][1].z == edges2[j][1].z) ||
                    (edges1[i][0].x == edges2[j][1].x && edges1[i][0].y == edges2[j][1].y && edges1[i][0].z == edges2[j][1].z &&
                        edges1[i][1].x == edges2[j][0].x && edges1[i][1].y == edges2[j][0].y && edges1[i][1].z == edges2[j][0].z)) {
                    return true;
                }
            }
        }
        return false;
    }

    float CalculateDistance(const Vertex& v1, const Vertex& v2) {
        return std::sqrt(std::pow(v2.x - v1.x, 2) +
            std::pow(v2.y - v1.y, 2) +
            std::pow(v2.z - v1.z, 2));
    }

    NaviCell* FindCellForPoint(const Vertex& point) {
        std::vector<NaviCell*> potentialCells = grid->GetCellsForPoint(point);
        NaviCell* closestCell = nullptr;
        float minDistance = std::numeric_limits<float>::infinity(); 

        for (NaviCell* cell : potentialCells) {
            if (IsPointInCell(point, cell)) {
                return cell; 
            }

            float distance = CalculateDistance(point, cell->center);
            if (distance < minDistance) {
                minDistance = distance;
                closestCell = cell;
            }
        }

        
        return closestCell;
    }

   
    bool IsPointInCell(const Vertex& point, NaviCell* cell) {
      
        auto Sign = [](const Vertex& p1, const Vertex& p2, const Vertex& p3) {
            return (p1.x - p3.x) * (p2.z - p3.z) - (p2.x - p3.x) * (p1.z - p3.z);
            };

        float d1 = Sign(point, cell->v1, cell->v2);
        float d2 = Sign(point, cell->v2, cell->v3);
        float d3 = Sign(point, cell->v3, cell->v1);

        bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
        bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);

        return !(hasNeg && hasPos);  
    }
};

// A* 알고리즘용 노드 구조체
struct AStarNode {
    NaviCell* cell;
    AStarNode* parent;
    float gCost;  
    float hCost;
    float fCost() const { return gCost + hCost; }
};


float Heuristic(const Vertex& start, const Vertex& goal, const NaviCell* currentCell) {
    // 유클리드 거리 계산
    float distance = std::sqrt(std::pow(goal.x - start.x, 2) +
        std::pow(goal.y - start.y, 2) +
        std::pow(goal.z - start.z, 2));

    

    return distance; 
}


std::vector<NaviCell*> ReconstructPath(AStarNode* node) {
    std::vector<NaviCell*> path;
    while (node != nullptr) {
        path.push_back(node->cell);
        node = node->parent;
    }
    std::reverse(path.begin(), path.end());
    return path;
}


std::vector<NaviCell*> AStar(NaviCell* startCell, NaviCell* goalCell) {
    std::priority_queue<AStarNode*, std::vector<AStarNode*>, std::function<bool(AStarNode*, AStarNode*)>> openSet(
        [](AStarNode* a, AStarNode* b) { return a->fCost() > b->fCost(); });
    std::unordered_map<NaviCell*, AStarNode*> allNodes;

    AStarNode* startNode = new AStarNode{ startCell, nullptr, 0.0f, Heuristic(startCell->center, goalCell->center, startCell) };
    openSet.push(startNode);
    allNodes[startCell] = startNode;

    AStarNode* bestNode = startNode; 

    while (!openSet.empty()) {
        AStarNode* currentNode = openSet.top();
        openSet.pop();

        // 가장 낮은 fCost를 가진 노드를 추적
        if (currentNode->fCost() < bestNode->fCost()) {
            bestNode = currentNode;
        }

        if (currentNode->cell == goalCell) {
            return ReconstructPath(currentNode);
        }

        // 이웃 셀 탐색
        for (int i = 0; i < 3; ++i) {
            NaviCell* neighbor = currentNode->cell->link[i];
            if (neighbor == nullptr) continue;

            float tentativeGCost = currentNode->gCost + currentNode->cell->arrivalCost[i];
            if (allNodes.find(neighbor) == allNodes.end()) {
                
                AStarNode* neighborNode = new AStarNode{
                    neighbor, currentNode, tentativeGCost, Heuristic(neighbor->center, goalCell->center, neighbor) };
                openSet.push(neighborNode);
                allNodes[neighbor] = neighborNode;
            }
            else if (tentativeGCost < allNodes[neighbor]->gCost) {
           
                AStarNode* neighborNode = allNodes[neighbor];
                neighborNode->gCost = tentativeGCost;
                neighborNode->parent = currentNode;
            }
        }
    }

    
    return ReconstructPath(bestNode);
}

