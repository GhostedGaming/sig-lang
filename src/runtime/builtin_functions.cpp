#include <iostream>
#include <string>
#include <cmath>
#include <cstring>
#include <cstdlib>

extern "C" {
    // input(prompt) - reads a line from stdin, prints prompt first
    char* sig_input(const char* prompt) {
        if (prompt) {
            printf("%s", prompt);
            fflush(stdout);
        }
        
        char* buffer = (char*)malloc(1024);
        if (!buffer) {
            return nullptr;
        }
        
        if (fgets(buffer, 1024, stdin)) {
            // Remove trailing newline if present
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n') {
                buffer[len - 1] = '\0';
            }
            return buffer;
        }
        
        free(buffer);
        return nullptr;
    }
    
    // len(string) - returns length of a string
    int sig_len(const char* str) {
        if (!str) {
            return 0;
        }
        return (int)strlen(str);
    }
    
    // abs(number) - absolute value of a number
    double sig_abs(double x) {
        return fabs(x);
    }
    
    // sqrt(number) - square root of a number
    double sig_sqrt(double x) {
        if (x < 0) {
            return 0.0;  // Return 0 for negative inputs instead of NaN
        }
        return sqrt(x);
    }
    
    // max(a, b) - maximum of two numbers
    double sig_max(double a, double b) {
        return (a > b) ? a : b;
    }
    
    // min(a, b) - minimum of two numbers
    double sig_min(double a, double b) {
        return (a < b) ? a : b;
    }
}
