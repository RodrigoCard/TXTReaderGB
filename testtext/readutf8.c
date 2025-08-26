#include <stdio.h>
#include <stdint.h>

#include <windows.h>
#include <locale.h>

#define BUFSIZE 4096

// run on terminal before if wonky chars:
// chcp 65001
// for utf8

void show_utf8_chars(FILE *f) {
    uint8_t buf[BUFSIZE];
    size_t n;

    SetConsoleOutputCP(CP_UTF8); // Força o terminal a usar UTF-8
    setlocale(LC_ALL, ""); // Para printf e afins usarem UTF-8 internamente

    while ((n = fread(buf, 1, BUFSIZE, f)) > 0) {
        size_t i = 0;
        while (i < n) {
            uint8_t b = buf[i], len = 1;

            // determina comprimento pela faixa do byte inicial
            if (b < 0x80)             len = 1;        // ASCII
            else if ((b & 0xE0) == 0xC0) len = 2;      // 110xxxxx
            else if ((b & 0xF0) == 0xE0) len = 3;      // 1110xxxx
            else if ((b & 0xF8) == 0xF0) len = 4;      // 11110xxx
            else                         len = 1;      // inválido cai como 1

            // se a sequência estiver cortada no final do buffer, retrocede e espera o próximo bloco
            if (i + len > n) {
                // reposiciona o ponteiro do arquivo para
                // ler de novo a partir desse byte no próximo fread
                long back = (long)(n - i);
                fseek(f, -back, SEEK_CUR);
                break;
            }

            // imprime o “caractere” UTF‑8
            fwrite(buf + i, 1, len, stdout);
            i += len;
        }
    }
}

int main(int argc, char **argv) {
    FILE *f = argc > 1 ? fopen(argv[1], "rb") : stdin;
    if (!f) { perror("fopen"); return 1; }
    show_utf8_chars(f);
    if (f != stdin) fclose(f);
    return 0;
}

// A' AA aá éê ! teste
