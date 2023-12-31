
#pragma once

#include <vector>
#include <queue>
#include <algorithm>
#include <iomanip>
#include <unordered_map>

#include "file.h"
#include "pcb.h"
#include "table.h"
#include "scheduler.h"

template <typename C>
class Simulator
{
    ProcessTable<C> table;
    Scheduler& scheduler;
    PCB* activeProcess = NULL;
    std::vector<int> result;
    int contextSwitchCounter;
    
    C activeContext;

    std::unordered_map<int, int> process_index_to_pid_map;

public:

    void show_context_table() {
        std::cout << "Active: ";
        activeContext.show();

        for (const PCB& pcb : table.getAllProcesses()) {
            std::cout << "P" << pcb.id << ": ";
            pcb.context->show();
        }
    }

    void switch_context(PCB* next_process) {
        #if DEBUG
        show_context_table();
        std::cout << "--- switching context. ";
        if (activeProcess) {
            std::cout << "saving active to " << activeProcess->id << ". ";
        }
        if (next_process) {
            std::cout << "loading " << next_process->id << " to active.";
        }
        std::cout << std::endl;
        #endif


        if (activeProcess) {
            table.saveContext(activeProcess, activeContext);
        } 
        if (next_process) {
            activeContext = table.loadContext(next_process);
            contextSwitchCounter++;
        }
        activeProcess = next_process;

        #if DEBUG
        show_context_table();
        std::cout << std::endl;
        #endif
    }
    
    Simulator(Scheduler& strategy) : scheduler(strategy) {}

    void simulate(std::vector<ProcessParams> processes) {
        // Garante que a simulação comece do zero
        int time = 0;
        contextSwitchCounter = 0;
        table.clear();
        result.clear();

        // preparar fila de criacao de processos
        // certifica que estao na ordem de criacao adequada
        std::sort(processes.begin(), processes.end(), [](ProcessParams a, ProcessParams b) {
            return a.creation_time < b.creation_time;
        });
        std::queue<ProcessParams> creationQueue;
        for (ProcessParams p : processes) creationQueue.push(p);

        // Simulação, cada iteração é uma unidade de tempo
        while (1) {
            // Checa quais processos devem ser criados no instante atual
            while (!creationQueue.empty()) {
                ProcessParams p = creationQueue.front();
                if (p.creation_time > time)
                    break;

                int id = table.createProcess(p.creation_time, p.duration, p.priority);
                if (id >= 0) {
                    PCB& process = table.getProcess(id);
                    if (process.finished()) {
                        table.changeState(&process, pFinished);
                        process.endTime = time;
                    }
                    process_index_to_pid_map.emplace(p.index, id);
                }

                creationQueue.pop();
            }
            // Envia os processos criados para o escalonador organiza-los
            for (PCB* process : table.getByState(pNew)) {
                table.changeState(process, pReady);
                scheduler.insert(*process);
            }

            // Analisa se o processo ativo deve ser trocado
            bool shouldSwitch = false;
            if (activeProcess) {
                // Testa se o processo atual encerrou
                if (activeProcess->finished()) {
                    table.changeState(activeProcess, pFinished);
                    activeProcess->endTime = time;
                    shouldSwitch = true;
                } else if (scheduler.test(*activeProcess)) {  // Testa se o processo atual deve ser preemptado
                    // devolve processo para a estratégia
                    table.changeState(activeProcess, pReady);
                    scheduler.insert(*activeProcess);
                    shouldSwitch = true;
                }
            } 

            // escolhe o proximo processo a ser executado e troca o contexto
            if (shouldSwitch || !activeProcess) {
                switch_context(scheduler.pick());
                if (activeProcess) table.changeState(activeProcess, pExecuting);
            }

            // Informacoes e simulacao do instante e processo atual
            time++;
            if (!activeProcess) {
                if (creationQueue.empty()) break;  // Se nao ha processos ativos e nao ha processos a serem criados, a simulacao termina
                result.push_back(-1);
                continue;  // Se nao ha processos ativos, mas ha processos a serem criados, a simulacao continua, sem simular a execucao de nenhum processo
            }
            activeProcess->executingTime++;
            activeContext.tick(activeProcess->id);
            result.push_back(activeProcess->id);
        }
    }

    std::vector<int> get_result() {
        return result;
    }

    void print_graph() {
        int total_processes = table.getProcessCount();
        int total_time = result.size();

        std::cout << "tempo ";
        for(int i = 0; i < total_processes; i++) {
            std::cout << "P" << i << " ";
        }
        std::cout << std::endl;
        for(int timestamp = 0; timestamp < total_time; timestamp++) {
            std::cout << left << std::setw(6) << std::to_string(timestamp) + "-" + std::to_string(timestamp+1);
            for (int i = 0; i < total_processes; i++) {
                if (process_index_to_pid_map.count(i) == 0) continue;
                int pid = process_index_to_pid_map.at(i);
                
                PCB& process = table.getProcess(pid);
                bool process_is_running_or_waiting = (process.startTime <= timestamp) && (process.endTime > timestamp);
                if (result[timestamp] == pid) {
                    std::cout << "## ";
                } else if (process_is_running_or_waiting) {
                    std::cout << "-- ";
                } else {
                    std::cout << "   ";
                }
            }
            std::cout << std::endl;
        }
    }

    void show_data() {
        int total_processes = table.getProcessCount();

        float average_wait_time = .0;
        for (PCB p : table.getAllProcesses()) {
            average_wait_time += p.endTime - p.startTime - p.duration;
        }
        average_wait_time /= total_processes;
        
        float average_turnaround_time = .0;
        for (int i = 0; i < total_processes; i++) {
            if (process_index_to_pid_map.count(i) == 0) continue;

            int pid = process_index_to_pid_map.at(i);
            PCB& p = table.getProcess(pid);
            std::cout << "Processo " << i << " - Turnaround time: " << p.endTime - p.startTime << std::endl;
            average_turnaround_time += p.endTime - p.startTime;
        }
        average_turnaround_time /= total_processes;

        std::cout << "Tempo médio de turnaround: " << average_turnaround_time << std::endl;
        std::cout << "Tempo médio de espera: " << average_wait_time << std::endl;
        std::cout << "Número de mudanças de contexto: " << contextSwitchCounter << std::endl;
        std::cout << "Tempo total de execução: " << result.size() << std::endl;
    }
};
