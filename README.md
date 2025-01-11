# Escalonador de Prioridades Estáticas Unix

```
                            _____  .__ ___.                                                              
  __________               /  _  \ |  |\_ |__ _____                                                      
 /  ___/  _ \    ______   /  /_\  \|  | | __ \\__  \                                                     
 \___ (  <_> )  /_____/  /    |    \  |_| \_\ \/ __ \_                                                   
/____  >____/            \____|__  /____/___  (____  /                                                   
     \/                          \/         \/     \/                                                    

```

Este programa implementa um escalonador que gerencia processos em um ambiente Unix pela política Round Robin, utilizando até 4 filas de prioridade (0, 1, 2, 3) e um quantum específico. Ele simula um ambiente multicore, escalonando processos de acordo com suas prioridades e tempos de início.

---

## Compilação

### Opção 1: Utilizando o Makefile


   ```bash
   make
   ```

Isso irá compilar o programa e gerar o executável `escalona`, além de compilar todos os processos de teste dentro do diretório `procsess`.

### Opção 2: Compilação Manual


   ```bash
   gcc -o escalona scheduler.c
   ```

## Execução

   ```bash
   ./escalona <numero_de_cores> <quantum> <arquivo_de_entrada>
   ```

   Exemplo:

   ```bash
   ./escalona 4 2 entrada.txt
   ```
Dois arquivos de entrada de teste já estão disponíveis no mesmo diretório do escalonador.

3. **Formato do Arquivo de Entrada:**
   O arquivo de entrada deve seguir o formato:

   ```
   <id> <executable> <start_time> <priority>
   ```

   Exemplo:

   ```
   1 teste20 0 2
   2 teste10 0 0
   3 teste30 20 0
   4 teste10 15 1
   ```

---

## Funcionalidades

- Gerencia processos em 4 filas de prioridade.
- Suporte a round-robin para cada fila.
- Utiliza semáforos para gerenciar o acesso aos cores.
- Monitora processos via pipes para comunicação interprocessos.
- Gera um relatório final com tempo de turnaround e execução de cada processo.

---

### Relatório Final

O programa imprime:

- Ordem de execução dos processos.
- Tempo de turnaround de cada processo.
- Tempo médio de turnaround.

---

## Observações

- Ainda não foi possível implementar a funcionalidade de múltiplos cores. A lógica de escalonamento está implementada, mas a execução paralela ainda não. (Em desenvolvimento)
