#include <iostream>
#include <map>
#include <set>
#include <list>
#include <fstream>
#include <sstream>
#include <vector>
#include <stack>
#include <string>
#include <algorithm>

using namespace std;

typedef map<int, multimap<int, pair<double, string>>> Graph;

/**
*@brief Funkcja odczytująca układ ulic z pliku
*
*@param fileName Nazwa pliku
*@return Graf z układem ulic
*/
Graph LoadFromFile(const string& fileName) {

    Graph graph;
    ifstream in(fileName);

    if (in) {
        string line;
        while (getline(in, line)) {
            stringstream ss(line);
            int cross1, cross2;
            double lenght;
            string street;
            if (!(ss >> cross1)) continue;
            if (!(ss >> cross2)) continue;
            if (!(ss >> lenght)) continue;
            if (!getline(ss, street)) continue;
            graph[cross1].insert({ cross2, {lenght, street}});
            graph[cross2].insert({ cross1, {lenght, street}});
        }
        in.close();
    }
    else {
        cout << "Blad otwarcia pliku";
    }
    return graph;
}

/**
*@brief Funkcja dodajaca ścieżke do grafu
*
*@param graph Graf układ ulic
*@param cross1 Pierwszy wierzchołek
*@param cross2 Drugi wierzchołek
*@param weight Długość ulicy
*@param streetName Nazwa ulicy
*/
void addEdge(Graph& graph, int cross1, int cross2, double weight, string streetName) {
	graph[cross1].insert(make_pair(cross2, make_pair(weight, streetName)));
	graph[cross2].insert(make_pair(cross1, make_pair(weight, streetName)));
}

/**
*@brief Funkcja usuwająca ścieżke z grafu
*
*@param graph Graf układ ulic
*@param cross1 Pierwszy wierzchołek
*@param cross2 Drugi wierzchołek
*/
void removeEdge(Graph& graph, int cross1, int cross2) {

	auto road1 = graph[cross1].begin();
	while (road1 != graph[cross1].end()) {
		if (road1->first == cross2) {
			graph[cross1].erase(road1);
			break;
		}
		++road1;
	}

	auto road2 = graph[cross2].begin();
	while (road2 != graph[cross2].end()) {
		if (road2->first == cross1) {
			graph[cross2].erase(road2);
			break;
		}
		++road2;
	}
}

/**
*@brief Funkcja przeszukująca graf w głąb
*
*@param graf Graf układ ulic
*@param v Wierzchołek startowy
*@param visited lista odwiedzanych wierzchołków 
*/
void dfs(Graph& graph, int v, map<int, bool>& visited) {

	visited[v] = true;

	for (auto vertex = graph[v].begin(); vertex != graph[v].end(); ++vertex) {
		int neighbor = vertex->first;
		if (!visited[neighbor]) {
			dfs(graph, neighbor, visited);
		}
	}	
}

/**
*@brief Funkcja sprawdzająca czy graf jest spójny
*
*@param graph Graf układ ulic
*@return Zwraca wartość boolowską czy graf jest połączony czy nie
*/
bool isConnected(Graph& graph) {

	map<int, bool> visited;

	auto firstVertex = graph.begin();

	int startVertex = firstVertex->first;

	dfs(graph, startVertex, visited);

	for ( auto vertex = graph.begin(); vertex != graph.end(); ++vertex) {
		if (!visited[vertex->first]) {
			return false;
		}
	}
	return true;
}

/**
*@brief Funkcja znajduje najkrótsze ścieżeki w grafie od danego punktu 
*
*@param graph Graf z układem ulic
*@param start Startowy wierzchołek
*@return Zwraca mape z najkrótszymi ścieżkami
*/
map<int, double> dijkstra(const Graph& graph, int start) {

	map<int, double> distances;
	set<pair<double, int>> priorityQueue;

	for (const auto& vertex : graph) {
		distances[vertex.first] = numeric_limits<double>::infinity();
	}

	distances[start] = 0;	
	priorityQueue.insert({ 0, start });

	while (!priorityQueue.empty()) {
		int current = priorityQueue.begin()->second;
		priorityQueue.erase(priorityQueue.begin());
		
		for (const auto& edge : graph.at(current)) {
			int neighbor = edge.first;
			double weight = edge.second.first;

			if (distances[current] + weight < distances[neighbor]) {
				priorityQueue.erase({ distances[neighbor], neighbor });
				distances[neighbor] = distances[current] + weight;
				priorityQueue.insert({ distances[neighbor], neighbor });
			}
		}
	}
	return distances;
}

/**
*@brief Funkcja łącząca nieprzyste wierzchołki w grafie
*
*@param originalGraph Graf z układem ulic
*@return Zwraca graf wejściowy ale z połączonymi wierzchołkami o nieparzystych stopniach
*/
Graph connectOddDegreeVertices(const Graph& originalGraph) {

	vector<int> oddDegreeVertices;

	for (const auto& vertex : originalGraph) {
		if (vertex.second.size() % 2 != 0) {
			oddDegreeVertices.push_back(vertex.first);
		}
	}

	Graph modifiedGraph = originalGraph;	

	for (size_t i = 0; i < oddDegreeVertices.size(); i+=2) {
			int source = oddDegreeVertices[i];
			int target = oddDegreeVertices[i+1];

			map<int, double> shortestPaths = dijkstra(originalGraph, source);

			int current = target;
			while (current != source) {

				int previousVertex = min_element(originalGraph.at(current).begin(), originalGraph.at(current).end(),
					[&shortestPaths](const auto& leftSite, const auto& rightSite) {
						return shortestPaths[leftSite.first] + leftSite.second.first < shortestPaths[rightSite.first] + rightSite.second.first;
					})->first;

				pair<double, string> edgeInfo = originalGraph.at(current).find(previousVertex)->second;

				modifiedGraph[previousVertex].insert({ current, edgeInfo });
				modifiedGraph[current].insert({ previousVertex, edgeInfo });

				current = previousVertex;
			}
	}
	return modifiedGraph;
}

/**
*@brief Funkcja znajduje cykl Eulera w grafie - cykl który przechodzi przez każdą krawędź dokładnie raz
*
*@param graph Graf z układem ulic
*@param startVertex Startowy wierzchołek
*@return Zwraca listę z cyklem Eulera
*/
list<int> findEulerianCycle(Graph& graph, int startVertex) {

	stack<int> tempStack;
	list<int> finalRoad;

	tempStack.push(startVertex);

	while (!tempStack.empty()) {
		int topVertex = tempStack.top();

		if (!graph[topVertex].empty()) {
			auto vertex = graph[topVertex].begin();
			int neighbor = vertex->first;

			tempStack.push(neighbor);
			removeEdge(graph, topVertex, neighbor);
		}
		else {
			finalRoad.push_front(topVertex);
			tempStack.pop();
		}
	}
	return finalRoad;
}

/**
*@brief Funkcja wypisująca wynikowy graf do pliku
*
*@param graph Graf z układem ulic
*@param finalList Lista z najkrótszą drogą
*@param startCross Startowe skrzyżowanie
*@param fileName Nazwa pliku do którego wpisywany jest wynik
*/
void writeResultToFile(Graph graph, list<int> finalList,int startCross, string& fileName) {

	ofstream outputFile(fileName);

	if (outputFile) {
		int sizeOfRoad = finalList.size();
		int secondCross;

		for (int i = 0; i < sizeOfRoad; i++) {
			int firstCross = finalList.front();
			finalList.pop_front();
			
			if (!finalList.empty()) {
				secondCross = finalList.front();
			}
			else {
				secondCross = startCross;
			}

			for (const auto& line : graph) {
				if (line.first == firstCross) {
					for (const auto line2 : line.second) {
						if (line2.first==secondCross) {
							outputFile << firstCross << " " << secondCross << " " << line2.second.second<<"\n";
						}
					}
				}
			}
		}
		cout << "Najkrotsza trasa przeslana do pliku." << endl;
	}
	else {
		cout << "Blad otwarcia pliku wyjsciowego";
	}
	outputFile.close();
}

/**
*@brief Funkcja główna programu
*
*@param n Ilość argumentów wiersza poleceń
*@param carg Tablica argumentów wiersza poleceń
*@return Kod zakończenia programu
*/
int main(int n, char** carg)
{

	std::map<std::string, std::string> args;

	for (size_t i = 1; i < n; i += 2)
	{
		args[carg[i]] = carg[i + 1];
	}

	if (args["-i"].empty() || args["-p"].empty() || args["-o"].empty()) {
		cout << "Niepoprawne komendy";
	} 
	else {
		string inputFileName = args["-i"];
		int startCross = stoi(args["-p"]);
		string outputFileName = args["-o"];

		auto graph = LoadFromFile(inputFileName);
		
		if (isConnected(graph)) {

			Graph modifiedGraph = connectOddDegreeVertices(graph);

			list<int> finalListOfRoad = findEulerianCycle(modifiedGraph, startCross);
			
			writeResultToFile(graph, finalListOfRoad, startCross, outputFileName);
			
		}
		else {
			cout << "Graf nie jest spojny";
		}
	}	
        
	return 0;
}