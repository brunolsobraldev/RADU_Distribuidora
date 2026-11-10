#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <sql.h>
#include <sqlext.h>

#include "database.h"
#include "estoque.h"

/* ============================================================
   LEITURA SEGURA DE ENTRADA
   ============================================================ */

static void lerTexto(const char *prompt, char *destino, size_t tamanho) {

    int caractere;
    size_t posicao = 0;

    printf("%s", prompt);
    fflush(stdout);

    /* Descarta espacos e quebras de linha pendentes. */
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

    /* Descarta o restante da linha. */
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

static int anoBissexto(int ano) {
    return (ano % 4 == 0 && ano % 100 != 0) || (ano % 400 == 0);
}

static int diasNoMes(int mes, int ano) {

    static const int dias[] =
        {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (mes < 1 || mes > 12) {
        return 0;
    }

    if (mes == 2 && anoBissexto(ano)) {
        return 29;
    }

    return dias[mes - 1];
}

/*
    Le uma data no formato dd/mm/aaaa e converte para aaaa-mm-dd,
    que e o formato usado nas consultas SQL (CONVERT(date, ?, 23)).
*/
static int lerData(const char *prompt, char *destino, size_t tamanho) {

    char linha[32];

    int dia;
    int mes;
    int ano;

    lerTexto(prompt, linha, sizeof(linha));

    if (sscanf(linha, "%d/%d/%d", &dia, &mes, &ano) != 3) {
        return 0;
    }

    if (ano < 1900 || ano > 9999) {
        return 0;
    }

    if (mes < 1 || mes > 12) {
        return 0;
    }

    if (dia < 1 || dia > diasNoMes(mes, ano)) {
        return 0;
    }

    snprintf(destino, tamanho, "%04d-%02d-%02d", ano, mes, dia);

    return 1;
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
   MENU DE ESTOQUE
   ============================================================ */

void menuEstoque(void) {

    int opcao;

    do {

        printf("\n");
        printf("====================================\n");
        printf("           MENU DE ESTOQUE\n");
        printf("====================================\n");
        printf("1 - Registrar entrada\n");
        printf("2 - Consultar estoque\n");
        printf("3 - Consultar lotes\n");
        printf("4 - Validar estoque\n");
        printf("0 - Voltar\n");
        printf("====================================\n");

        if (!lerInteiro("\nEscolha uma opcao: ", &opcao)) {
            opcao = -1;
        }

        switch (opcao) {

            case 1:
                registrarEntrada();
                break;

            case 2:
                consultarEstoque();
                break;

            case 3:
                consultarLotes();
                break;

            case 4:
                validarEstoque();
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
   REGISTRAR ENTRADA
   ============================================================ */

void registrarEntrada(void) {

    SQLHENV ambiente = SQL_NULL_HENV;
    SQLHDBC conexao = SQL_NULL_HDBC;
    SQLHSTMT comando = SQL_NULL_HSTMT;

    SQLRETURN resultado;

    int codProduto = 0;
    int quantidade = 0;
    int idLote = 0;
    int transacao = 0;

    char nomeProduto[101] = "";
    char dataEntrada[11] = "";
    char dataValidade[11] = "";
    char observacao[201] = "";

    SQLLEN indicador;

    printf("\n");
    printf("====================================\n");
    printf("       REGISTRAR ENTRADA\n");
    printf("====================================\n\n");

    if (!lerInteiro("Codigo do produto: ", &codProduto) ||
        codProduto <= 0) {

        printf("\nErro: codigo de produto invalido.\n");
        return;
    }

    if (!abrirConexao(&ambiente, &conexao)) {
        return;
    }

    /* --------------------------------------------------------
       Verificar se o produto existe
       -------------------------------------------------------- */

    comando = criarComando(conexao);

    if (comando == SQL_NULL_HSTMT) {
        fecharConexao(ambiente, conexao);
        return;
    }

    {
        SQLCHAR sql[] =
            "SELECT Nome "
            "FROM dbo.PRODUTO "
            "WHERE Cod_Produto = ?;";

        SQLLEN tamanhoCodigo = sizeof(codProduto);

        resultado = SQLPrepare(comando, sql, SQL_NTS);

        if (!SQL_SUCCEEDED(resultado)) {
            printf("\nErro ao preparar consulta do produto.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);

            SQLFreeHandle(SQL_HANDLE_STMT, comando);
            fecharConexao(ambiente, conexao);
            return;
        }

        resultado = SQLBindParameter(
            comando,
            1,
            SQL_PARAM_INPUT,
            SQL_C_SLONG,
            SQL_INTEGER,
            0,
            0,
            &codProduto,
            sizeof(codProduto),
            &tamanhoCodigo
        );

        if (!SQL_SUCCEEDED(resultado)) {
            printf("\nErro ao associar codigo do produto.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);

            SQLFreeHandle(SQL_HANDLE_STMT, comando);
            fecharConexao(ambiente, conexao);
            return;
        }

        resultado = SQLExecute(comando);

        if (!SQL_SUCCEEDED(resultado)) {
            printf("\nErro ao consultar produto.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);

            SQLFreeHandle(SQL_HANDLE_STMT, comando);
            fecharConexao(ambiente, conexao);
            return;
        }

        resultado = SQLFetch(comando);

        if (!SQL_SUCCEEDED(resultado)) {
            printf(
                "\nErro: produto com codigo %d nao encontrado.\n",
                codProduto
            );

            SQLFreeHandle(SQL_HANDLE_STMT, comando);
            fecharConexao(ambiente, conexao);
            return;
        }

        SQLGetData(
            comando,
            1,
            SQL_C_CHAR,
            nomeProduto,
            sizeof(nomeProduto),
            &indicador
        );
    }

    SQLFreeHandle(SQL_HANDLE_STMT, comando);
    comando = SQL_NULL_HSTMT;

    printf("\nProduto: %s\n\n", nomeProduto);

    /* --------------------------------------------------------
       Dados da entrada
       -------------------------------------------------------- */

    if (!lerInteiro("Quantidade: ", &quantidade) || quantidade <= 0) {
        printf("\nErro: a quantidade deve ser maior que zero.\n");
        fecharConexao(ambiente, conexao);
        return;
    }

    if (!lerData(
            "Data de entrada (dd/mm/aaaa): ",
            dataEntrada,
            sizeof(dataEntrada))) {

        printf("\nErro: data de entrada invalida.\n");
        fecharConexao(ambiente, conexao);
        return;
    }

    if (!lerData(
            "Data de validade (dd/mm/aaaa): ",
            dataValidade,
            sizeof(dataValidade))) {

        printf("\nErro: data de validade invalida.\n");
        fecharConexao(ambiente, conexao);
        return;
    }

    if (strcmp(dataValidade, dataEntrada) < 0) {
        printf(
            "\nErro: a data de validade nao pode ser anterior "
            "a data de entrada.\n"
        );

        fecharConexao(ambiente, conexao);
        return;
    }

    /* --------------------------------------------------------
       Transacao
       -------------------------------------------------------- */

    resultado = SQLSetConnectAttr(
        conexao,
        SQL_ATTR_AUTOCOMMIT,
        (SQLPOINTER)(SQLLEN)SQL_AUTOCOMMIT_OFF,
        0
    );

    if (!SQL_SUCCEEDED(resultado)) {
        printf("\nErro ao iniciar transacao.\n");
        imprimirErroODBC(SQL_HANDLE_DBC, conexao);

        fecharConexao(ambiente, conexao);
        return;
    }

    transacao = 1;

    /* --------------------------------------------------------
       Inserir lote
       -------------------------------------------------------- */

    comando = criarComando(conexao);

    if (comando == SQL_NULL_HSTMT) {
        goto erro;
    }

    {
        SQLCHAR sql[] =
            "INSERT INTO dbo.LOTE_ESTOQUE "
            "(FK_Cod_Produto, Data_Recebimento, Data_Validade, "
            "Quantidade_Recebida, Saldo_Disponivel) "
            "OUTPUT INSERTED.Id_Lote "
            "VALUES (?, CONVERT(date, ?, 23), CONVERT(date, ?, 23), ?, ?);";

        SQLLEN tamanhoCodigo = sizeof(codProduto);
        SQLLEN tamanhoDataEntrada = SQL_NTS;
        SQLLEN tamanhoDataValidade = SQL_NTS;
        SQLLEN tamanhoQuantidade = sizeof(quantidade);

        resultado = SQLPrepare(comando, sql, SQL_NTS);

        if (!SQL_SUCCEEDED(resultado)) {
            printf("\nErro ao preparar insercao do lote.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);
            goto erro;
        }

        SQLBindParameter(
            comando, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
            0, 0, &codProduto, sizeof(codProduto), &tamanhoCodigo
        );

        SQLBindParameter(
            comando, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR,
            10, 0, dataEntrada, sizeof(dataEntrada), &tamanhoDataEntrada
        );

        SQLBindParameter(
            comando, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR,
            10, 0, dataValidade, sizeof(dataValidade), &tamanhoDataValidade
        );

        SQLBindParameter(
            comando, 4, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
            0, 0, &quantidade, sizeof(quantidade), &tamanhoQuantidade
        );

        SQLBindParameter(
            comando, 5, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
            0, 0, &quantidade, sizeof(quantidade), &tamanhoQuantidade
        );

        resultado = SQLExecute(comando);

        if (!SQL_SUCCEEDED(resultado)) {
            printf("\nErro ao inserir lote.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);
            goto erro;
        }

        resultado = SQLFetch(comando);

        if (!SQL_SUCCEEDED(resultado)) {
            printf("\nErro ao obter o codigo do lote inserido.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);
            goto erro;
        }

        SQLGetData(
            comando,
            1,
            SQL_C_SLONG,
            &idLote,
            sizeof(idLote),
            &indicador
        );
    }

    SQLFreeHandle(SQL_HANDLE_STMT, comando);
    comando = SQL_NULL_HSTMT;

    /* --------------------------------------------------------
       Atualizar estoque (INSERT ou UPDATE)
       -------------------------------------------------------- */

    comando = criarComando(conexao);

    if (comando == SQL_NULL_HSTMT) {
        goto erro;
    }

    {
        SQLCHAR sql[] =
            "UPDATE dbo.ESTOQUE "
            "SET Quantidade = Quantidade + ? "
            "WHERE Cod_Produto = ?;";

        SQLLEN tamanhoQuantidade = sizeof(quantidade);
        SQLLEN tamanhoCodigo = sizeof(codProduto);
        SQLLEN linhasAfetadas = 0;

        resultado = SQLPrepare(comando, sql, SQL_NTS);

        if (!SQL_SUCCEEDED(resultado)) {
            printf("\nErro ao preparar atualizacao do estoque.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);
            goto erro;
        }

        SQLBindParameter(
            comando, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
            0, 0, &quantidade, sizeof(quantidade), &tamanhoQuantidade
        );

        SQLBindParameter(
            comando, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
            0, 0, &codProduto, sizeof(codProduto), &tamanhoCodigo
        );

        resultado = SQLExecute(comando);

        if (!SQL_SUCCEEDED(resultado) && resultado != SQL_NO_DATA) {
            printf("\nErro ao atualizar estoque.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);
            goto erro;
        }

        SQLRowCount(comando, &linhasAfetadas);

        SQLFreeHandle(SQL_HANDLE_STMT, comando);
        comando = SQL_NULL_HSTMT;

        if (linhasAfetadas == 0) {

            comando = criarComando(conexao);

            if (comando == SQL_NULL_HSTMT) {
                goto erro;
            }

            {
                SQLCHAR sqlInsert[] =
                    "INSERT INTO dbo.ESTOQUE "
                    "(Cod_Produto, Quantidade) "
                    "VALUES (?, ?);";

                resultado = SQLPrepare(comando, sqlInsert, SQL_NTS);

                if (!SQL_SUCCEEDED(resultado)) {
                    printf("\nErro ao preparar insercao do estoque.\n");
                    imprimirErroODBC(SQL_HANDLE_STMT, comando);
                    goto erro;
                }

                SQLBindParameter(
                    comando, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
                    0, 0, &codProduto, sizeof(codProduto), &tamanhoCodigo
                );

                SQLBindParameter(
                    comando, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
                    0, 0, &quantidade, sizeof(quantidade), &tamanhoQuantidade
                );

                resultado = SQLExecute(comando);

                if (!SQL_SUCCEEDED(resultado)) {
                    printf("\nErro ao inserir registro de estoque.\n");
                    imprimirErroODBC(SQL_HANDLE_STMT, comando);
                    goto erro;
                }
            }
        }
    }

    if (comando != SQL_NULL_HSTMT) {
        SQLFreeHandle(SQL_HANDLE_STMT, comando);
        comando = SQL_NULL_HSTMT;
    }

    /* --------------------------------------------------------
       Registrar movimentacao
       -------------------------------------------------------- */

    comando = criarComando(conexao);

    if (comando == SQL_NULL_HSTMT) {
        goto erro;
    }

    {
        SQLCHAR sql[] =
            "INSERT INTO dbo.MOVIMENTACAO_ESTOQUE "
            "(FK_Cod_Produto, FK_Id_Lote, Tipo_Movimento, "
            "Quantidade, Data_Movimento, Observacao) "
            "VALUES (?, ?, 'ENTRADA', ?, SYSDATETIME(), ?);";

        SQLLEN tamanhoCodigo = sizeof(codProduto);
        SQLLEN tamanhoLote = sizeof(idLote);
        SQLLEN tamanhoQuantidade = sizeof(quantidade);
        SQLLEN tamanhoObservacao = SQL_NTS;

        snprintf(
            observacao,
            sizeof(observacao),
            "Entrada de estoque - lote #%d",
            idLote
        );

        resultado = SQLPrepare(comando, sql, SQL_NTS);

        if (!SQL_SUCCEEDED(resultado)) {
            printf("\nErro ao preparar movimentacao de estoque.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);
            goto erro;
        }

        SQLBindParameter(
            comando, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
            0, 0, &codProduto, sizeof(codProduto), &tamanhoCodigo
        );

        SQLBindParameter(
            comando, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
            0, 0, &idLote, sizeof(idLote), &tamanhoLote
        );

        SQLBindParameter(
            comando, 3, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
            0, 0, &quantidade, sizeof(quantidade), &tamanhoQuantidade
        );

        SQLBindParameter(
            comando, 4, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR,
            200, 0, observacao, sizeof(observacao), &tamanhoObservacao
        );

        resultado = SQLExecute(comando);

        if (!SQL_SUCCEEDED(resultado)) {
            printf("\nErro ao registrar movimentacao de estoque.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);
            goto erro;
        }
    }

    SQLFreeHandle(SQL_HANDLE_STMT, comando);
    comando = SQL_NULL_HSTMT;

    /* --------------------------------------------------------
       Commit
       -------------------------------------------------------- */

    resultado = SQLEndTran(SQL_HANDLE_DBC, conexao, SQL_COMMIT);

    if (!SQL_SUCCEEDED(resultado)) {
        printf("\nErro ao confirmar a transacao.\n");
        imprimirErroODBC(SQL_HANDLE_DBC, conexao);
        goto erro;
    }

    SQLSetConnectAttr(
        conexao,
        SQL_ATTR_AUTOCOMMIT,
        (SQLPOINTER)(SQLLEN)SQL_AUTOCOMMIT_ON,
        0
    );

    transacao = 0;

    fecharConexao(ambiente, conexao);

    printf("\n====================================\n");
    printf("  Entrada registrada com sucesso!\n");
    printf("====================================\n");
    printf("Lote: #%d\n", idLote);
    printf("Produto: %s\n", nomeProduto);
    printf("Quantidade: %d\n", quantidade);

    return;

erro:

    printf("\nErro ao registrar entrada. Operacao cancelada.\n");

    if (comando != SQL_NULL_HSTMT) {
        SQLFreeHandle(SQL_HANDLE_STMT, comando);
        comando = SQL_NULL_HSTMT;
    }

    if (transacao) {
        SQLEndTran(SQL_HANDLE_DBC, conexao, SQL_ROLLBACK);

        SQLSetConnectAttr(
            conexao,
            SQL_ATTR_AUTOCOMMIT,
            (SQLPOINTER)(SQLLEN)SQL_AUTOCOMMIT_ON,
            0
        );
    }

    fecharConexao(ambiente, conexao);
}

/* ============================================================
   CONSULTAR ESTOQUE
   ============================================================ */

void consultarEstoque(void) {

    SQLHENV ambiente = SQL_NULL_HENV;
    SQLHDBC conexao = SQL_NULL_HDBC;
    SQLHSTMT comando = SQL_NULL_HSTMT;

    SQLRETURN resultado;

    SQLINTEGER codProduto;
    SQLINTEGER quantidade;

    char nome[101];

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
            "SELECT p.Cod_Produto, p.Nome, ISNULL(e.Quantidade, 0) "
            "FROM dbo.PRODUTO p "
            "LEFT JOIN dbo.ESTOQUE e "
            "ON e.Cod_Produto = p.Cod_Produto "
            "ORDER BY p.Cod_Produto;";

        resultado = SQLExecDirect(comando, consulta, SQL_NTS);

        if (!SQL_SUCCEEDED(resultado)) {
            printf("\nErro ao consultar estoque.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);

            SQLFreeHandle(SQL_HANDLE_STMT, comando);
            fecharConexao(ambiente, conexao);
            return;
        }
    }

    printf("\n");
    printf("====================================================\n");
    printf("                  ESTOQUE ATUAL\n");
    printf("====================================================\n");
    printf("%-8s | %-30s | %10s\n", "Codigo", "Produto", "Quantidade");
    printf("----------------------------------------------------\n");

    {
        int encontrou = 0;

        while (SQL_SUCCEEDED(SQLFetch(comando))) {

            encontrou = 1;

            SQLGetData(
                comando, 1, SQL_C_SLONG,
                &codProduto, sizeof(codProduto), &indicador
            );

            SQLGetData(
                comando, 2, SQL_C_CHAR,
                nome, sizeof(nome), &indicador
            );

            SQLGetData(
                comando, 3, SQL_C_SLONG,
                &quantidade, sizeof(quantidade), &indicador
            );

            printf(
                "%-8d | %-30.30s | %10d\n",
                (int)codProduto,
                nome,
                (int)quantidade
            );
        }

        if (!encontrou) {
            printf("Nenhum produto cadastrado.\n");
        }
    }

    printf("====================================================\n");

    SQLFreeHandle(SQL_HANDLE_STMT, comando);
    fecharConexao(ambiente, conexao);
}

/* ============================================================
   CONSULTAR LOTES
   ============================================================ */

void consultarLotes(void) {

    SQLHENV ambiente = SQL_NULL_HENV;
    SQLHDBC conexao = SQL_NULL_HDBC;
    SQLHSTMT comando = SQL_NULL_HSTMT;

    SQLRETURN resultado;

    SQLINTEGER idLote;
    SQLINTEGER quantidadeRecebida;
    SQLINTEGER saldoDisponivel;

    char nome[101];
    char dataRecebimento[11];
    char dataValidade[11];
    char status[30];

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
            "SELECT p.Nome, l.Id_Lote, l.Quantidade_Recebida, "
            "l.Saldo_Disponivel, "
            "CONVERT(varchar(10), l.Data_Recebimento, 103), "
            "CONVERT(varchar(10), l.Data_Validade, 103), "
            "CASE "
            "  WHEN l.Data_Validade < CAST(SYSDATETIME() AS date) "
            "       THEN 'VENCIDO' "
            "  WHEN l.Data_Validade <= DATEADD(day, 30, "
            "       CAST(SYSDATETIME() AS date)) "
            "       THEN 'PROXIMO DO VENCIMENTO' "
            "  ELSE 'VALIDO' "
            "END "
            "FROM dbo.LOTE_ESTOQUE l "
            "JOIN dbo.PRODUTO p ON p.Cod_Produto = l.FK_Cod_Produto "
            "ORDER BY l.Data_Validade, l.Id_Lote;";

        resultado = SQLExecDirect(comando, consulta, SQL_NTS);

        if (!SQL_SUCCEEDED(resultado)) {
            printf("\nErro ao consultar lotes.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);

            SQLFreeHandle(SQL_HANDLE_STMT, comando);
            fecharConexao(ambiente, conexao);
            return;
        }
    }

    printf("\n");
    printf("====================================\n");
    printf("              LOTES\n");
    printf("====================================\n");

    {
        int encontrou = 0;

        while (SQL_SUCCEEDED(SQLFetch(comando))) {

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
                &quantidadeRecebida,
                sizeof(quantidadeRecebida),
                &indicador
            );

            SQLGetData(
                comando, 4, SQL_C_SLONG,
                &saldoDisponivel,
                sizeof(saldoDisponivel),
                &indicador
            );

            SQLGetData(
                comando, 5, SQL_C_CHAR,
                dataRecebimento,
                sizeof(dataRecebimento),
                &indicador
            );

            SQLGetData(
                comando, 6, SQL_C_CHAR,
                dataValidade,
                sizeof(dataValidade),
                &indicador
            );

            SQLGetData(
                comando, 7, SQL_C_CHAR,
                status,
                sizeof(status),
                &indicador
            );

            printf("\nProduto: %s\n", nome);
            printf("Lote: #%d\n", (int)idLote);
            printf("Quantidade recebida: %d\n", (int)quantidadeRecebida);
            printf("Saldo disponivel: %d\n", (int)saldoDisponivel);
            printf("Entrada: %s\n", dataRecebimento);
            printf("Validade: %s\n", dataValidade);
            printf("Status: %s\n", status);
            printf("------------------------------------\n");
        }

        if (!encontrou) {
            printf("\nNenhum lote cadastrado.\n");
        }
    }

    SQLFreeHandle(SQL_HANDLE_STMT, comando);
    fecharConexao(ambiente, conexao);
}

/* ============================================================
   VALIDAR ESTOQUE
   ============================================================ */

void validarEstoque(void) {

    SQLHENV ambiente = SQL_NULL_HENV;
    SQLHDBC conexao = SQL_NULL_HDBC;
    SQLHSTMT comando = SQL_NULL_HSTMT;

    SQLRETURN resultado;

    SQLINTEGER semEstoque = 0;
    SQLINTEGER negativas = 0;
    SQLINTEGER vencidos = 0;
    SQLINTEGER lotesInvalidos = 0;
    SQLINTEGER divergencias = 0;

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
            "(SELECT COUNT(*) "
            " FROM dbo.PRODUTO p "
            " LEFT JOIN dbo.ESTOQUE e "
            " ON e.Cod_Produto = p.Cod_Produto "
            " WHERE ISNULL(e.Quantidade, 0) = 0), "
            "(SELECT COUNT(*) "
            " FROM dbo.ESTOQUE "
            " WHERE Quantidade < 0), "
            "(SELECT COUNT(*) "
            " FROM dbo.LOTE_ESTOQUE "
            " WHERE Data_Validade < CAST(SYSDATETIME() AS date) "
            " AND Saldo_Disponivel > 0), "
            "(SELECT COUNT(*) "
            " FROM dbo.LOTE_ESTOQUE "
            " WHERE Saldo_Disponivel < 0 "
            " OR Saldo_Disponivel > Quantidade_Recebida), "
            "(SELECT COUNT(*) "
            " FROM dbo.PRODUTO p "
            " LEFT JOIN dbo.ESTOQUE e "
            " ON e.Cod_Produto = p.Cod_Produto "
            " OUTER APPLY ( "
            "   SELECT SUM(l.Saldo_Disponivel) AS Total "
            "   FROM dbo.LOTE_ESTOQUE l "
            "   WHERE l.FK_Cod_Produto = p.Cod_Produto "
            " ) s "
            " WHERE ISNULL(e.Quantidade, 0) <> ISNULL(s.Total, 0));";

        resultado = SQLExecDirect(comando, consulta, SQL_NTS);

        if (!SQL_SUCCEEDED(resultado)) {
            printf("\nErro ao validar estoque.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);

            SQLFreeHandle(SQL_HANDLE_STMT, comando);
            fecharConexao(ambiente, conexao);
            return;
        }

        resultado = SQLFetch(comando);

        if (!SQL_SUCCEEDED(resultado)) {
            printf("\nErro ao obter resultado da validacao.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);

            SQLFreeHandle(SQL_HANDLE_STMT, comando);
            fecharConexao(ambiente, conexao);
            return;
        }

        SQLGetData(
            comando, 1, SQL_C_SLONG,
            &semEstoque, sizeof(semEstoque), &indicador
        );

        SQLGetData(
            comando, 2, SQL_C_SLONG,
            &negativas, sizeof(negativas), &indicador
        );

        SQLGetData(
            comando, 3, SQL_C_SLONG,
            &vencidos, sizeof(vencidos), &indicador
        );

        SQLGetData(
            comando, 4, SQL_C_SLONG,
            &lotesInvalidos, sizeof(lotesInvalidos), &indicador
        );

        SQLGetData(
            comando, 5, SQL_C_SLONG,
            &divergencias, sizeof(divergencias), &indicador
        );
    }

    SQLFreeHandle(SQL_HANDLE_STMT, comando);
    fecharConexao(ambiente, conexao);

    printf("\n");
    printf("====================================\n");
    printf("        VALIDACAO DO ESTOQUE\n");
    printf("====================================\n");
    printf("Produtos sem estoque: %d\n", (int)semEstoque);
    printf("Quantidades negativas: %d\n", (int)negativas);
    printf("Lotes vencidos: %d\n", (int)vencidos);
    printf("Lotes com quantidade invalida: %d\n", (int)lotesInvalidos);
    printf("Divergencias entre estoque e lotes: %d\n", (int)divergencias);
    printf("====================================\n");

    if (semEstoque == 0 &&
        negativas == 0 &&
        vencidos == 0 &&
        lotesInvalidos == 0 &&
        divergencias == 0) {

        printf("\nEstoque validado com sucesso.\n");

    } else {

        printf("\nForam encontrados problemas no estoque.\n");
    }
}
