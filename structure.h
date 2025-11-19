#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <mutex>
#include <stdexcept>
#include <format>

namespace multi_threaded_data
{
    /**
     * @brief Структура даних для багатопотокової програми з 3 цілих полів.
     * Оптимізована для Варіанту №11: String=40%, Write2=20%.
     * Використовує 3 окремі std::mutex для ізоляції полів.
     */
    class OptimizedDataStructure
    {
    private:
        // Умова: m=3 цілих поля
        std::vector<int> m_data = {0, 0, 0};

        // Схема захисту: 3 окремі м'ютекси для ізоляції полів
        // Це дозволяє незалежним Read/Write операціям виконуватися паралельно.
        // Операція 'string' блокує всі три атомарно.
        mutable std::mutex m_mutexes[3];

    public:
        // Заборона копіювання та присвоєння
        OptimizedDataStructure() = default;
        ~OptimizedDataStructure() = default;
        OptimizedDataStructure(const OptimizedDataStructure&) = delete;
        OptimizedDataStructure& operator=(const OptimizedDataStructure&) = delete;

        // Операції добування та встановлення значення

        int read(size_t field_index) const
        {
            if (field_index >= m_data.size()) {
                throw std::out_of_range("Field index out of range.");
            }
            // lock_guard для потокобезпечного читання одного поля
            std::lock_guard<std::mutex> lock(m_mutexes[field_index]);
            return m_data[field_index];
        }

        void write(size_t field_index, int value)
        {
            if (field_index >= m_data.size()) {
                throw std::out_of_range("Field index out of range.");
            }
            // lock_guard для потокобезпечного запису одного поля
            std::lock_guard<std::mutex> lock(m_mutexes[field_index]);
            m_data[field_index] = value;
        }

        // Перевантажений operator string

        operator std::string() const
        {
            // Операція 'string' (40%) вимагає читання ВСІХ полів.

            // 1. Атомарне захоплення всіх м'ютексів для запобігання Deadlock
            std::lock(m_mutexes[0], m_mutexes[1], m_mutexes[2]);

            // 2. Створюємо блокувальники (RAII)
            std::lock_guard<std::mutex> lock0(m_mutexes[0], std::adopt_lock);
            std::lock_guard<std::mutex> lock1(m_mutexes[1], std::adopt_lock);
            std::lock_guard<std::mutex> lock2(m_mutexes[2], std::adopt_lock);

            // Читання усіх полів після їх успішного блокування
            return std::format("[{}, {}, {}]", m_data[0], m_data[1], m_data[2]);
        }
    };
}