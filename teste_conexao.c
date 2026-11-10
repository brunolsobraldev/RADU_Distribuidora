
#include <stdio.h>
#include "database.h"

int main() {

    printf("Testando conexao com o banco...\n");

    if (conectarBanco()) {
        printf("Banco RADU conectado com sucesso!\n");
    } else {
        printf("Nao foi possivel conectar.\n");
    }

    return 0;
}
