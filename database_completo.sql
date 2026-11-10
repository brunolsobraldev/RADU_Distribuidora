/*
    RADU Distribuidora
    Script COMPLETO de criacao da estrutura do banco.

    Use este arquivo para montar o banco em OUTRA maquina.
    Ele cria as 8 tabelas do sistema, com chaves, checks e FKs.

    IMPORTANTE:
    - Nao usa DROP TABLE, TRUNCATE TABLE nem DELETE.
    - Cada objeto so e criado se ainda nao existir (IF ... IS NULL).
    - Cria apenas a ESTRUTURA (sem dados).
    - Para levar tambem os dados, use backup/restore (.bak) do SQL Server.
*/

USE RADU_Distribuidora;
GO

SET NOCOUNT ON;
GO

/* ============================================================
   1. FORNECEDOR
   ============================================================ */

IF OBJECT_ID(N'dbo.FORNECEDOR', N'U') IS NULL
BEGIN
    CREATE TABLE dbo.FORNECEDOR
    (
        CNPJ          CHAR(14)     NOT NULL,
        Razao_Social  VARCHAR(120) NOT NULL,
        Nome_Fantasia VARCHAR(100) NOT NULL,
        Telefone      VARCHAR(20)  NOT NULL,
        Email         VARCHAR(120) NOT NULL,
        Contato_Nome  VARCHAR(100) NOT NULL,
        Endereco      VARCHAR(200) NOT NULL,

        CONSTRAINT PK_FORNECEDOR
            PRIMARY KEY (CNPJ),

        CONSTRAINT UQ_FORNECEDOR_EMAIL
            UNIQUE (Email)
    );

    PRINT 'Tabela dbo.FORNECEDOR criada.';
END
ELSE
BEGIN
    PRINT 'Tabela dbo.FORNECEDOR ja existe.';
END;
GO

/* ============================================================
   2. CLIENTE
   ============================================================ */

IF OBJECT_ID(N'dbo.CLIENTE', N'U') IS NULL
BEGIN
    CREATE TABLE dbo.CLIENTE
    (
        Id_Cliente        INT IDENTITY(1,1) NOT NULL,
        Nome_Razao_Social VARCHAR(120) NOT NULL,
        CPF_CNPJ          VARCHAR(14)  NOT NULL,
        Telefone          VARCHAR(20)  NOT NULL,
        Email             VARCHAR(120) NOT NULL,
        Endereco_Entrega  VARCHAR(200) NOT NULL,

        CONSTRAINT PK_CLIENTE
            PRIMARY KEY (Id_Cliente),

        CONSTRAINT UQ_CLIENTE_CPF_CNPJ
            UNIQUE (CPF_CNPJ),

        CONSTRAINT UQ_CLIENTE_EMAIL
            UNIQUE (Email)
    );

    PRINT 'Tabela dbo.CLIENTE criada.';
END
ELSE
BEGIN
    PRINT 'Tabela dbo.CLIENTE ja existe.';
END;
GO

/* ============================================================
   3. PRODUTO
   ============================================================ */

IF OBJECT_ID(N'dbo.PRODUTO', N'U') IS NULL
BEGIN
    CREATE TABLE dbo.PRODUTO
    (
        Cod_Produto         INT IDENTITY(1,1) NOT NULL,
        Nome                VARCHAR(100)  NOT NULL,
        Volume              VARCHAR(20)   NOT NULL,
        Tipo_Recipiente     VARCHAR(30)   NOT NULL,
        Categoria           VARCHAR(50)   NOT NULL,
        Preco_Custo_Unidade DECIMAL(10,2) NOT NULL,
        Preco_Custo_Pacote  DECIMAL(10,2) NOT NULL,
        Preco_Venda_Unidade DECIMAL(10,2) NOT NULL,
        Preco_Venda_Pacote  DECIMAL(10,2) NOT NULL,
        FK_CNPJ_Fornecedor  CHAR(14)      NOT NULL,

        CONSTRAINT PK_PRODUTO
            PRIMARY KEY (Cod_Produto),

        CONSTRAINT FK_PRODUTO_FORNECEDOR
            FOREIGN KEY (FK_CNPJ_Fornecedor)
            REFERENCES dbo.FORNECEDOR (CNPJ),

        CONSTRAINT CK_PRODUTO_CUSTO_UNIDADE
            CHECK (Preco_Custo_Unidade >= 0),

        CONSTRAINT CK_PRODUTO_CUSTO_PACOTE
            CHECK (Preco_Custo_Pacote >= 0),

        CONSTRAINT CK_PRODUTO_VENDA_UNIDADE
            CHECK (Preco_Venda_Unidade >= 0),

        CONSTRAINT CK_PRODUTO_VENDA_PACOTE
            CHECK (Preco_Venda_Pacote >= 0)
    );

    PRINT 'Tabela dbo.PRODUTO criada.';
END
ELSE
BEGIN
    PRINT 'Tabela dbo.PRODUTO ja existe.';
END;
GO

/* ============================================================
   4. ESTOQUE (saldo consolidado por produto)
   ============================================================ */

IF OBJECT_ID(N'dbo.ESTOQUE', N'U') IS NULL
BEGIN
    CREATE TABLE dbo.ESTOQUE
    (
        Cod_Estoque INT IDENTITY(1,1) NOT NULL,
        Cod_Produto INT NOT NULL,
        Quantidade  INT NOT NULL
            CONSTRAINT DF_ESTOQUE_QUANTIDADE DEFAULT (0),

        CONSTRAINT PK_ESTOQUE
            PRIMARY KEY (Cod_Estoque),

        CONSTRAINT UQ_ESTOQUE_PRODUTO
            UNIQUE (Cod_Produto),

        CONSTRAINT FK_ESTOQUE_PRODUTO
            FOREIGN KEY (Cod_Produto)
            REFERENCES dbo.PRODUTO (Cod_Produto),

        CONSTRAINT CK_ESTOQUE_QUANTIDADE
            CHECK (Quantidade >= 0)
    );

    PRINT 'Tabela dbo.ESTOQUE criada.';
END
ELSE
BEGIN
    PRINT 'Tabela dbo.ESTOQUE ja existe.';
END;
GO

/* ============================================================
   5. LOTE_ESTOQUE
   ============================================================ */

IF OBJECT_ID(N'dbo.LOTE_ESTOQUE', N'U') IS NULL
BEGIN
    CREATE TABLE dbo.LOTE_ESTOQUE
    (
        Id_Lote              INT IDENTITY(1,1) NOT NULL,
        FK_Cod_Produto       INT  NOT NULL,
        Data_Recebimento     DATE NOT NULL,
        Data_Validade        DATE NOT NULL,
        Quantidade_Recebida  INT  NOT NULL,
        Saldo_Disponivel     INT  NOT NULL,

        CONSTRAINT PK_LOTE_ESTOQUE
            PRIMARY KEY (Id_Lote),

        /* Necessaria para as FKs compostas de ITENS_PEDIDO e
           MOVIMENTACAO_ESTOQUE (FK_Id_Lote + FK_Cod_Produto). */
        CONSTRAINT UQ_LOTE_ID_PRODUTO
            UNIQUE (Id_Lote, FK_Cod_Produto),

        CONSTRAINT FK_LOTE_PRODUTO
            FOREIGN KEY (FK_Cod_Produto)
            REFERENCES dbo.PRODUTO (Cod_Produto),

        CONSTRAINT CK_LOTE_QUANTIDADE_RECEBIDA
            CHECK (Quantidade_Recebida > 0),

        CONSTRAINT CK_LOTE_SALDO
            CHECK (Saldo_Disponivel >= 0
                   AND Saldo_Disponivel <= Quantidade_Recebida),

        CONSTRAINT CK_LOTE_DATAS
            CHECK (Data_Validade >= Data_Recebimento)
    );

    PRINT 'Tabela dbo.LOTE_ESTOQUE criada.';
END
ELSE
BEGIN
    PRINT 'Tabela dbo.LOTE_ESTOQUE ja existe.';
END;
GO

/* ============================================================
   6. PEDIDO
   ============================================================ */

IF OBJECT_ID(N'dbo.PEDIDO', N'U') IS NULL
BEGIN
    CREATE TABLE dbo.PEDIDO
    (
        Num_Pedido    INT IDENTITY(1,1) NOT NULL,
        Data_Pedido   DATETIME2      NOT NULL,
        Status_Pedido VARCHAR(20)    NOT NULL,
        Valor_Total   DECIMAL(12,2)  NOT NULL,
        FK_Id_Cliente INT            NOT NULL,

        CONSTRAINT PK_PEDIDO
            PRIMARY KEY (Num_Pedido),

        CONSTRAINT FK_PEDIDO_CLIENTE
            FOREIGN KEY (FK_Id_Cliente)
            REFERENCES dbo.CLIENTE (Id_Cliente),

        CONSTRAINT CK_PEDIDO_STATUS
            CHECK (Status_Pedido IN
                   ('ABERTO', 'PAGO', 'CONCLUIDO', 'CANCELADO')),

        CONSTRAINT CK_PEDIDO_VALOR
            CHECK (Valor_Total >= 0)
    );

    PRINT 'Tabela dbo.PEDIDO criada.';
END
ELSE
BEGIN
    PRINT 'Tabela dbo.PEDIDO ja existe.';
END;
GO

/* ============================================================
   7. ITENS_PEDIDO
   ============================================================ */

IF OBJECT_ID(N'dbo.ITENS_PEDIDO', N'U') IS NULL
BEGIN
    CREATE TABLE dbo.ITENS_PEDIDO
    (
        Id_Item_Pedido  INT IDENTITY(1,1) NOT NULL,
        FK_Num_Pedido   INT           NOT NULL,
        FK_Cod_Produto  INT           NOT NULL,
        FK_Id_Lote      INT           NOT NULL,
        Qtd_Vendida     INT           NOT NULL,
        Tipo_Venda      VARCHAR(10)   NOT NULL,
        Preco_Praticado DECIMAL(10,2) NOT NULL,

        CONSTRAINT PK_ITENS_PEDIDO
            PRIMARY KEY (Id_Item_Pedido),

        CONSTRAINT FK_ITEM_PEDIDO
            FOREIGN KEY (FK_Num_Pedido)
            REFERENCES dbo.PEDIDO (Num_Pedido),

        CONSTRAINT FK_ITEM_PRODUTO
            FOREIGN KEY (FK_Cod_Produto)
            REFERENCES dbo.PRODUTO (Cod_Produto),

        /* Garante que o lote pertence ao produto informado. */
        CONSTRAINT FK_ITEM_LOTE_PRODUTO
            FOREIGN KEY (FK_Id_Lote, FK_Cod_Produto)
            REFERENCES dbo.LOTE_ESTOQUE (Id_Lote, FK_Cod_Produto),

        CONSTRAINT CK_ITEM_QUANTIDADE
            CHECK (Qtd_Vendida > 0),

        CONSTRAINT CK_ITEM_TIPO_VENDA
            CHECK (Tipo_Venda IN ('UNIDADE', 'PACOTE')),

        CONSTRAINT CK_ITEM_PRECO
            CHECK (Preco_Praticado >= 0)
    );

    PRINT 'Tabela dbo.ITENS_PEDIDO criada.';
END
ELSE
BEGIN
    PRINT 'Tabela dbo.ITENS_PEDIDO ja existe.';
END;
GO

/* ============================================================
   8. MOVIMENTACAO_ESTOQUE
   ============================================================ */

IF OBJECT_ID(N'dbo.MOVIMENTACAO_ESTOQUE', N'U') IS NULL
BEGIN
    CREATE TABLE dbo.MOVIMENTACAO_ESTOQUE
    (
        Id_Movimentacao INT IDENTITY(1,1) NOT NULL,
        FK_Cod_Produto  INT          NOT NULL,
        FK_Id_Lote      INT          NOT NULL,
        Tipo_Movimento  VARCHAR(10)  NOT NULL,
        Quantidade      INT          NOT NULL,
        Data_Movimento  DATETIME2    NOT NULL,
        Observacao      VARCHAR(200) NOT NULL,

        CONSTRAINT PK_MOVIMENTACAO_ESTOQUE
            PRIMARY KEY (Id_Movimentacao),

        /* Garante que o lote pertence ao produto informado. */
        CONSTRAINT FK_MOVIMENTACAO_LOTE_PRODUTO
            FOREIGN KEY (FK_Id_Lote, FK_Cod_Produto)
            REFERENCES dbo.LOTE_ESTOQUE (Id_Lote, FK_Cod_Produto),

        CONSTRAINT CK_MOVIMENTACAO_TIPO
            CHECK (Tipo_Movimento IN ('ENTRADA', 'SAIDA', 'AJUSTE')),

        CONSTRAINT CK_MOVIMENTACAO_QUANTIDADE
            CHECK (Quantidade > 0)
    );

    PRINT 'Tabela dbo.MOVIMENTACAO_ESTOQUE criada.';
END
ELSE
BEGIN
    PRINT 'Tabela dbo.MOVIMENTACAO_ESTOQUE ja existe.';
END;
GO

/* ============================================================
   INDICE PARA CONSULTA DE LOTES POR PRODUTO E VALIDADE
   ============================================================ */

IF NOT EXISTS
(
    SELECT 1
    FROM sys.indexes
    WHERE name = N'IX_LOTE_ESTOQUE_PRODUTO_VALIDADE'
      AND object_id = OBJECT_ID(N'dbo.LOTE_ESTOQUE')
)
BEGIN
    CREATE INDEX IX_LOTE_ESTOQUE_PRODUTO_VALIDADE
        ON dbo.LOTE_ESTOQUE (FK_Cod_Produto, Data_Validade);

    PRINT 'Indice IX_LOTE_ESTOQUE_PRODUTO_VALIDADE criado.';
END
ELSE
BEGIN
    PRINT 'Indice IX_LOTE_ESTOQUE_PRODUTO_VALIDADE ja existe.';
END;
GO

/* ============================================================
   VERIFICACAO FINAL
   ============================================================ */

PRINT '--------------------------------------------';
PRINT 'Tabelas encontradas no banco:';
SELECT name AS Tabela
FROM sys.tables
ORDER BY name;
PRINT '--------------------------------------------';
PRINT 'Estrutura completa verificada com sucesso.';
GO
