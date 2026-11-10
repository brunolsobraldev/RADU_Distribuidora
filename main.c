#include <stdio.h>

#include "produtos.h"
#include "estoque.h"
#include "vendas.h"
#include "relatorios.h"

/* ====================================
   LEITURA SEGURA DE OPCAO
   ==================================== */

static int lerOpcao(const char *prompt) {

    int valor;

    printf("%s", prompt);
    fflush(stdout);

    if (scanf("%d", &valor) != 1) {

        int caractere;

        while ((caractere = getchar()) != '\n' &&
               caractere != EOF) {
        }

        return -1;
    }

    return valor;
}

/* ====================================
   PROTOTIPOS DOS MENUS
   ==================================== */

void menuPrincipal(void);
void menuProdutos(void);

/* ====================================
   FUNCAO PRINCIPAL
   ==================================== */

int main(void) {

    int opcao;

    do {

        menuPrincipal();

        opcao = lerOpcao("\nEscolha uma opcao: ");

        switch (opcao) {

            // ====================================
            // PRODUTOS
            // ====================================

            case 1:
                menuProdutos();
                break;

            // ====================================
            // ESTOQUE
            // ====================================

            case 2:
                menuEstoque();
                break;

            // ====================================
            // VENDAS
            // ====================================

            case 3:
                menuVendas();
                break;

            // ====================================
            // RELATORIOS
            // ====================================

            case 4:
                menuRelatorios();
                break;

            // ====================================
            // SAIR
            // ====================================

            case 0:
                printf("\n====================================\n");
                printf("      ENCERRANDO O SISTEMA...\n");
                printf("====================================\n");
                break;

            default:
                printf("\nOpcao invalida!\n");
                break;
        }

    } while (opcao != 0);

    return 0;
}


/* ====================================
   MENU PRINCIPAL
   ==================================== */

void menuPrincipal(void) {

    printf("\n");
    printf("====================================\n");
    printf("       RADU DISTRIBUIDORA\n");
    printf("====================================\n");
    printf("1 - Produtos\n");
    printf("2 - Estoque\n");
    printf("3 - Vendas e pedidos\n");
    printf("4 - Relatorios\n");
    printf("0 - Sair\n");
    printf("====================================\n");
}


/* ====================================
   MENU DE PRODUTOS
   ==================================== */

void menuProdutos(void) {

    int opcao;
    int codigo;

    do {

        printf("\n");
        printf("====================================\n");
        printf("          MENU DE PRODUTOS\n");
        printf("====================================\n");
        printf("1 - Cadastrar produto\n");
        printf("2 - Pesquisar produto\n");
        printf("3 - Listar produtos\n");
        printf("4 - Alterar produto\n");
        printf("5 - Excluir produto\n");
        printf("0 - Voltar\n");
        printf("====================================\n");

        opcao = lerOpcao("\nEscolha uma opcao: ");

        switch (opcao) {

            // ====================================
            // CADASTRAR
            // ====================================

            case 1:

                cadastrarProduto();

                break;


            // ====================================
            // PESQUISAR
            // ====================================

            case 2:

                printf("\n");
                printf("====================================\n");
                printf("       PESQUISAR PRODUTO\n");
                printf("====================================\n");

                codigo = lerOpcao("Digite o codigo do produto: ");

                if (codigo <= 0) {
                    printf("\nErro: codigo de produto invalido.\n");
                    break;
                }

                buscarProdutoPorCodigo(codigo);

                break;


            // ====================================
            // LISTAR
            // ====================================

            case 3:

                listarProdutos();

                break;


            // ====================================
            // ALTERAR
            // ====================================

            case 4:

                printf("\n");
                printf("====================================\n");
                printf("          ALTERAR PRODUTO\n");
                printf("====================================\n");

                codigo = lerOpcao("Digite o codigo do produto: ");

                if (codigo <= 0) {
                    printf("\nErro: codigo de produto invalido.\n");
                    break;
                }

                alterarProduto(codigo);

                break;


            // ====================================
            // EXCLUIR
            // ====================================

            case 5:

                printf("\n");
                printf("====================================\n");
                printf("          EXCLUIR PRODUTO\n");
                printf("====================================\n");

                codigo = lerOpcao("Digite o codigo do produto: ");

                if (codigo <= 0) {
                    printf("\nErro: codigo de produto invalido.\n");
                    break;
                }

                excluirProduto(codigo);

                break;


            // ====================================
            // VOLTAR
            // ====================================

            case 0:

                printf("\nVoltando ao menu principal...\n");

                break;


            // ====================================
            // OPCAO INVALIDA
            // ====================================

            default:

                printf("\nOpcao invalida!\n");

                break;
        }

    } while (opcao != 0);
}
