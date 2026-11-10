#include <stdio.h>
#include "produtos.h"

int main() {

    int opcao;

    do {

        printf("\n====================================\n");
        printf("     RADU DISTRIBUIDORA\n");
        printf("====================================\n");
        printf("1 - Cadastrar produto\n");
        printf("2 - Listar produtos\n");
        printf("0 - Sair\n");
        printf("====================================\n");
        printf("Escolha uma opcao: ");

        scanf("%d", &opcao);

        switch (opcao) {

            case 1:
                cadastrarProduto();
                break;

            case 2:
                listarProdutos();
                break;

            case 0:
                printf("\nEncerrando programa...\n");
                break;

            default:
                printf("\nOpcao invalida!\n");
        }

    } while (opcao != 0);

    return 0;
}

void buscarProdutoPorCodigo(int codigo)
{
    SQLHSTMT hStmt;
    SQLRETURN ret;

    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

    char sql[] =
        "SELECT Cod_Produto, Nome, Volume, Tipo_Recipiente, "
        "Categoria, Preco_Compra, Preco_Venda, Preco_Atacado, "
        "Preco_Varejo, FK_CNPJ_Fornecedor "
        "FROM Produto "
        "WHERE Cod_Produto = ?";

    ret = SQLPrepare(hStmt, (SQLCHAR *)sql, SQL_NTS);

    if (!SQL_SUCCEEDED(ret))
    {
        printf("Erro ao preparar consulta.\n");
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return;
    }

    SQLBindParameter(
        hStmt,
        1,
        SQL_PARAM_INPUT,
        SQL_C_LONG,
        SQL_INTEGER,
        0,
        0,
        &codigo,
        0,
        NULL
    );

    ret = SQLExecute(hStmt);

    if (!SQL_SUCCEEDED(ret))
    {
        printf("Erro ao buscar produto.\n");
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return;
    }

    SQLINTEGER cod;
    char nome[100];
    char volume[50];
    char recipiente[50];
    char categoria[100];
    double precoCompra;
    double precoVenda;
    double precoAtacado;
    double precoVarejo;
    char fornecedor[20];

    SQLBindCol(hStmt, 1, SQL_C_LONG, &cod, 0, NULL);
    SQLBindCol(hStmt, 2, SQL_C_CHAR, nome, sizeof(nome), NULL);
    SQLBindCol(hStmt, 3, SQL_C_CHAR, volume, sizeof(volume), NULL);
    SQLBindCol(hStmt, 4, SQL_C_CHAR, recipiente, sizeof(recipiente), NULL);
    SQLBindCol(hStmt, 5, SQL_C_CHAR, categoria, sizeof(categoria), NULL);
    SQLBindCol(hStmt, 6, SQL_C_DOUBLE, &precoCompra, 0, NULL);
    SQLBindCol(hStmt, 7, SQL_C_DOUBLE, &precoVenda, 0, NULL);
    SQLBindCol(hStmt, 8, SQL_C_DOUBLE, &precoAtacado, 0, NULL);
    SQLBindCol(hStmt, 9, SQL_C_DOUBLE, &precoVarejo, 0, NULL);
    SQLBindCol(hStmt, 10, SQL_C_CHAR, fornecedor, sizeof(fornecedor), NULL);

    if (SQLFetch(hStmt) == SQL_SUCCESS)
    {
        printf("\n==============================================\n");
        printf("             PRODUTO ENCONTRADO\n");
        printf("==============================================\n");

        printf("Codigo: %d\n", cod);
        printf("Nome: %s\n", nome);
        printf("Volume: %s\n", volume);
        printf("Tipo de recipiente: %s\n", recipiente);
        printf("Categoria: %s\n", categoria);
        printf("Preco de compra: R$ %.2f\n", precoCompra);
        printf("Preco de venda: R$ %.2f\n", precoVenda);
        printf("Preco atacado: R$ %.2f\n", precoAtacado);
        printf("Preco varejo: R$ %.2f\n", precoVarejo);
        printf("Fornecedor: %s\n", fornecedor);

        printf("==============================================\n");
    }
    else
    {
        printf("\nProduto com codigo %d nao encontrado.\n", codigo);
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}