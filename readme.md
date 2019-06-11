# FileTransfer-UDP

Autores:
- [Bruno Fernando Lopes](https://github.com/Briuor/) - 2017014669
- [Érick de Oliveira Teixeira](https://github.com/ErickOliveiraT) - 2017001437

O FileTransfer-UDP é um transferidor de arquivos entre clientes com o auxílio de um um servidor que intermedia a conexão de dois clientes, baseado na arquitetura P2P e no protocolo BitTorrent.
O protocolo utilizado foi o UDP, portanto, os artifícios para garantia de integridade e segurança foram implementados na camada de aplicação.

### Instruções para Execução
Os comandos dever ser utilizador na pasta raíz do projeto.
- Execução do Servidor (Terminal 1)
```sh
$ make
$ cd server
$ ./server
```
- Execução do Cliente Semeador (Terminal 2)
```sh
$ cd cliente_envia
$ ./client
```
- Execução do Cliente Baixador (Terminal 3)
```sh
$ cd cliente_baixa
$ ./client
```

Após executar o servidor e os dois clientes, o arquivo a ser baixado deve ser informado no terminal do cliente baixador.
(O arquivo deve estar previamente no diretório do cliente semeador).

### Informações Impressas sobre os Pacotes

As informações que são impressas no período de transferência dos arquivo, dizem respeito a cada um dos pacotes que são transferidos. São elas, respectivamente:

- Número de Sequência
- ACK
- Soma de Verificação

### Explicações sobre a Soma de Verificação

As operações são feitas sobre cada um dos bits do buffer de cada um dos pacotes transferidos, seguindo o seguinte pseudocódigo:

```sh
soma <- 0
para i de 0 a TAM_BUFFER:
    se buffer[i] = 1 (bin)
        soma += 2*i
    ou se buffer[i] = 0 (bin)
        soma += i
```
O resultado da soma de verificação é salvo dentro do própio pacote em uma variável inteira. Quando o pacote chega, o cliente baixador calcula a soma de verificação novamente, com o mesmo algoritmo, e compara com o resultado presente no pacote.
Caso o resultado não segue igual, ele solicita ao outro cliente que o mesmo pacote seja enviado novamente.