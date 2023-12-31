#pragma once

#include <vector>
#include <unordered_map>
#include <iostream>
#include "pcb.h"

template <typename C>
class ProcessTable {
    std::vector<PCB> processes;
    static const int MAX_PROCESS_COUNT = 100;

    std::unordered_map<ProcessState, std::vector<PCB*>> stateMap;

    std::vector<PCB*>& getCreateByState(ProcessState state) {
        if (stateMap.count(state) == 0) {
            stateMap.emplace(state, std::vector<PCB*>());
        }
        return stateMap.at(state);
    }

public:
    ProcessTable() {
        // std::vector naturalmente moveria a array para uma nova posição
        // da memoria quando ocorresse um resize, o que faria todos os ponteiros
        // de PCBs apontarem pra uma região invalida.
        processes.reserve(MAX_PROCESS_COUNT);
    }

    ~ProcessTable() {
        clear();
    }

    int createProcess(int creationTime, int duration, int priority) {
        if (processes.size() >= MAX_PROCESS_COUNT) return -1;

        int id = processes.size();
        C* context = new C();
        PCB& process = processes.emplace_back(id, creationTime, duration, priority, context);
        changeState(&process, pNew);

        return id;
    }

    PCB& getProcess(int id) {
        return processes.at(id);
    }

    const std::vector<PCB*> getByState(ProcessState state) {
        return getCreateByState(state);
    }

    const std::vector<PCB>& getAllProcesses() {
        return processes;
    }

    int getProcessCount() {
        return processes.size();
    }

    void changeState(PCB* process, ProcessState state) {
        // remove processo da lista do estado anterior
        std::vector<PCB*>& prevState = getCreateByState(process->processState);
        auto it = std::find(prevState.begin(), prevState.end(), process);
        if (it != prevState.end()) {
            prevState.erase(it);
        }
        // adiciona processo na lista do estado novo
        std::vector<PCB*>& newState = getCreateByState(state);
        auto newIt = std::find(newState.begin(), newState.end(), process);
        if (newIt == newState.end()) {
            newState.push_back(process);
        }

        process->processState = state;        
    }

    C loadContext(PCB* process) {
        return *static_cast<C*>(process->context);
    }

    void saveContext(PCB* process, C context) {
        *static_cast<C*>(process->context) = context;
    }

    void clear() {
        for (PCB& pcb : processes) {
            delete static_cast<C*>(pcb.context);
        }
        processes.clear();
        stateMap.clear();
    };
};