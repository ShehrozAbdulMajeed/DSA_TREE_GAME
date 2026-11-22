#include "StringUtil.h"
#include <cstring>

void copyText(char* destination, int capacity, const char* source) {
    if (destination == 0 || capacity <= 0) return;
    if (source == 0) { destination[0] = '\0'; return; }
    int i = 0;
    while (i < capacity - 1 && source[i] != '\0') {
        destination[i] = source[i];
        i++;
    }
    destination[i] = '\0';
}

void appendText(char* destination, int capacity, const char* source) {
    if (destination == 0 || source == 0 || capacity <= 0) return;
    int length = (int)std::strlen(destination);
    if (length >= capacity) return;
    int i = 0;
    while (length + i < capacity - 1 && source[i] != '\0') {
        destination[length + i] = source[i];
        i++;
    }
    destination[length + i] = '\0';
}

void appendInt(char* destination, int capacity, int value) {
    if (destination == 0 || capacity <= 0) return;
    char digits[16];
    int count = 0;
    unsigned int number;
    if (value < 0) number = 0u - (unsigned int)value;
    else number = (unsigned int)value;
    do {
        digits[count] = (char)('0' + number % 10);
        count++;
        number /= 10;
    } while (number > 0 && count < 15);
    if (value < 0) digits[count++] = '-';
    int length = (int)std::strlen(destination);
    int i = count - 1;
    while (i >= 0 && length < capacity - 1) {
        destination[length] = digits[i];
        length++;
        i--;
    }
    destination[length] = '\0';
}
