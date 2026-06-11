# Simulador de Rede de Distribuição de Água — São Cristóvão

Projeto prático de **Estrutura de Dados (Teoria de Grafos)** que simula uma rede de abastecimento urbano em C++ com renderização visual e interativa automática em HTML/SVG.

## 🚀 Funcionalidades Principais

- **Modelagem Avançada:** Representação por Lista de Adjacência para máxima eficiência de memória (Grafo Esparso).
- **Diagnóstico de Abastecimento (BFS):** Varredura em largura a partir da ETA para identificar pontos sem água ou em estado de emergência.
- **Integridade Estrutural (DFS):** Algoritmo recursivo para detectar a divisão da rede em componentes conexas isoladas.
- **Rotas de Contingência (Dijkstra Otimizado):** Cálculo imediato do caminho de menor tempo para locais críticos, utilizando uma **Fila de Prioridade (Min-Heap)** para alcançar complexidade $O(E \log V)$.
- **Simulação Dinâmica:** Opções para simular rompimento de tubulações, restauração de fluxo e edição de propriedades físicas (tempo e vazão em L/s) ao vivo no terminal.

## 🛠️ Tecnologias Utilizadas

- **Core/Backend:** C++ Nativizado (STL, `<queue>`, `<vector>`, `<fstream>`).
- **Interface Visual:** HTML5, CSS3 e SVG dinâmico injetado via file stream do C++.
- **Orquestração:** Windows API (`ShellExecuteA`) para automação de abertura do navegador Chrome.

## 💻 Como Executar

1. Abra o arquivo `.cpp` no **Visual Studio**.
2. Certifique-se de ter a carga de trabalho de "Desenvolvimento para desktop com C++" instalada.
3. Compile e execute pressionando **Ctrl + F5**.
4. Interaja pelo menu numérico do terminal. O mapa visual (`mapa_rede.html`) abrirá e atualizará de forma automática no seu navegador padrão a cada comando.

## 🗂️ Estrutura de Divisão Técnica (Apresentação)

- **Bloco 1 (Linhas 32-56):** Fundações do Modelo (`enum TipoVertice`, `struct Vertice`, `struct Aresta`).
- **Bloco 2 (Linhas 57-120):** Classe Grafo, Lista de Adjacência e Controle de Fluxo.
- **Bloco 3 (Linhas 121-169):** Algoritmo e Execução da Busca em Largura (BFS).
- **Bloco 4 (Linhas 170-219):** Algoritmo e Execução da Busca em Profundidade (DFS).
- **Bloco 5 (Linhas 220-272):** Inteligência e Otimização do Algoritmo de Dijkstra.
- **Bloco 6 (Linhas 280-480):** Motor de Renderização e Escrita do Arquivo HTML/SVG.
- **Bloco 7 (Linhas 504 ao Fim):** Instanciação da Cidade, Orquestração e Menu Principal.
