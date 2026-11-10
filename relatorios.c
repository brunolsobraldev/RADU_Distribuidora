#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <sql.h>
#include <sqlext.h>

#include "database.h"
#include "estoque.h"
#include "relatorios.h"

/* ============================================================
   LEITURA SEGURA DE ENTRADA
   ============================================================ */

static void lerTexto(const char *prompt, char *destino, size_t tamanho) {

    int caractere;
    size_t posicao = 0;

    printf("%s", prompt);
    fflush(stdout);

    while ((caractere = getchar()) != EOF &&
           (caractere == ' ' ||
            caractere == '\t' ||
            caractere == '\n' ||
            caractere == '\r')) {
    }

    if (caractere == EOF) {
        destino[0] = '\0';
        return;
    }

    while (caractere != EOF &&
           caractere != '\n' &&
           posicao + 1 < tamanho) {

        destino[posicao++] = (char)caractere;
        caractere = getchar();
    }

    destino[posicao] = '\0';

    while (caractere != EOF && caractere != '\n') {
        caractere = getchar();
    }
}

static int lerInteiro(const char *prompt, int *valor) {

    char linha[64];

    lerTexto(prompt, linha, sizeof(linha));

    if (linha[0] == '\0') {
        return 0;
    }

    return sscanf(linha, "%d", valor) == 1;
}

/* ============================================================
   APOIO ODBC
   ============================================================ */

static SQLHSTMT criarComando(SQLHDBC conexao) {

    SQLHSTMT comando = SQL_NULL_HSTMT;

    SQLRETURN resultado = SQLAllocHandle(
        SQL_HANDLE_STMT,
        conexao,
        &comando
    );

    if (!SQL_SUCCEEDED(resultado)) {
        printf("\nErro ao criar comando SQL.\n");
        imprimirErroODBC(SQL_HANDLE_DBC, conexao);
        return SQL_NULL_HSTMT;
    }

    return comando;
}

/* ============================================================
   MENU DE RELATORIOS
   ============================================================ */

void menuRelatorios(void) {

    int opcao;

    do {

        printf("\n");
        printf("====================================\n");
        printf("          MENU DE RELATORIOS\n");
        printf("====================================\n");
        printf("1 - Relatorio de estoque\n");
        printf("2 - Relatorio de vendas\n");
        printf("3 - Produtos mais vendidos\n");
        printf("4 - Produtos proximos do vencimento\n");
        printf("5 - Movimentacao de estoque\n");
        printf("0 - Voltar\n");
        printf("====================================\n");

        if (!lerInteiro("\nEscolha uma opcao: ", &opcao)) {
            opcao = -1;
        }

        switch (opcao) {

            case 1:
                relatorioEstoque();
                break;

            case 2:
                relatorioVendas();
                break;

            case 3:
                produtosMaisVendidos();
                break;

            case 4:
                produtosProximosVencimento();
                break;

            case 5:
                movimentacaoEstoque();
                break;

            case 0:
                printf("\nVoltando ao menu principal...\n");
                break;

            default:
                printf("\nOpcao invalida!\n");
                break;
        }

    } while (opcao != 0);
}

/* ============================================================
   RELATORIO DE ESTOQUE
   ============================================================ */

void relatorioEstoque(void) {

    printf("\n");
    printf("====================================\n");
    printf("        RELATORIO DE ESTOQUE\n");
    printf("====================================\n");

    /* Reutiliza a consulta de estoque ja implementada. */
    consultarEstoque();
}

/* ============================================================
   RELATORIO DE VENDAS
   ============================================================ */

void relatorioVendas(void) {

    SQLHENV ambiente = SQL_NULL_HENV;
    SQLHDBC conexao = SQL_NULL_HDBC;
    SQLHSTMT comando = SQL_NULL_HSTMT;

    SQLRETURN resultado;

    double totalVendas = 0.0;

    SQLLEN indicador;

    if (!abrirConexao(&ambiente, &conexao)) {
        return;
    }

    comando = criarComando(conexao);

    if (comando == SQL_NULL_HSTMT) {
        fecharConexao(ambiente, conexao);
        return;
    }

    {
        SQLCHAR consulta[] =
            "SELECT p.Num_Pedido, "
            "CONVERT(varchar(10), p.Data_Pedido, 103), "
            "c.Nome_Razao_Social, p.Valor_Total "
            "FROM dbo.PEDIDO p "
            "JOIN dbo.CLIENTE c ON c.Id_Cliente = p.FK_Id_Cliente "
            "ORDER BY p.Num_Pedido;";

        resultado = SQLExecDirect(comando, consulta, SQL_NTS);

        if (!SQL_SUCCEEDED(resultado)) {
            printf("\nErro ao gerar relatorio de vendas.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);

            SQLFreeHandle(SQL_HANDLE_STMT, comando);
            fecharConexao(ambiente, conexao);
            return;
        }
    }

    printf("\n");
    printf("================================================================\n");
    printf("                     RELATORIO DE VENDAS\n");
    printf("================================================================\n");
    printf(
        "%-8s | %-10s | %-25s | %12s\n",
        "Pedido", "Data", "Cliente", "Total"
    );
    printf("----------------------------------------------------------------\n");

    {
        int encontrou = 0;

        while (SQL_SUCCEEDED(SQLFetch(comando))) {

            SQLINTEGER numPedido = 0;
            char dataPedido[11];
            char nomeCliente[121];
            double valorTotal = 0.0;

            encontrou = 1;

            SQLGetData(
                comando, 1, SQL_C_SLONG,
                &numPedido, sizeof(numPedido), &indicador
            );

            SQLGetData(
                comando, 2, SQL_C_CHAR,
                dataPedido, sizeof(dataPedido), &indicador
            );

            SQLGetData(
                comando, 3, SQL_C_CHAR,
                nomeCliente, sizeof(nomeCliente), &indicador
            );

            SQLGetData(
                comando, 4, SQL_C_DOUBLE,
                &valorTotal, sizeof(valorTotal), &indicador
            );

            printf(
                "%-8d | %-10s | %-25.25s | %12.2f\n",
                (int)numPedido,
                dataPedido,
                nomeCliente,
                valorTotal
            );

            totalVendas += valorTotal;
        }

        if (!encontrou) {
            printf("Nenhum pedido registrado.\n");
        }
    }

    printf("----------------------------------------------------------------\n");
    printf("TOTAL DE VENDAS: R$ %.2f\n", totalVendas);
    printf("================================================================\n");

    SQLFreeHandle(SQL_HANDLE_STMT, comando);
    fecharConexao(ambiente, conexao);
}

/* ============================================================
   PRODUTOS MAIS VENDIDOS
   ============================================================ */

void produtosMaisVendidos(void) {

    SQLHENV ambiente = SQL_NULL_HENV;
    SQLHDBC conexao = SQL_NULL_HDBC;
    SQLHSTMT comando = SQL_NULL_HSTMT;

    SQLRETURN resultado;

    SQLLEN indicador;

    if (!abrirConexao(&ambiente, &conexao)) {
        return;
    }

    comando = criarComando(conexao);

    if (comando == SQL_NULL_HSTMT) {
        fecharConexao(ambiente, conexao);
        return;
    }

    {
        SQLCHAR consulta[] =
            "SELECT TOP 10 pr.Nome, SUM(i.Qtd_Vendida) AS Total "
            "FROM dbo.ITENS_PEDIDO i "
            "JOIN dbo.PRODUTO pr ON pr.Cod_Produto = i.FK_Cod_Produto "
            "GROUP BY pr.Nome "
            "ORDER BY Total DESC, pr.Nome;";

        resultado = SQLExecDirect(comando, consulta, SQL_NTS);

        if (!SQL_SUCCEEDED(resultado)) {
            printf("\nErro ao gerar relatorio de produtos mais vendidos.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);

            SQLFreeHandle(SQL_HANDLE_STMT, comando);
            fecharConexao(ambiente, conexao);
            return;
        }
    }

    printf("\n");
    printf("====================================\n");
    printf("       PRODUTOS MAIS VENDIDOS\n");
    printf("====================================\n");

    {
        int posicao = 1;
        int encontrou = 0;

        while (SQL_SUCCEEDED(SQLFetch(comando))) {

            char nome[101];
            SQLINTEGER quantidade = 0;

            encontrou = 1;

            SQLGetData(
                comando, 1, SQL_C_CHAR,
                nome, sizeof(nome), &indicador
            );

            SQLGetData(
                comando, 2, SQL_C_SLONG,
                &quantidade, sizeof(quantidade), &indicador
            );

            printf(
                "%d. %-30.30s %6d unidades\n",
                posicao,
                nome,
                (int)quantidade
            );

            posicao++;
        }

        if (!encontrou) {
            printf("\nNenhuma venda registrada.\n");
        }
    }

    printf("====================================\n");

    SQLFreeHandle(SQL_HANDLE_STMT, comando);
    fecharConexao(ambiente, conexao);
}

/* ============================================================
   PRODUTOS PROXIMOS DO VENCIMENTO
   ============================================================ */

void produtosProximosVencimento(void) {

    SQLHENV ambiente = SQL_NULL_HENV;
    SQLHDBC conexao = SQL_NULL_HDBC;
    SQLHSTMT comando = SQL_NULL_HSTMT;

    SQLRETURN resultado;

    SQLLEN indicador;

    if (!abrirConexao(&ambiente, &conexao)) {
        return;
    }

    comando = criarComando(conexao);

    if (comando == SQL_NULL_HSTMT) {
        fecharConexao(ambiente, conexao);
        return;
    }

    {
        SQLCHAR consulta[] =
            "SELECT pr.Nome, l.Id_Lote, l.Saldo_Disponivel, "
            "CONVERT(varchar(10), l.Data_Validade, 103), "
            "CASE "
            "  WHEN l.Data_Validade < CAST(SYSDATETIME() AS date) "
            "       THEN 'VENCIDO' "
            "  ELSE 'PROXIMO DO VENCIMENTO' "
            "END "
            "FROM dbo.LOTE_ESTOQUE l "
            "JOIN dbo.PRODUTO pr ON pr.Cod_Produto = l.FK_Cod_Produto "
            "WHERE l.Saldo_Disponivel > 0 "
            "AND l.Data_Validade <= DATEADD(day, 30, "
            "    CAST(SYSDATETIME() AS date)) "
            "ORDER BY l.Data_Validade, pr.Nome;";

        resultado = SQLExecDirect(comando, consulta, SQL_NTS);

        if (!SQL_SUCCEEDED(resultado)) {
            printf("\nErro ao gerar relatorio de vencimentos.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);

            SQLFreeHandle(SQL_HANDLE_STMT, comando);
            fecharConexao(ambiente, conexao);
            return;
        }
    }

    printf("\n");
    printf("====================================\n");
    printf("   PRODUTOS PROXIMOS DO VENCIMENTO\n");
    printf("====================================\n");
    printf(
        "%-25s | %-6s | %6s | %-10s | %s\n",
        "Produto", "Lote", "Qtd", "Validade", "Status"
    );
    printf("------------------------------------------------------------\n");

    {
        int encontrou = 0;

        while (SQL_SUCCEEDED(SQLFetch(comando))) {

            char nome[101];
            SQLINTEGER idLote = 0;
            SQLINTEGER saldo = 0;
            char dataValidade[11];
            char status[30];

            encontrou = 1;

            SQLGetData(
                comando, 1, SQL_C_CHAR,
                nome, sizeof(nome), &indicador
            );

            SQLGetData(
                comando, 2, SQL_C_SLONG,
                &idLote, sizeof(idLote), &indicador
            );

            SQLGetData(
                comando, 3, SQL_C_SLONG,
                &saldo, sizeof(saldo), &indicador
            );

            SQLGetData(
                comando, 4, SQL_C_CHAR,
                dataValidade, sizeof(dataValidade), &indicador
            );

            SQLGetData(
                comando, 5, SQL_C_CHAR,
                status, sizeof(status), &indicador
            );

            printf(
                "%-25.25s | %-6d | %6d | %-10s | %s\n",
                nome,
                (int)idLote,
                (int)saldo,
                dataValidade,
                status
            );
        }

        if (!encontrou) {
            printf(
                "Nenhum lote vencido ou proximo do vencimento.\n"
            );
        }
    }

    printf("============================================================\n");

    SQLFreeHandle(SQL_HANDLE_STMT, comando);
    fecharConexao(ambiente, conexao);
}

/* ============================================================
   MOVIMENTACAO DE ESTOQUE
   ============================================================ */

void movimentacaoEstoque(void) {

    SQLHENV ambiente = SQL_NULL_HENV;
    SQLHDBC conexao = SQL_NULL_HDBC;
    SQLHSTMT comando = SQL_NULL_HSTMT;

    SQLRETURN resultado;

    SQLLEN indicador;

    if (!abrirConexao(&ambiente, &conexao)) {
        return;
    }

    comando = criarComando(conexao);

    if (comando == SQL_NULL_HSTMT) {
        fecharConexao(ambiente, conexao);
        return;
    }

    {
        SQLCHAR consulta[] =
            "SELECT "
            "CONVERT(varchar(10), m.Data_Movimento, 103) + ' ' + "
            "CONVERT(varchar(8), m.Data_Movimento, 108), "
            "pr.Nome, m.Tipo_Movimento, m.Quantidade, m.Observacao "
            "FROM dbo.MOVIMENTACAO_ESTOQUE m "
            "JOIN dbo.PRODUTO pr ON pr.Cod_Produto = m.FK_Cod_Produto "
            "ORDER BY m.Data_Movimento DESC, m.Id_Movimentacao DESC;";

        resultado = SQLExecDirect(comando, consulta, SQL_NTS);

        if (!SQL_SUCCEEDED(resultado)) {
            printf("\nErro ao gerar relatorio de movimentacao.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);

            SQLFreeHandle(SQL_HANDLE_STMT, comando);
            fecharConexao(ambiente, conexao);
            return;
        }
    }

    printf("\n");
    printf("====================================\n");
    printf("      MOVIMENTACAO DE ESTOQUE\n");
    printf("====================================\n");
    printf(
        "%-19s | %-20s | %-8s | %5s | %s\n",
        "Data", "Produto", "Tipo", "Qtd", "Descricao"
    );
    printf("--------------------------------------------------------------------\n");

    {
        int encontrou = 0;

        while (SQL_SUCCEEDED(SQLFetch(comando))) {

            char dataMovimento[21];
            char nome[101];
            char tipo[11];
            SQLINTEGER quantidade = 0;
            char observacao[201];

            encontrou = 1;

            SQLGetData(
                comando, 1, SQL_C_CHAR,
                dataMovimento, sizeof(dataMovimento), &indicador
            );

            SQLGetData(
                comando, 2, SQL_C_CHAR,
                nome, sizeof(nome), &indicador
            );

            SQLGetData(
                comando, 3, SQL_C_CHAR,
                tipo, sizeof(tipo), &indicador
            );

            SQLGetData(
                comando, 4, SQL_C_SLONG,
                &quantidade, sizeof(quantidade), &indicador
            );

            SQLGetData(
                comando, 5, SQL_C_CHAR,
                observacao, sizeof(observacao), &indicador
            );

            printf(
                "%-19s | %-20.20s | %-8s | %5d | %s\n",
                dataMovimento,
                nome,
                tipo,
                (int)quantidade,
                observacao
            );
        }

        if (!encontrou) {
            printf("Nenhuma movimentacao registrada.\n");
        }
    }

    printf("====================================================================\n");

    SQLFreeHandle(SQL_HANDLE_STMT, comando);
    fecharConexao(ambiente, conexao);
}
