#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>

class Monitor {
private:
    int buffer;
    bool ready;
    std::mutex mutex;
    std::condition_variable condition;

public:
    Monitor() {
        ready = false;
    }

    void set_buffer(int& message) {
        std::lock_guard<std::mutex> lock(mutex);
        buffer = message;
        ready = true;
        condition.notify_one();
    }

    int get_buffer() {
        std::unique_lock<std::mutex> lock(mutex);
        condition.wait(lock,[this]() { return ready; });

        ready = false;
        return buffer;
    }
};

class Producer {
private:
    Monitor& monitor;
    int message;
public:
    Producer(Monitor& monitor) : monitor(monitor), message(0) {}

    void produce() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << "Отправлено: " << message << '\n';

            monitor.set_buffer(message);
            message++;
        }
    }

};

class Consumer {
private:
    char id;
    Monitor& monitor;
    int message;
public:
    Consumer(char id, Monitor& monitor) : id(id), monitor(monitor), message(0) {}

    void consume() {
        while(true) {
            message = monitor.get_buffer();
            std::cout << "Потребитель " << id << " получил: " << message << '\n';
        }
    }
};

int main() {
    Monitor monitor;
    Producer prod(monitor);
    Consumer cons('A',monitor);

    std::thread prod_thread(&Producer::produce, &prod);
    std::thread cons_thread(&Consumer::consume, &cons);

    prod_thread.join();
    cons_thread.join();
    return 0;
}