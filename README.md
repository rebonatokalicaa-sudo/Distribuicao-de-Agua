# Simulador de Rede de Distribuição de Água — São Cristóvão

Projeto prático de **Estrutura de Dados (Teoria de Grafos)** que simula uma rede de abastecimento urbano em C++ com renderização visual e interativa automática em HTML/SVG.

## 🚀 Funcionalidades Principais

- **Modelagem Avançada:** Representação por Lista de Adjacência para máxima eficiência de memória (Grafo Esparso).
- **Diagnóstico (BFS):** Varredura em largura a partir da ETA para identificar pontos desabastecidos.
- **Integridade (DFS):** Algoritmo recursivo para detectar a fragmentação da rede em componentes isoladas.
- **Contingência (Dijkstra):** Rota de menor tempo para locais críticos via **Fila de Prioridade (Min-Heap)** em $O(E \log V)$.
- **Simulação Ativa:** Rompimento, restauração e edição de propriedades físicas (tempo e vazão em L/s) ao vivo.

## 📸 Demonstração do Sistema

### 🛠️ Interface do Terminal (Painel de Controle C++)

![Interface do Terminal C++](<img width="744" height="710" alt="image" src="https://github.com/user-attachments/assets/f9e94e77-0ca2-4106-9043-8b8122d44dd6" />)

Console nativo em CPU para controle operacional da malha através de menu numérico. O destaque é a **Opção 12**, que edita pesos (tempo) e capacidades (vazão) de canos ao vivo, forçando o recálculo imediato do sistema antes dos diagnósticos estruturais.

### 💧 Interface Gráfica Dinâmica (Monitor HTML/SVG)

![Monitor de Rede HTML/SVG](<img width="1586" height="756" alt="image" src="https://github.com/user-attachments/assets/e1690367-770d-4301-a41e-643d5deae6d3" />
)


Visualização síncrona gerada de forma autônoma pelo C++ via arquivo `.html`. Exibe o estado lógico da memória RAM em tempo real: nós ativos validados pelo BFS, pulsação de emergência em locais críticos desabastecidos, linhas roxas com o caminho mínimo do Dijkstra e tracejados vermelhos indicando canos rompidos.

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
- **Bloco 5 (Linhas 220-272):** Inteligência e Otimização do Algoritmo de Dijkstra (Min-Heap).
- **Bloco 6 (Linhas 280-480):** Motor de Renderização e Escrita do Arquivo HTML/SVG.
- **Bloco 7 (Linhas 504 ao Fim):** Instanciação da Cidade, Orquestração e Menu Principal.
