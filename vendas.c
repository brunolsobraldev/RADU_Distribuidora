#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <sql.h>
#include <sqlext.h>

#include "database.h"
#include "vendas.h"

#define MAX_ITENS 100
#define MAX_LOTES 200

/* ============================================================
   ESTRUTURAS
   ============================================================ */

typedef struct {
    int codProduto;
    char nome[101];
    int quantidade;
    double precoUnitario;
} ItemPedido;

typedef struct {
    int idLote;
    int saldo;
} LoteInfo;

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
   APOIO AOS ITENS DO PEDIDO
   ============================================================ */

static int indiceDoProduto(
    ItemPedido *itens,
    int totalItens,
    int codProduto) {

    int i;

    for (i = 0; i < totalItens; i++) {
        if (itens[i].codProduto == codProduto) {
            return i;
        }
    }

    return -1;
}

static int quantidadeJaSolicitada(
    ItemPedido *itens,
    int totalItens,
    int codProduto) {

    int indice = indiceDoProduto(itens, totalItens, codProduto);

    if (indice < 0) {
        return 0;
    }

    return itens[indice].quantidade;
}

/* ============================================================
   CADASTRAR CLIENTE (funcao interna)
   ============================================================ */

static int cadastrarCliente(void) {

    SQLHENV ambiente = SQL_NULL_HENV;
    SQLHDBC conexao = SQL_NULL_HDBC;
    SQLHSTMT comando = SQL_NULL_HSTMT;

    SQLRETURN resultado;

    int idCliente = 0;

    char nome[121];
    char cpf[15];
    char telefone[21];
    char email[121];
    char endereco[201];

    SQLLEN indicador;

    printf("\n");
    printf("====================================\n");
    printf("        CADASTRO DE CLIENTE\n");
    printf("====================================\n\n");

    lerTexto("Nome/Razao social: ", nome, sizeof(nome));

    if (nome[0] == '\0') {
        printf("\nErro: o nome e obrigatorio.\n");
        return 0;
    }

    lerTexto("CPF ou CNPJ (11 ou 14 digitos): ", cpf, sizeof(cpf));

    if (strlen(cpf) != 11 && strlen(cpf) != 14) {
        printf("\nErro: informe um CPF (11) ou CNPJ (14) valido.\n");
        return 0;
    }

    lerTexto("Telefone: ", telefone, sizeof(telefone));

    if (telefone[0] == '\0') {
        printf("\nErro: o telefone e obrigatorio.\n");
        return 0;
    }

    lerTexto("Email: ", email, sizeof(email));

    if (email[0] == '\0') {
        printf("\nErro: o email e obrigatorio.\n");
        return 0;
    }

    lerTexto("Endereco de entrega: ", endereco, sizeof(endereco));

    if (endereco[0] == '\0') {
        printf("\nErro: o endereco de entrega e obrigatorio.\n");
        return 0;
    }

    if (!abrirConexao(&ambiente, &conexao)) {
        return 0;
    }

    comando = criarComando(conexao);

    if (comando == SQL_NULL_HSTMT) {
        fecharConexao(ambiente, conexao);
        return 0;
    }

    {
        SQLCHAR sql[] =
            "INSERT INTO dbo.CLIENTE "
            "(Nome_Razao_Social, CPF_CNPJ, Telefone, Email, "
            "Endereco_Entrega) "
            "OUTPUT INSERTED.Id_Cliente "
            "VALUES (?, ?, ?, ?, ?);";

        SQLLEN tamanhoNome = SQL_NTS;
        SQLLEN tamanhoCpf = SQL_NTS;
        SQLLEN tamanhoTelefone = SQL_NTS;
        SQLLEN tamanhoEmail = SQL_NTS;
        SQLLEN tamanhoEndereco = SQL_NTS;

        resultado = SQLPrepare(comando, sql, SQL_NTS);

        if (!SQL_SUCCEEDED(resultado)) {
            printf("\nErro ao preparar cadastro do cliente.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);

            SQLFreeHandle(SQL_HANDLE_STMT, comando);
            fecharConexao(ambiente, conexao);
            return 0;
        }

        SQLBindParameter(
            comando, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR,
            120, 0, nome, sizeof(nome), &tamanhoNome
        );

        SQLBindParameter(
            comando, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR,
            14, 0, cpf, sizeof(cpf), &tamanhoCpf
        );

        SQLBindParameter(
            comando, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR,
            20, 0, telefone, sizeof(telefone), &tamanhoTelefone
        );

        SQLBindParameter(
            comando, 4, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR,
            120, 0, email, sizeof(email), &tamanhoEmail
        );

        SQLBindParameter(
            comando, 5, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR,
            200, 0, endereco, sizeof(endereco), &tamanhoEndereco
        );

        resultado = SQLExecute(comando);

        if (!SQL_SUCCEEDED(resultado)) {
            printf("\nErro ao cadastrar cliente.\n");
            printf("Verifique se o CPF/CNPJ ou o email ja estao cadastrados.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);

            SQLFreeHandle(SQL_HANDLE_STMT, comando);
            fecharConexao(ambiente, conexao);
            return 0;
        }

        resultado = SQLFetch(comando);

        if (!SQL_SUCCEEDED(resultado)) {
            printf("\nErro ao obter o codigo do cliente cadastrado.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);

            SQLFreeHandle(SQL_HANDLE_STMT, comando);
            fecharConexao(ambiente, conexao);
            return 0;
        }

        SQLGetData(
            comando, 1, SQL_C_SLONG,
            &idCliente, sizeof(idCliente), &indicador
        );
    }

    SQLFreeHandle(SQL_HANDLE_STMT, comando);
    fecharConexao(ambiente, conexao);

    printf("\nCliente cadastrado com sucesso! Codigo: %d\n", idCliente);

    return idCliente;
}

/* ============================================================
   CONSULTAS DE APOIO
   ============================================================ */

static int buscarCliente(
    SQLHDBC conexao,
    int codCliente,
    char *nome,
    size_t tamanho) {

    SQLHSTMT comando = criarComando(conexao);
    SQLRETURN resultado;

    SQLLEN indicador;
    SQLLEN tamanhoCodigo = sizeof(codCliente);

    if (comando == SQL_NULL_HSTMT) {
        return 0;
    }

    {
        SQLCHAR sql[] =
            "SELECT Nome_Razao_Social "
            "FROM dbo.CLIENTE "
            "WHERE Id_Cliente = ?;";

        resultado = SQLPrepare(comando, sql, SQL_NTS);

        if (!SQL_SUCCEEDED(resultado)) {
            printf("\nErro ao preparar consulta do cliente.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);

            SQLFreeHandle(SQL_HANDLE_STMT, comando);
            return 0;
        }

        SQLBindParameter(
            comando, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
            0, 0, &codCliente, sizeof(codCliente), &tamanhoCodigo
        );

        resultado = SQLExecute(comando);

        if (!SQL_SUCCEEDED(resultado)) {
            printf("\nErro ao consultar cliente.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);

            SQLFreeHandle(SQL_HANDLE_STMT, comando);
            return 0;
        }

        resultado = SQLFetch(comando);

        if (!SQL_SUCCEEDED(resultado)) {
            SQLFreeHandle(SQL_HANDLE_STMT, comando);
            return 0;
        }

        SQLGetData(
            comando, 1, SQL_C_CHAR,
            nome, (SQLLEN)tamanho, &indicador
        );
    }

    SQLFreeHandle(SQL_HANDLE_STMT, comando);

    return 1;
}

static int buscarProdutoEstoque(
    SQLHDBC conexao,
    int codProduto,
    char *nome,
    size_t tamanho,
    double *preco,
    int *disponivel) {

    SQLHSTMT comando = criarComando(conexao);
    SQLRETURN resultado;

    SQLLEN indicador;
    SQLLEN tamanhoCodigo = sizeof(codProduto);

    SQLINTEGER estoque = 0;

    if (comando == SQL_NULL_HSTMT) {
        return 0;
    }

    {
        SQLCHAR sql[] =
            "SELECT p.Nome, p.Preco_Venda_Unidade, "
            "ISNULL(e.Quantidade, 0) "
            "FROM dbo.PRODUTO p "
            "LEFT JOIN dbo.ESTOQUE e "
            "ON e.Cod_Produto = p.Cod_Produto "
            "WHERE p.Cod_Produto = ?;";

        resultado = SQLPrepare(comando, sql, SQL_NTS);

        if (!SQL_SUCCEEDED(resultado)) {
            printf("\nErro ao preparar consulta do produto.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);

            SQLFreeHandle(SQL_HANDLE_STMT, comando);
            return 0;
        }

        SQLBindParameter(
            comando, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
            0, 0, &codProduto, sizeof(codProduto), &tamanhoCodigo
        );

        resultado = SQLExecute(comando);

        if (!SQL_SUCCEEDED(resultado)) {
            printf("\nErro ao consultar produto.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);

            SQLFreeHandle(SQL_HANDLE_STMT, comando);
            return 0;
        }

        resultado = SQLFetch(comando);

        if (!SQL_SUCCEEDED(resultado)) {
            SQLFreeHandle(SQL_HANDLE_STMT, comando);
            return 0;
        }

        SQLGetData(
            comando, 1, SQL_C_CHAR,
            nome, (SQLLEN)tamanho, &indicador
        );

        SQLGetData(
            comando, 2, SQL_C_DOUBLE,
            preco, sizeof(*preco), &indicador
        );

        SQLGetData(
            comando, 3, SQL_C_SLONG,
            &estoque, sizeof(estoque), &indicador
        );
    }

    SQLFreeHandle(SQL_HANDLE_STMT, comando);

    *disponivel = (int)estoque;

    return 1;
}

/* ============================================================
   OPERACOES DA TRANSACAO DE VENDA
   ============================================================ */

static int inserirPedido(
    SQLHDBC conexao,
    int codCliente,
    double valorTotal,
    int *numPedido) {

    SQLHSTMT comando = criarComando(conexao);
    SQLRETURN resultado;

    SQLLEN indicador;
    SQLLEN tamanhoValor = sizeof(valorTotal);
    SQLLEN tamanhoCliente = sizeof(codCliente);

    if (comando == SQL_NULL_HSTMT) {
        return 0;
    }

    {
        SQLCHAR sql[] =
            "INSERT INTO dbo.PEDIDO "
            "(Data_Pedido, Status_Pedido, Valor_Total, FK_Id_Cliente) "
            "OUTPUT INSERTED.Num_Pedido "
            "VALUES (SYSDATETIME(), 'CONCLUIDO', ?, ?);";

        resultado = SQLPrepare(comando, sql, SQL_NTS);

        if (!SQL_SUCCEEDED(resultado)) {
            printf("\nErro ao preparar insercao do pedido.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);

            SQLFreeHandle(SQL_HANDLE_STMT, comando);
            return 0;
        }

        SQLBindParameter(
            comando, 1, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL,
            12, 2, &valorTotal, sizeof(valorTotal), &tamanhoValor
        );

        SQLBindParameter(
            comando, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
            0, 0, &codCliente, sizeof(codCliente), &tamanhoCliente
        );

        resultado = SQLExecute(comando);

        if (!SQL_SUCCEEDED(resultado)) {
            printf("\nErro ao inserir pedido.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);

            SQLFreeHandle(SQL_HANDLE_STMT, comando);
            return 0;
        }

        resultado = SQLFetch(comando);

        if (!SQL_SUCCEEDED(resultado)) {
            printf("\nErro ao obter o numero do pedido.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);

            SQLFreeHandle(SQL_HANDLE_STMT, comando);
            return 0;
        }

        SQLGetData(
            comando, 1, SQL_C_SLONG,
            numPedido, sizeof(*numPedido), &indicador
        );
    }

    SQLFreeHandle(SQL_HANDLE_STMT, comando);

    return 1;
}

static int atualizarSaldoLote(
    SQLHDBC conexao,
    int idLote,
    int quantidade) {

    SQLHSTMT comando = criarComando(conexao);
    SQLRETURN resultado;

    SQLLEN linhasAfetadas = 0;
    SQLLEN tamanhoQuantidade = sizeof(quantidade);
    SQLLEN tamanhoLote = sizeof(idLote);

    if (comando == SQL_NULL_HSTMT) {
        return 0;
    }

    {
        SQLCHAR sql[] =
            "UPDATE dbo.LOTE_ESTOQUE "
            "SET Saldo_Disponivel = Saldo_Disponivel - ? "
            "WHERE Id_Lote = ? AND Saldo_Disponivel >= ?;";

        resultado = SQLPrepare(comando, sql, SQL_NTS);

        if (!SQL_SUCCEEDED(resultado)) {
            printf("\nErro ao preparar baixa do lote.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);

            SQLFreeHandle(SQL_HANDLE_STMT, comando);
            return 0;
        }

        SQLBindParameter(
            comando, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
            0, 0, &quantidade, sizeof(quantidade), &tamanhoQuantidade
        );

        SQLBindParameter(
            comando, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
            0, 0, &idLote, sizeof(idLote), &tamanhoLote
        );

        SQLBindParameter(
            comando, 3, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
            0, 0, &quantidade, sizeof(quantidade), &tamanhoQuantidade
        );

        resultado = SQLExecute(comando);

        if (!SQL_SUCCEEDED(resultado) && resultado != SQL_NO_DATA) {
            printf("\nErro ao baixar lote.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);

            SQLFreeHandle(SQL_HANDLE_STMT, comando);
            return 0;
        }

        SQLRowCount(comando, &linhasAfetadas);
    }

    SQLFreeHandle(SQL_HANDLE_STMT, comando);

    return linhasAfetadas == 1;
}

static int inserirItemPedido(
    SQLHDBC conexao,
    int numPedido,
    int codProduto,
    int idLote,
    int quantidade,
    double precoUnitario) {

    SQLHSTMT comando = criarComando(conexao);
    SQLRETURN resultado;

    SQLLEN tamanhoPedido = sizeof(numPedido);
    SQLLEN tamanhoProduto = sizeof(codProduto);
    SQLLEN tamanhoLote = sizeof(idLote);
    SQLLEN tamanhoQuantidade = sizeof(quantidade);
    SQLLEN tamanhoPreco = sizeof(precoUnitario);

    if (comando == SQL_NULL_HSTMT) {
        return 0;
    }

    {
        SQLCHAR sql[] =
            "INSERT INTO dbo.ITENS_PEDIDO "
            "(FK_Num_Pedido, FK_Cod_Produto, FK_Id_Lote, "
            "Qtd_Vendida, Tipo_Venda, Preco_Praticado) "
            "VALUES (?, ?, ?, ?, 'UNIDADE', ?);";

        resultado = SQLPrepare(comando, sql, SQL_NTS);

        if (!SQL_SUCCEEDED(resultado)) {
            printf("\nErro ao preparar item do pedido.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);

            SQLFreeHandle(SQL_HANDLE_STMT, comando);
            return 0;
        }

        SQLBindParameter(
            comando, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
            0, 0, &numPedido, sizeof(numPedido), &tamanhoPedido
        );

        SQLBindParameter(
            comando, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
            0, 0, &codProduto, sizeof(codProduto), &tamanhoProduto
        );

        SQLBindParameter(
            comando, 3, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
            0, 0, &idLote, sizeof(idLote), &tamanhoLote
        );

        SQLBindParameter(
            comando, 4, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
            0, 0, &quantidade, sizeof(quantidade), &tamanhoQuantidade
        );

        SQLBindParameter(
            comando, 5, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL,
            10, 2, &precoUnitario, sizeof(precoUnitario), &tamanhoPreco
        );

        resultado = SQLExecute(comando);

        if (!SQL_SUCCEEDED(resultado)) {
            printf("\nErro ao inserir item do pedido.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);

            SQLFreeHandle(SQL_HANDLE_STMT, comando);
            return 0;
        }
    }

    SQLFreeHandle(SQL_HANDLE_STMT, comando);

    return 1;
}

static int inserirMovimentacaoSaida(
    SQLHDBC conexao,
    int codProduto,
    int idLote,
    int quantidade,
    int numPedido) {

    SQLHSTMT comando = criarComando(conexao);
    SQLRETURN resultado;

    char observacao[201];

    SQLLEN tamanhoProduto = sizeof(codProduto);
    SQLLEN tamanhoLote = sizeof(idLote);
    SQLLEN tamanhoQuantidade = sizeof(quantidade);
    SQLLEN tamanhoObservacao = SQL_NTS;

    if (comando == SQL_NULL_HSTMT) {
        return 0;
    }

    {
        SQLCHAR sql[] =
            "INSERT INTO dbo.MOVIMENTACAO_ESTOQUE "
            "(FK_Cod_Produto, FK_Id_Lote, Tipo_Movimento, "
            "Quantidade, Data_Movimento, Observacao) "
            "VALUES (?, ?, 'SAIDA', ?, SYSDATETIME(), ?);";

        snprintf(
            observacao,
            sizeof(observacao),
            "Venda pedido #%d",
            numPedido
        );

        resultado = SQLPrepare(comando, sql, SQL_NTS);

        if (!SQL_SUCCEEDED(resultado)) {
            printf("\nErro ao preparar movimentacao de saida.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);

            SQLFreeHandle(SQL_HANDLE_STMT, comando);
            return 0;
        }

        SQLBindParameter(
            comando, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
            0, 0, &codProduto, sizeof(codProduto), &tamanhoProduto
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
            printf("\nErro ao registrar movimentacao de saida.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);

            SQLFreeHandle(SQL_HANDLE_STMT, comando);
            return 0;
        }
    }

    SQLFreeHandle(SQL_HANDLE_STMT, comando);

    return 1;
}

static int atualizarEstoqueSaida(
    SQLHDBC conexao,
    int codProduto,
    int quantidade) {

    SQLHSTMT comando = criarComando(conexao);
    SQLRETURN resultado;

    SQLLEN linhasAfetadas = 0;
    SQLLEN tamanhoQuantidade = sizeof(quantidade);
    SQLLEN tamanhoProduto = sizeof(codProduto);

    if (comando == SQL_NULL_HSTMT) {
        return 0;
    }

    {
        SQLCHAR sql[] =
            "UPDATE dbo.ESTOQUE "
            "SET Quantidade = Quantidade - ? "
            "WHERE Cod_Produto = ? AND Quantidade >= ?;";

        resultado = SQLPrepare(comando, sql, SQL_NTS);

        if (!SQL_SUCCEEDED(resultado)) {
            printf("\nErro ao preparar baixa do estoque.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);

            SQLFreeHandle(SQL_HANDLE_STMT, comando);
            return 0;
        }

        SQLBindParameter(
            comando, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
            0, 0, &quantidade, sizeof(quantidade), &tamanhoQuantidade
        );

        SQLBindParameter(
            comando, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
            0, 0, &codProduto, sizeof(codProduto), &tamanhoProduto
        );

        SQLBindParameter(
            comando, 3, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
            0, 0, &quantidade, sizeof(quantidade), &tamanhoQuantidade
        );

        resultado = SQLExecute(comando);

        if (!SQL_SUCCEEDED(resultado) && resultado != SQL_NO_DATA) {
            printf("\nErro ao baixar estoque.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);

            SQLFreeHandle(SQL_HANDLE_STMT, comando);
            return 0;
        }

        SQLRowCount(comando, &linhasAfetadas);
    }

    SQLFreeHandle(SQL_HANDLE_STMT, comando);

    return linhasAfetadas == 1;
}

/*
    Baixa a quantidade vendida de um produto usando FEFO
    (primeiro o lote com validade mais proxima).

    Pode gerar mais de uma linha em ITENS_PEDIDO para o mesmo produto
    quando a quantidade consome mais de um lote.
*/
static int baixarEstoqueProduto(
    SQLHDBC conexao,
    int codProduto,
    int quantidade,
    double precoUnitario,
    int numPedido) {

    SQLHSTMT comando = SQL_NULL_HSTMT;
    SQLRETURN resultado;

    LoteInfo lotes[MAX_LOTES];
    int totalLotes = 0;

    int restante = quantidade;
    int i;

    SQLLEN indicador;
    SQLLEN tamanhoProduto = sizeof(codProduto);

    /* --------------------------------------------------------
       1. Ler os lotes disponiveis
       -------------------------------------------------------- */

    comando = criarComando(conexao);

    if (comando == SQL_NULL_HSTMT) {
        return 0;
    }

    {
        SQLCHAR sql[] =
            "SELECT Id_Lote, Saldo_Disponivel "
            "FROM dbo.LOTE_ESTOQUE "
            "WHERE FK_Cod_Produto = ? AND Saldo_Disponivel > 0 "
            "ORDER BY Data_Validade, Id_Lote;";

        resultado = SQLPrepare(comando, sql, SQL_NTS);

        if (!SQL_SUCCEEDED(resultado)) {
            printf("\nErro ao preparar consulta de lotes.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);

            SQLFreeHandle(SQL_HANDLE_STMT, comando);
            return 0;
        }

        SQLBindParameter(
            comando, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
            0, 0, &codProduto, sizeof(codProduto), &tamanhoProduto
        );

        resultado = SQLExecute(comando);

        if (!SQL_SUCCEEDED(resultado)) {
            printf("\nErro ao consultar lotes.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);

            SQLFreeHandle(SQL_HANDLE_STMT, comando);
            return 0;
        }

        while (SQL_SUCCEEDED(SQLFetch(comando))) {

            SQLINTEGER idLote = 0;
            SQLINTEGER saldo = 0;

            if (totalLotes >= MAX_LOTES) {
                printf("\nErro: limite de lotes por produto excedido.\n");

                SQLFreeHandle(SQL_HANDLE_STMT, comando);
                return 0;
            }

            SQLGetData(
                comando, 1, SQL_C_SLONG,
                &idLote, sizeof(idLote), &indicador
            );

            SQLGetData(
                comando, 2, SQL_C_SLONG,
                &saldo, sizeof(saldo), &indicador
            );

            lotes[totalLotes].idLote = (int)idLote;
            lotes[totalLotes].saldo = (int)saldo;

            totalLotes++;
        }
    }

    SQLFreeHandle(SQL_HANDLE_STMT, comando);
    comando = SQL_NULL_HSTMT;

    /* --------------------------------------------------------
       2. Consumir os lotes em FEFO
       -------------------------------------------------------- */

    for (i = 0; i < totalLotes && restante > 0; i++) {

        int usar = restante < lotes[i].saldo
            ? restante
            : lotes[i].saldo;

        if (usar <= 0) {
            continue;
        }

        if (!atualizarSaldoLote(conexao, lotes[i].idLote, usar)) {
            printf("\nErro ao baixar o lote #%d.\n", lotes[i].idLote);
            return 0;
        }

        if (!inserirItemPedido(
                conexao,
                numPedido,
                codProduto,
                lotes[i].idLote,
                usar,
                precoUnitario)) {

            return 0;
        }

        if (!inserirMovimentacaoSaida(
                conexao,
                codProduto,
                lotes[i].idLote,
                usar,
                numPedido)) {

            return 0;
        }

        restante -= usar;
    }

    if (restante > 0) {
        printf(
            "\nErro: lotes insuficientes para o produto %d "
            "(faltam %d unidades).\n",
            codProduto,
            restante
        );

        return 0;
    }

    /* --------------------------------------------------------
       3. Baixar o saldo consolidado do estoque
       -------------------------------------------------------- */

    if (!atualizarEstoqueSaida(conexao, codProduto, quantidade)) {
        printf(
            "\nErro: estoque insuficiente para o produto %d "
            "no momento da baixa.\n",
            codProduto
        );

        return 0;
    }

    return 1;
}

/* ============================================================
   MENU DE VENDAS
   ============================================================ */

void menuVendas(void) {

    int opcao;

    do {

        printf("\n");
        printf("====================================\n");
        printf("          MENU DE VENDAS\n");
        printf("====================================\n");
        printf("1 - Registrar pedido\n");
        printf("2 - Consultar pedido\n");
        printf("0 - Voltar\n");
        printf("====================================\n");

        if (!lerInteiro("\nEscolha uma opcao: ", &opcao)) {
            opcao = -1;
        }

        switch (opcao) {

            case 1:
                registrarPedido();
                break;

            case 2:
                consultarPedido();
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
   REGISTRAR PEDIDO
   ============================================================ */

void registrarPedido(void) {

    SQLHENV ambiente = SQL_NULL_HENV;
    SQLHDBC conexao = SQL_NULL_HDBC;

    SQLRETURN resultado;

    int codCliente = 0;
    char nomeCliente[121] = "";

    ItemPedido itens[MAX_ITENS];
    int totalItens = 0;

    double valorTotal = 0.0;

    int numPedido = 0;
    int transacao = 0;
    int opcao = 0;

    int i;

    printf("\n");
    printf("====================================\n");
    printf("          NOVO PEDIDO\n");
    printf("====================================\n\n");

    if (!lerInteiro(
            "Codigo do cliente (0 para cadastrar novo): ",
            &codCliente)) {

        printf("\nErro: codigo de cliente invalido.\n");
        return;
    }

    if (codCliente < 0) {
        printf("\nErro: codigo de cliente invalido.\n");
        return;
    }

    if (codCliente == 0) {

        codCliente = cadastrarCliente();

        if (codCliente <= 0) {
            printf("\nPedido cancelado: cliente nao cadastrado.\n");
            return;
        }
    }

    if (!abrirConexao(&ambiente, &conexao)) {
        return;
    }

    if (!buscarCliente(
            conexao,
            codCliente,
            nomeCliente,
            sizeof(nomeCliente))) {

        printf(
            "\nErro: cliente com codigo %d nao encontrado.\n",
            codCliente
        );

        fecharConexao(ambiente, conexao);
        return;
    }

    printf("\nCliente: %s\n", nomeCliente);
    printf("\nDigite 0 no codigo do produto para finalizar o pedido.\n");

    while (1) {

        int codProduto = 0;
        int quantidade = 0;
        int disponivel = 0;
        int jaSolicitada = 0;
        int indice = -1;

        double preco = 0.0;
        char nomeProduto[101] = "";

        if (!lerInteiro("\nCodigo do produto: ", &codProduto)) {
            printf("\nErro: codigo de produto invalido.\n");
            continue;
        }

        if (codProduto == 0) {
            break;
        }

        if (codProduto < 0) {
            printf("\nErro: codigo de produto invalido.\n");
            continue;
        }

        if (totalItens >= MAX_ITENS) {
            printf("\nLimite de itens por pedido atingido.\n");
            break;
        }

        if (!buscarProdutoEstoque(
                conexao,
                codProduto,
                nomeProduto,
                sizeof(nomeProduto),
                &preco,
                &disponivel)) {

            printf(
                "\nErro: produto com codigo %d nao encontrado.\n",
                codProduto
            );

            continue;
        }

        jaSolicitada = quantidadeJaSolicitada(
            itens,
            totalItens,
            codProduto
        );

        disponivel -= jaSolicitada;

        printf("\nProduto: %s\n", nomeProduto);
        printf("Preco unitario: R$ %.2f\n", preco);
        printf("Estoque disponivel: %d\n", disponivel);

        if (!lerInteiro("Quantidade: ", &quantidade) ||
            quantidade <= 0) {

            printf("\nErro: a quantidade deve ser maior que zero.\n");
            continue;
        }

        if (quantidade > disponivel) {
            printf("\nEstoque insuficiente.\n");
            printf("\nDisponivel: %d\n", disponivel);
            printf("Solicitado: %d\n", quantidade);

            continue;
        }

        indice = indiceDoProduto(itens, totalItens, codProduto);

        if (indice >= 0) {

            itens[indice].quantidade += quantidade;

        } else {

            itens[totalItens].codProduto = codProduto;

            snprintf(
                itens[totalItens].nome,
                sizeof(itens[totalItens].nome),
                "%s",
                nomeProduto
            );

            itens[totalItens].quantidade = quantidade;
            itens[totalItens].precoUnitario = preco;

            totalItens++;
        }

        valorTotal += quantidade * preco;

        printf(
            "\nItem adicionado. Subtotal: R$ %.2f\n",
            quantidade * preco
        );
    }

    if (totalItens == 0) {
        printf("\nNenhum item foi adicionado. Pedido cancelado.\n");
        fecharConexao(ambiente, conexao);
        return;
    }

    /* --------------------------------------------------------
       Resumo
       -------------------------------------------------------- */

    printf("\n");
    printf("====================================\n");
    printf("          RESUMO DO PEDIDO\n");
    printf("====================================\n\n");

    printf("Cliente: %s\n\n", nomeCliente);

    printf("%-30s %6s %12s\n", "Produto", "Qtd", "Total");
    printf("------------------------------------\n");

    for (i = 0; i < totalItens; i++) {
        printf(
            "%-30.30s %6d %12.2f\n",
            itens[i].nome,
            itens[i].quantidade,
            itens[i].quantidade * itens[i].precoUnitario
        );
    }

    printf("------------------------------------\n");
    printf("TOTAL: %29.2f\n", valorTotal);

    printf("\n1 - Confirmar\n");
    printf("2 - Cancelar\n");

    if (!lerInteiro("Opcao: ", &opcao) || opcao != 1) {
        printf("\nPedido cancelado.\n");
        fecharConexao(ambiente, conexao);
        return;
    }

    /* --------------------------------------------------------
       Transacao de venda
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

    if (!inserirPedido(conexao, codCliente, valorTotal, &numPedido)) {
        goto erro;
    }

    for (i = 0; i < totalItens; i++) {

        if (!baixarEstoqueProduto(
                conexao,
                itens[i].codProduto,
                itens[i].quantidade,
                itens[i].precoUnitario,
                numPedido)) {

            goto erro;
        }
    }

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
    printf("  Pedido #%d registrado com sucesso!\n", numPedido);
    printf("====================================\n");
    printf("Total: R$ %.2f\n", valorTotal);

    return;

erro:

    printf("\nErro ao registrar pedido. Operacao cancelada.\n");

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
   CONSULTAR PEDIDO
   ============================================================ */

void consultarPedido(void) {

    SQLHENV ambiente = SQL_NULL_HENV;
    SQLHDBC conexao = SQL_NULL_HDBC;
    SQLHSTMT comando = SQL_NULL_HSTMT;

    SQLRETURN resultado;

    int numPedido = 0;

    char nomeCliente[121] = "";
    char dataPedido[11] = "";
    char status[21] = "";

    double valorTotal = 0.0;

    SQLLEN indicador;
    SQLLEN tamanhoPedido = sizeof(numPedido);

    printf("\n");
    printf("====================================\n");
    printf("         CONSULTAR PEDIDO\n");
    printf("====================================\n\n");

    if (!lerInteiro("Codigo do pedido: ", &numPedido) ||
        numPedido <= 0) {

        printf("\nErro: codigo de pedido invalido.\n");
        return;
    }

    if (!abrirConexao(&ambiente, &conexao)) {
        return;
    }

    /* --------------------------------------------------------
       Cabecalho do pedido
       -------------------------------------------------------- */

    comando = criarComando(conexao);

    if (comando == SQL_NULL_HSTMT) {
        fecharConexao(ambiente, conexao);
        return;
    }

    {
        SQLCHAR sql[] =
            "SELECT c.Nome_Razao_Social, "
            "CONVERT(varchar(10), p.Data_Pedido, 103), "
            "p.Status_Pedido, p.Valor_Total "
            "FROM dbo.PEDIDO p "
            "JOIN dbo.CLIENTE c ON c.Id_Cliente = p.FK_Id_Cliente "
            "WHERE p.Num_Pedido = ?;";

        resultado = SQLPrepare(comando, sql, SQL_NTS);

        if (!SQL_SUCCEEDED(resultado)) {
            printf("\nErro ao preparar consulta do pedido.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);

            SQLFreeHandle(SQL_HANDLE_STMT, comando);
            fecharConexao(ambiente, conexao);
            return;
        }

        SQLBindParameter(
            comando, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
            0, 0, &numPedido, sizeof(numPedido), &tamanhoPedido
        );

        resultado = SQLExecute(comando);

        if (!SQL_SUCCEEDED(resultado)) {
            printf("\nErro ao consultar pedido.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);

            SQLFreeHandle(SQL_HANDLE_STMT, comando);
            fecharConexao(ambiente, conexao);
            return;
        }

        resultado = SQLFetch(comando);

        if (!SQL_SUCCEEDED(resultado)) {
            printf(
                "\nPedido com codigo %d nao encontrado.\n",
                numPedido
            );

            SQLFreeHandle(SQL_HANDLE_STMT, comando);
            fecharConexao(ambiente, conexao);
            return;
        }

        SQLGetData(
            comando, 1, SQL_C_CHAR,
            nomeCliente, sizeof(nomeCliente), &indicador
        );

        SQLGetData(
            comando, 2, SQL_C_CHAR,
            dataPedido, sizeof(dataPedido), &indicador
        );

        SQLGetData(
            comando, 3, SQL_C_CHAR,
            status, sizeof(status), &indicador
        );

        SQLGetData(
            comando, 4, SQL_C_DOUBLE,
            &valorTotal, sizeof(valorTotal), &indicador
        );
    }

    SQLFreeHandle(SQL_HANDLE_STMT, comando);
    comando = SQL_NULL_HSTMT;

    printf("\n");
    printf("====================================\n");
    printf("          PEDIDO #%d\n", numPedido);
    printf("====================================\n\n");
    printf("Cliente: %s\n", nomeCliente);
    printf("Data: %s\n", dataPedido);
    printf("Status: %s\n\n", status);

    /* --------------------------------------------------------
       Itens do pedido
       -------------------------------------------------------- */

    comando = criarComando(conexao);

    if (comando == SQL_NULL_HSTMT) {
        fecharConexao(ambiente, conexao);
        return;
    }

    {
        SQLCHAR sql[] =
            "SELECT pr.Nome, SUM(i.Qtd_Vendida), "
            "SUM(i.Qtd_Vendida * i.Preco_Praticado) "
            "FROM dbo.ITENS_PEDIDO i "
            "JOIN dbo.PRODUTO pr ON pr.Cod_Produto = i.FK_Cod_Produto "
            "WHERE i.FK_Num_Pedido = ? "
            "GROUP BY pr.Nome "
            "ORDER BY pr.Nome;";

        resultado = SQLPrepare(comando, sql, SQL_NTS);

        if (!SQL_SUCCEEDED(resultado)) {
            printf("\nErro ao preparar consulta dos itens.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);

            SQLFreeHandle(SQL_HANDLE_STMT, comando);
            fecharConexao(ambiente, conexao);
            return;
        }

        SQLBindParameter(
            comando, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
            0, 0, &numPedido, sizeof(numPedido), &tamanhoPedido
        );

        resultado = SQLExecute(comando);

        if (!SQL_SUCCEEDED(resultado)) {
            printf("\nErro ao consultar itens do pedido.\n");
            imprimirErroODBC(SQL_HANDLE_STMT, comando);

            SQLFreeHandle(SQL_HANDLE_STMT, comando);
            fecharConexao(ambiente, conexao);
            return;
        }
    }

    printf("%-30s %6s %12s\n", "Produto", "Qtd", "Total");
    printf("------------------------------------\n");

    while (SQL_SUCCEEDED(SQLFetch(comando))) {

        char nomeProduto[101];
        SQLINTEGER quantidade = 0;
        double subtotal = 0.0;

        SQLGetData(
            comando, 1, SQL_C_CHAR,
            nomeProduto, sizeof(nomeProduto), &indicador
        );

        SQLGetData(
            comando, 2, SQL_C_SLONG,
            &quantidade, sizeof(quantidade), &indicador
        );

        SQLGetData(
            comando, 3, SQL_C_DOUBLE,
            &subtotal, sizeof(subtotal), &indicador
        );

        printf(
            "%-30.30s %6d %12.2f\n",
            nomeProduto,
            (int)quantidade,
            subtotal
        );
    }

    printf("------------------------------------\n");
    printf("TOTAL: %29.2f\n", valorTotal);

    SQLFreeHandle(SQL_HANDLE_STMT, comando);
    fecharConexao(ambiente, conexao);
}
