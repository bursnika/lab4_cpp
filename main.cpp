#include "structure.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <random>
#include <thread>
#include <algorithm>
#include <format>

using namespace std;
using namespace multi_threaded_data;

// Частоти операцій для Варіанту №11 (String=40%)
// Поле 0: R 10%, W 5% | Поле 1: R 10%, W 5% | Поле 2: R 10%, W 20% | String: 40%
const vector<pair<string, double>> V11_FREQUENCIES = {
        {"read 0", 0.10},    // P0 Read 10%
        {"write 0 1", 0.05}, // P0 Write 5%
        {"read 1", 0.10},    // P1 Read 10%
        {"write 1 1", 0.05}, // P1 Write 5%
        {"read 2", 0.10},    // P2 Read 10%
        {"write 2 1", 0.20}, // P2 Write 20%
        {"string", 0.40}     // String 40%
};

const vector<pair<string, double>> EQUAL_FREQUENCIES = {
        {"read 0", 1.0/7.0},
        {"write 0 1", 1.0/7.0},
        {"read 1", 1.0/7.0},
        {"write 1 1", 1.0/7.0},
        {"read 2", 1.0/7.0},
        {"write 2 1", 1.0/7.0},
        {"string", 1.0/7.0}
};

// Приклад для 'дуже сильно не відповідають умові'
// Умова: String=40%, Write2=20%. Тут: Read0=5%, String=5%, Write2=5%.
const vector<pair<string, double>> MISMATCH_FREQUENCIES = {
        {"read 0", 0.05},
        {"write 0 1", 0.05},
        {"read 1", 0.10},
        {"write 1 1", 0.05},
        {"read 2", 0.30}, // Збільшено читання
        {"write 2 1", 0.40}, // Збільшено запис
        {"string", 0.05}     // Зменшено string
};

constexpr int TOTAL_OPERATIONS = 100000;


/**
 * @brief Генерує файл з послідовністю дій.
 * @param filename Ім'я файлу.
 * @param frequencies Частоти операцій.
 */
void generate_file(const string& filename, const vector<pair<string, double>>& frequencies)
{
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    random_device rd;
    mt19937 gen(rd());
    vector<double> weights;
    for (const auto& p : frequencies) {
        weights.push_back(p.second);
    }

    discrete_distribution<> dist(weights.begin(), weights.end());

    for (int i = 0; i < TOTAL_OPERATIONS; ++i) {
        file << frequencies[dist(gen)].first << endl;
    }
}

/**
 * @brief Виконує дії з файлу в структурі даних.
 * @param ds Структура даних.
 * @param filename Ім'я файлу з послідовністю дій.
 */
void execute_actions(OptimizedDataStructure& ds, const string& filename)
{
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string command;
        ss >> command;

        if (command == "write") {
            int field_index, value;
            ss >> field_index >> value;
            ds.write(field_index, value);
        } else if (command == "read") {
            int field_index;
            ss >> field_index;
            ds.read(field_index);
        } else if (command == "string") {
            (string)ds;
        }
    }
}

/**
 * @brief Вимірює час виконання дій з файлів.
 * @param ds Структура даних.
 * @param filenames Вектор імен файлів (один файл на потік).
 * @param num_threads Кількість потоків.
 * @return Час виконання в мілісекундах.
 */
double measure_time(OptimizedDataStructure& ds, const vector<string>& filenames, int num_threads)
{
    auto start = chrono::high_resolution_clock::now();

    if (num_threads == 1) {
        execute_actions(ds, filenames[0]);
    } else {
        vector<thread> threads;
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back(execute_actions, ref(ds), filenames[i]);
        }
        for (auto& t : threads) {
            t.join();
        }
    }

    auto end = chrono::high_resolution_clock::now();
    return chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000.0;
}

void run_test_set(const string& file_prefix, const vector<pair<string, double>>& frequencies)
{
    cout << format("--- Тест: {} ---", file_prefix) << endl;

    vector<string> filenames;
    for(int i = 0; i < 3; ++i) {
        string filename = format("{}_{}.txt", file_prefix, i);
        generate_file(filename, frequencies);
        filenames.push_back(filename);
    }

    vector<int> num_threads_set = {1, 2, 3};

    cout << format("{:<15} {:<10} {:<10} {:<10}", "Threads", "Time(1t)", "Time(2t)", "Time(3t)") << endl;

    for (int nt : num_threads_set) {
        OptimizedDataStructure ds;
        vector<string> current_filenames;
        for(int i = 0; i < nt; ++i) {
            current_filenames.push_back(filenames[i]);
        }

        // Тут має бути цикл для усереднення результатів у звіті
        double time_ms = measure_time(ds, current_filenames, nt);
        cout << format("{:<15} {:.3f} ms", format("{}T", nt), time_ms) << endl;
    }
    cout << endl;
}

int main()
{
    cout << "Багатопотоковий проєкт (Варіант №11)" << endl;
    cout << "Кількість полів: 3" << endl;
    cout << "Оптимізація під String=40%, Write2=20%" << endl;

    // a) Частоти відповідають умові
    run_test_set("cond_match", V11_FREQUENCIES);

    // б) Всі частоти рівні
    run_test_set("cond_equal", EQUAL_FREQUENCIES);

    // в) Частоти дуже сильно не відповідають умові
    run_test_set("cond_mismatch", MISMATCH_FREQUENCIES);

    return 0;
}