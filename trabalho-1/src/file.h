#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

struct ProcessParams
{
public:
	int creation_time;
	int duration; //seconds
	int priority;
	int index;

	ProcessParams(int c, int d, int p, int i) { 
		creation_time = c;
		duration = d;
		priority = p;
		index = i;
	}

	friend ostream &operator<<(ostream& os, const ProcessParams& p) {
		os << "Creation time = " << p.creation_time << " duration = " << p.duration << " priority = " << p.priority << endl;
		return os;
	}
};


class File {
	ifstream myfile; 
	vector<ProcessParams> processes;

public:
	File() {
		myfile.open("entrada.txt");
		if (!myfile.is_open()) {
			cout << "Erro ao abrir o arquivo!\n";
		}
	}
	
	void read_file() {

		int index = 0;
	
		int a, b, c;
		
		if (!myfile.is_open()) return;
		
		while (myfile >> a >> b >> c) {
			ProcessParams p(a, b, c, index++);
			processes.push_back(p);
		}

		cout << "Quantidade de processos lidos do arquivo: " << processes.size() << endl;
	}

	void print_processes_params() {
		vector<ProcessParams>::iterator iter = processes.begin();

		for(; iter < processes.end(); iter++) {
			ProcessParams p = *iter;
			cout << p;
		}
	}

	vector<ProcessParams> get_processes() {
		return processes;
	}
};