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

void show_utf8_chars2(FILE *f) {
    // uint8_t buf[BUFSIZE];
    size_t n;
    uint8_t b;

    while ((n = fread(&b, 1, 1, f)) > 0) {
        size_t i = 0;
        while (i < n) {
            fwrite(&b, 1, 1, stdout);
            i += 1;
        }
    }
}

void show_utf8_charsX(FILE *f) {
    uint8_t buf[BUFSIZE];
    size_t n;

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
            if (b == '\n')
                printf("(%02X)\n", b );
            else if (b == '\t')
                printf("\t %02X ", b );
            else
                printf("%02X ", b);
            i += len;
        }
    }
}

int main(int argc, char **argv) {
    // SetConsoleOutputCP(CP_UTF8); // Força o terminal a usar UTF-8
    // setlocale(LC_ALL, ""); // Para printf e afins usarem UTF-8 internamente
    FILE *f = argc > 1 ? fopen(argv[1], "rb") : stdin;
    if (!f) { perror("fopen"); return 1; }
    show_utf8_chars(f);
    if (f != stdin) fclose(f);
    printf("\n");

    f = argc > 1 ? fopen(argv[1], "rb") : stdin;
    if (!f) { perror("fopen"); return 1; }
    show_utf8_chars2(f);
    if (f != stdin) fclose(f);
    printf("\n");

    f = argc > 1 ? fopen(argv[1], "rb") : stdin;
    if (!f) { perror("fopen"); return 1; }
    show_utf8_charsX(f);
    if (f != stdin) fclose(f);

    return 0;
}

// A' AA aá éê ! teste
