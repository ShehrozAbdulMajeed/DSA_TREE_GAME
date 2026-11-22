#pragma once
// Tiny string helpers so we don't need <string> / <cstdio>.
inline int tLen(const char* s) { int n = 0; while (s[n]) n++; return n; }
inline void tAppend(char* dst, int cap, const char* s) {
	int n = tLen(dst), i = 0;
	while (s[i] && n < cap - 1) dst[n++] = s[i++];
	dst[n] = 0;
}
inline void tCopy(char* dst, int cap, const char* s) { dst[0] = 0; tAppend(dst, cap, s); }
inline void tAppendInt(char* dst, int cap, int v) {
	char tmp[16]; int n = 0;
	long long x = v; bool neg = x < 0; if (neg) x = -x;
	if (x == 0) tmp[n++] = '0';
	while (x > 0) { tmp[n++] = (char)('0' + x % 10); x /= 10; }
	char out[20]; int o = 0;
	if (neg) out[o++] = '-';
	while (n > 0) out[o++] = tmp[--n];
	out[o] = 0;
	tAppend(dst, cap, out);
}