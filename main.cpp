// market_data_feed_handler_sim.cpp
// Simulates a low-latency market data feed handler for HFT systems

#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include <chrono>
#include <unordered_map>
#include <cstring>  // for memcpy
#include <netinet/in.h> // sockaddr_in
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h> // for close()
#include <fcntl.h>

using namespace std;
using namespace chrono;

// Sample binary message format: [8-byte timestamp][4-byte price][4-byte quantity]
struct MarketDataMessage {
    uint64_t timestamp; // microseconds
    uint32_t price;     // in cents
    uint32_t quantity;
};

// Order book entry
struct OrderBookEntry {
    uint32_t price;
    uint32_t quantity;
};

mutex book_mutex;
unordered_map<uint32_t, OrderBookEntry> order_book; // keyed by price

// Timestamp in microseconds (μs)
uint64_t current_microseconds() {
    return duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
}

void process_message(const MarketDataMessage& msg) {
    lock_guard<mutex> lock(book_mutex);
    order_book[msg.price] = {msg.price, msg.quantity};
}

void print_order_book_snapshot() {
    lock_guard<mutex> lock(book_mutex);
    cout << "\nOrder Book Snapshot at " << current_microseconds() << " μs:\n";
    for (const auto& [price, entry] : order_book) {
        cout << "Price: " << price << ", Quantity: " << entry.quantity << endl;
    }
}

// Simulated UDP listener (in real HFT would use raw sockets or multicast)
void udp_listener(int port, atomic<bool>& running) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        cerr << "Failed to create socket" << endl;
        return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (::bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0){
        cerr << "Bind failed" << endl;
        close(sock);
        return;
    }

    char buffer[sizeof(MarketDataMessage)];
    while (running) {
        ssize_t len = recv(sock, buffer, sizeof(buffer), 0);
        if (len == sizeof(MarketDataMessage)) {
            MarketDataMessage msg;
            memcpy(&msg, buffer, sizeof(msg));
            process_message(msg);
        }
    }
    close(sock);
}

// Binary test message sender (localhost)
void send_test_messages(const string& ip, int port) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        cerr << "Failed to create sender socket" << endl;
        return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

    for (int i = 0; i < 20; ++i) {
        MarketDataMessage msg;
        msg.timestamp = current_microseconds();
        msg.price = 1000 + i;
        msg.quantity = 10 * (i + 1);
        sendto(sock, &msg, sizeof(msg), 0, (sockaddr*)&addr, sizeof(addr));
        this_thread::sleep_for(milliseconds(100));
    }
    close(sock);
}

int main() {
    atomic<bool> running(true);
    thread listener_thread(udp_listener, 9000, ref(running));
    thread sender_thread(send_test_messages, "127.0.0.1", 9000);

    for (int i = 0; i < 10; ++i) {
        this_thread::sleep_for(milliseconds(500));
        print_order_book_snapshot();
    }

    running = false;
    listener_thread.join();
    sender_thread.join();
    return 0;
}
