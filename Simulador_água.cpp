/*
 ==============================================================
   SIMULADOR DE REDE DE DISTRIBUICAO DE AGUA
   Cidade: Sao Cristovao
   Disciplina: Estrutura de Dados - Grafos em C++
   Integrantes: Eduardo Bravo RA:52421070, Lucas Giovani Steves RA:52420131, Kaique Rebonato RA:52522487
 ==============================================================
   Como funciona:
     1. Rode pelo Visual Studio (Ctrl+F5)
     2. Interaja pelo terminal
     3. A cada acao, o Chrome abre/atualiza com o mapa visual
 ==============================================================
*/

#include <iostream>
#include <vector>
#include <queue>
#include <limits>
#include <string>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cmath>
#include <thread>
#include <chrono>
#include <windows.h>
#undef max
#undef min

using namespace std;
// ############################################################
//  BLOCO 1 — FUNDACOES DO MODELO
//  enum TipoVertice + struct Vertice + struct Aresta
//  Apresentador: Steves
// ============================================================
//  ENUM
// ============================================================
enum TipoVertice { FONTE, RESERVATORIO, CRITICO, BAIRRO };

// ============================================================
//  Estruturas
// ============================================================
struct Vertice {
    int         id;
    string      nome;
    string      descricao;
    TipoVertice tipo;
    double x, y;   // posicao no mapa HTML (0.0 a 1.0)
};

struct Aresta {
    int    destino;
    int    peso;
    string tipo;
    int    capacidade;
    bool   ativo;
    int    id;
};
// ############################################################
//  BLOCO 2 — CLASSE GRAFO E GERENCIAMENTO DA REDE
//  lista de adjacencia + adicionar/remover/restaurar arestas
//  Apresentador(a): Steves
// ############################################################

// ============================================================
//  CLASS GRAFO
// ============================================================
class Grafo {
private:
    int proximo_id;

public:
    vector<Vertice>        vertices;
    vector<vector<Aresta>> adj;
    string ultima_acao;
    string ultimo_resultado;
    vector<int> ultimo_caminho;
    vector<bool> ultimo_alcance;

    Grafo() : proximo_id(0) {}

    int total() const { return (int)vertices.size(); }

    void add_vertice(const string& nome, const string& desc,
        TipoVertice tipo, double x, double y) {
        int id = (int)vertices.size();
        vertices.push_back({ id, nome, desc, tipo, x, y });
        adj.push_back({});
        ultimo_alcance.push_back(true);
    }

    void add_aresta(int u, int v, int peso, int cap,
        const string& tipo = "tubulacao") {
        if (u < 0 || u >= total() || v < 0 || v >= total() || u == v) return;
        int id = proximo_id++;
        adj[u].push_back({ v, peso, tipo, cap, true, id });
        adj[v].push_back({ u, peso, tipo, cap, true, id });
    }

    bool rem_aresta(int u, int v) {
        for (auto& a : adj[u]) {
            if (a.destino == v && a.ativo) {
                int aid = a.id;
                a.ativo = false;
                for (auto& b : adj[v]) if (b.id == aid) b.ativo = false;
                return true;
            }
        }
        return false;
    }

    bool restaurar_aresta(int u, int v) {
        for (auto& a : adj[u]) {
            if (a.destino == v && !a.ativo) {
                int aid = a.id;
                a.ativo = true;
                for (auto& b : adj[v]) if (b.id == aid) b.ativo = true;
                return true;
            }
        }
        return false;
    }
    // ############################################################
    //  BLOCO 3 — DIAGNOSTICO DE FLUXO (BFS)
    //  verificacao de abastecimento a partir da ETA
    // Apresentador: Steves
    // ############################################################

    void restaurar_todas() {
        for (auto& lista : adj)
            for (auto& a : lista) a.ativo = true;
    }

    // ----------------------------------------------------------
    //  BFS
    // ----------------------------------------------------------
    vector<bool> bfs(int origem = 0) {
        int V = total();
        vector<bool> visitado(V, false);
        queue<int> fila;
        visitado[origem] = true;
        fila.push(origem);
        while (!fila.empty()) {
            int u = fila.front(); fila.pop();
            for (const auto& a : adj[u])
                if (a.ativo && !visitado[a.destino]) {
                    visitado[a.destino] = true;
                    fila.push(a.destino);
                }
        }
        ultimo_alcance = visitado;
        return visitado;
    }

    void executar_bfs() {
        cout << "\n===========================================================\n";
        cout << "   BFS - BUSCA EM LARGURA\n";
        cout << "===========================================================\n";

        animacao("  Executando BFS");
        vector<bool> visitado = bfs(0);

        ultima_acao = "BFS - Verificacao de Abastecimento";
        ultimo_resultado = "";
        ultimo_caminho.clear();

        bool alerta = false;
        for (int i = 1; i < total(); i++) {
            string status = visitado[i] ? "[OK] COM AGUA" : "[!!] SEM AGUA";
            cout << "  " << left << setw(13) << vertices[i].nome << " " << status;
            if (!visitado[i] && vertices[i].tipo == CRITICO) {
                cout << " <<< EMERGENCIA >>>";
                alerta = true;
            }
            cout << "\n";
            ultimo_resultado += vertices[i].nome + ": " +
                (visitado[i] ? "COM AGUA" : "SEM AGUA") + "\n";
        }

        if (alerta) {
            cout << "\n  !!! ALERTA: LOCAL CRITICO SEM ABASTECIMENTO !!!\n";
            ultimo_resultado += "\nALERTA: LOCAL CRITICO SEM AGUA!";
        }
        else {
            cout << "\n  [OK] Toda a rede esta abastecida!\n";
        }
    }

    // ############################################################
    //  BLOCO 4 — INTEGRIDADE ESTRUTURAL (DFS)
    //  analise de conectividade / componentes da rede
    // Apresentador: Eduardo
    // ############################################################
    // ----------------------------------------------------------
    //  DFS
    // ----------------------------------------------------------
    void dfs_util(int v, vector<bool>& visitado, vector<int>& comp) {
        visitado[v] = true; comp.push_back(v);
        for (const auto& a : adj[v])
            if (a.ativo && !visitado[a.destino])
                dfs_util(a.destino, visitado, comp);
    }
    void executar_dfs() {
        cout << "\n===========================================================\n";
        cout << "   DFS - BUSCA EM PROFUNDIDADE\n";
        cout << "===========================================================\n";

        animacao("  Explorando componentes");
        int V = total();
        vector<bool> visitado(V, false);
        ultima_acao = "DFS - Componentes da Rede";
        ultimo_resultado = "";
        ultimo_caminho.clear();
        int nc = 0;

        for (int i = 0; i < V; i++) {
            if (!visitado[i]) {
                nc++;
                vector<int> comp;
                dfs_util(i, visitado, comp);
                cout << "\n  Componente " << nc << ": ";
                string linha = "Componente " + to_string(nc) + ": ";
                for (int j = 0; j < (int)comp.size(); j++) {
                    if (j) { cout << " -> "; linha += " -> "; }
                    cout << vertices[comp[j]].nome;
                    linha += vertices[comp[j]].nome;
                    if (vertices[comp[j]].tipo == CRITICO) {
                        cout << "[!]"; linha += "[!]";
                    }
                }
                cout << "\n";
                ultimo_resultado += linha + "\n";
            }
        }

        if (nc == 1) {
            cout << "\n  [OK] Rede completamente conectada.\n";
            ultimo_resultado += "\nRede completamente conectada.";
        }
        else {
            cout << "\n  [!!] " << nc << " componentes separadas!\n";
            ultimo_resultado += "\n" + to_string(nc) + " componentes separadas!";
        }
        bfs(0);
    }
    // ############################################################
    //  BLOCO 5 — MELHOR ROTA (DIJKSTRA)
    //  caminho de menor tempo, desviando dos canos rompidos
    //  Apresentador: Eduardo
    // ############################################################
    // ----------------------------------------------------------
    //  DIJKSTRA
    // ----------------------------------------------------------
    void dijkstra(int origem, int destino) {
        cout << "\n===========================================================\n";
        cout << "   DIJKSTRA - MELHOR ROTA\n";
        cout << "===========================================================\n";
        cout << "  Origem : " << vertices[origem].nome << "\n";
        cout << "  Destino: " << vertices[destino].nome << "\n";

        animacao("  Calculando");

        int n = total();
        const int INF = numeric_limits<int>::max();
        vector<int> distancia(n, INF);
        vector<bool> visitado(n, false);
        vector<int> anterior(n, -1);
        distancia[origem] = 0;

        // Versao O(V^2): a cada passo escolhe o vertice nao visitado de
        // menor distancia (busca linear) e relaxa os canos ativos que saem dele.
        for (int i = 0; i < n - 1; i++) {
            int u = -1;
            for (int j = 0; j < n; j++) {
                if (!visitado[j] && (u == -1 || distancia[j] < distancia[u]))
                    u = j;
            }
            if (u == -1 || distancia[u] == INF) break;
            visitado[u] = true;

            for (const auto& a : adj[u]) {
                if (!a.ativo) continue;
                int v = a.destino;
                if (distancia[u] + a.peso < distancia[v]) {
                    distancia[v] = distancia[u] + a.peso;
                    anterior[v] = u;
                }
            }
        }

        ultima_acao = "Dijkstra: " + vertices[origem].nome +
            " -> " + vertices[destino].nome;
        ultimo_caminho.clear();

        if (distancia[destino] == INF) {
            cout << "\n  !!! NENHUMA ROTA ENCONTRADA !!!\n";
            ultimo_resultado = "NENHUMA ROTA ENCONTRADA!\n";
            if (vertices[destino].tipo == CRITICO)
                ultimo_resultado += "LOCAL CRITICO SEM ABASTECIMENTO!";
        }
        else {
            vector<int> caminho;
            for (int v = destino; v != -1; v = anterior[v]) caminho.push_back(v);
            reverse(caminho.begin(), caminho.end());
            ultimo_caminho = caminho;

            cout << "\n  [OK] Rota encontrada!\n  Caminho: ";
            ultimo_resultado = "Rota: ";
            for (int i = 0; i < (int)caminho.size(); i++) {
                if (i) { cout << " -> "; ultimo_resultado += " -> "; }
                cout << vertices[caminho[i]].nome;
                ultimo_resultado += vertices[caminho[i]].nome;
            }
            cout << "\n  Tempo: " << distancia[destino] << " minutos\n";
            ultimo_resultado += "\nTempo: " + to_string(distancia[destino]) + " min";
        }
        bfs(0);
    }

    // ----------------------------------------------------------
    //  SIMULAR ROMPIMENTO
    // ----------------------------------------------------------
    void simular_rompimento() {
        cout << "\n===========================================================\n";
        cout << "   SIMULAR ROMPIMENTO DE CANO\n";
        cout << "===========================================================\n";
        listar_arestas(true);

        int u, v;
        cout << "\n  No de ORIGEM  : "; cin >> u;
        cout << "  No de DESTINO : "; cin >> v;

        if (u < 0 || u >= total() || v < 0 || v >= total()) {
            cout << "  Invalido.\n"; return;
        }

        animacao("  Removendo cano " + vertices[u].nome +
            " <-> " + vertices[v].nome);

        if (!rem_aresta(u, v)) {
            cout << "  Cano nao encontrado.\n"; return;
        }

        ultima_acao = "Rompimento: " + vertices[u].nome +
            " <-> " + vertices[v].nome;
        ultimo_caminho.clear();

        vector<bool> visitado = bfs(0);
        ultimo_resultado = "";
        bool emergencia = false;

        cout << "\n  Impacto:\n";
        for (int i = 1; i < total(); i++) {
            if (!visitado[i]) {
                if (vertices[i].tipo == CRITICO) {
                    emergencia = true;
                    cout << "  <<< EMERGENCIA >>> " << vertices[i].nome << " SEM AGUA!\n";
                    ultimo_resultado += "EMERGENCIA: " + vertices[i].nome + " SEM AGUA!\n";
                }
                else {
                    cout << "  [!!] " << vertices[i].nome << " sem agua.\n";
                    ultimo_resultado += vertices[i].nome + " sem abastecimento.\n";
                }
            }
        }

        if (ultimo_resultado.empty()) {
            cout << "  [OK] Rede ainda conectada.\n";
            ultimo_resultado = "Rede ainda conectada apos rompimento.";
        }
        if (emergencia)
            cout << "\n  [!!] Use opcao 8 para rota alternativa!\n";
    }

    // ----------------------------------------------------------
    //  RESTAURAR
    // ----------------------------------------------------------
    void restaurar_cano() {
        cout << "\n===========================================================\n";
        cout << "   RESTAURAR CANO\n";
        cout << "===========================================================\n";
        listar_arestas(false);

        int u, v;
        cout << "\n  No de ORIGEM  : "; cin >> u;
        cout << "  No de DESTINO : "; cin >> v;

        if (u < 0 || u >= total() || v < 0 || v >= total()) {
            cout << "  Invalido.\n"; return;
        }

        animacao("  Restaurando");
        if (restaurar_aresta(u, v)) {
            cout << "  [OK] Cano " << vertices[u].nome << " <-> "
                << vertices[v].nome << " restaurado!\n";
            ultima_acao = "Restaurado: " + vertices[u].nome +
                " <-> " + vertices[v].nome;
            ultimo_resultado = "Cano restaurado com sucesso.";
        }
        else {
            cout << "  Cano nao encontrado ou ja ativo.\n";
        }
        bfs(0);
    }

    // ----------------------------------------------------------
    //  HELPERS
    // ----------------------------------------------------------
    void listar_vertices() const {
        cout << "\n  -----------------------------------------------------------\n";
        cout << "  " << left << setw(5) << "ID"
            << setw(13) << "Nome" << "Descricao\n";
        cout << "  -----------------------------------------------------------\n";
        for (const auto& n : vertices)
            cout << "  " << left << setw(5) << n.id
            << setw(13) << n.nome << n.descricao << "\n";
    }

    void listar_arestas(bool apenasAtivas) const {
        cout << "\n  " << left
            << setw(13) << "De" << setw(13) << "Para"
            << setw(10) << "Tempo" << setw(12) << "Cap(L/s)"
            << "Status\n";
        cout << "  " << string(56, '-') << "\n";
        for (int u = 0; u < total(); u++) {
            for (const auto& a : adj[u]) {
                if (a.destino < u) continue;
                if (apenasAtivas && !a.ativo) continue;
                if (!apenasAtivas && a.ativo) continue;
                cout << "  " << left
                    << setw(13) << vertices[u].nome
                    << setw(13) << vertices[a.destino].nome
                    << setw(10) << (to_string(a.peso) + "min")
                    << setw(12) << (to_string(a.capacidade) + "L/s")
                    << (a.ativo ? "[ativo]" : "[REMOVIDO]") << "\n";
            }
        }
    }

    static void animacao(const string& msg) {
        cout << msg;
        for (int i = 0; i < 3; i++) {
            cout << "." << flush;
            this_thread::sleep_for(chrono::milliseconds(250));
        }
        cout << "\n";
    }

    // ############################################################
    //  BLOCO 6 — MOTOR GRAFICO (gerar_html)
    //  gera o mapa em HTML/SVG a partir do estado do grafo
    //  Apresentador(a): Kaiquee
    // ############################################################
    // ==========================================================
    //  GERAR HTML — interface estilo simulador interativo
    // ==========================================================
    void gerar_html(const string& arquivo) {
        if ((int)ultimo_alcance.size() != total()) bfs(0);

        ofstream f(arquivo);
        if (!f.is_open()) { cout << "  Erro ao gerar HTML.\n"; return; }

        // Coletar arestas unicas
        struct AInfo { int u, v, cap, peso; string tipo; bool ativo; };
        vector<AInfo> arestas;
        for (int u = 0; u < total(); u++)
            for (const auto& a : adj[u])
                if (a.destino > u)
                    arestas.push_back({ u,a.destino,a.capacidade,a.peso,a.tipo,a.ativo });

        auto nosCaminho = [&](int u, int v) {
            for (int i = 0; i + 1 < (int)ultimo_caminho.size(); i++)
                if ((ultimo_caminho[i] == u && ultimo_caminho[i + 1] == v) ||
                    (ultimo_caminho[i] == v && ultimo_caminho[i + 1] == u)) return true;
            return false;
            };

        // Stats
        int criticos = 0, criticosOk = 0, bairros = 0, bairrosOk = 0, canosAt = 0, canosRem = 0;
        for (int i = 1; i < total(); i++) {
            if (vertices[i].tipo == CRITICO) { criticos++; if (ultimo_alcance[i])criticosOk++; }
            else if (vertices[i].tipo == BAIRRO) { bairros++; if (ultimo_alcance[i])bairrosOk++; }
        }
        for (auto& e : arestas) { if (e.ativo)canosAt++; else canosRem++; }
        bool haEmerg = (criticosOk < criticos);

        // Cor do no
        auto corNo = [&](const Vertice& n)->string {
            if (!ultimo_alcance[n.id] && n.id != 0) return "#6b7280";
            switch (n.tipo) {
            case FONTE:        return "#BA7517";
            case RESERVATORIO: return "#3B8BD4";
            case CRITICO:      return "#E24B4A";
            default:           return "#639922";
            }
            };

        // Raio do no
        auto rNo = [&](const Vertice& n)->double {
            if (n.tipo == FONTE)        return 22;
            if (n.tipo == RESERVATORIO) return 18;
            return 15;
            };

        // ── HTML ─────────────────────────────────────────────
        f << R"(<!DOCTYPE html>
<html lang="pt-BR">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Simulador de Agua - Sao Cristovao</title>
<style>
*{box-sizing:border-box;margin:0;padding:0;}
body{font-family:'Segoe UI',sans-serif;background:#111;color:#e2e8f0;height:100vh;display:flex;flex-direction:column;overflow:hidden;}
/* HEADER */
header{background:#1a1a2e;border-bottom:1px solid #2a2a4a;padding:10px 20px;display:flex;align-items:center;justify-content:space-between;flex-shrink:0;}
.logo{display:flex;align-items:center;gap:10px;}
.logo-icon{font-size:20px;}
.logo-title{font-size:14px;font-weight:700;letter-spacing:.3px;}
.logo-sub{font-size:10px;color:#64748b;margin-top:1px;font-family:monospace;}
.badges{display:flex;gap:6px;}
.badge{font-size:11px;padding:3px 10px;border-radius:4px;font-weight:600;font-family:monospace;}
.badge-ok  {background:#14532d;color:#86efac;border:1px solid #166534;}
.badge-warn{background:#78350f;color:#fcd34d;border:1px solid #92400e;}
.badge-err {background:#7f1d1d;color:#fca5a5;border:1px solid #991b1b;}
/* MAIN */
.main{display:grid;grid-template-columns:1fr 300px;flex:1;overflow:hidden;}
/* CANVAS */
.canvas-wrap{position:relative;overflow:hidden;background:#111;
  background-image:linear-gradient(rgba(42,42,74,.35) 1px,transparent 1px),
                   linear-gradient(90deg,rgba(42,42,74,.35) 1px,transparent 1px);
  background-size:36px 36px;}
svg{width:100%;height:100%;display:block;}
/* SIDEBAR */
.sidebar{background:#1a1a2e;border-left:1px solid #2a2a4a;display:flex;flex-direction:column;overflow:hidden;}
.sidebar-tabs{display:flex;border-bottom:1px solid #2a2a4a;flex-shrink:0;}
.tab-btn{flex:1;padding:9px 4px;font-size:11px;font-weight:600;font-family:monospace;
  background:none;border:none;color:#475569;cursor:pointer;border-bottom:2px solid transparent;transition:all .2s;}
.tab-btn.active{color:#38bdf8;border-bottom-color:#38bdf8;}
.tab-content{display:none;flex:1;overflow-y:auto;padding:12px;flex-direction:column;gap:8px;}
.tab-content.active{display:flex;}
/* PIPES */
.sec-label{font-size:10px;color:#475569;text-transform:uppercase;letter-spacing:1px;font-family:monospace;}
.pipe-item{display:flex;align-items:center;gap:7px;padding:6px 9px;border-radius:6px;
  cursor:pointer;border:1px solid #2a2a4a;background:#111;transition:all .15s;font-size:11px;}
.pipe-item:hover{border-color:#3b4a6b;background:#1e2a45;}
.pipe-item.removed{border-color:#7f1d1d;background:#1a0a0a;opacity:.85;}
.pipe-dot{width:8px;height:8px;border-radius:50%;flex-shrink:0;}
.pipe-name{font-family:monospace;font-size:11px;font-weight:600;flex:1;}
.pipe-cap{font-family:monospace;font-size:10px;color:#64748b;}
/* STATS */
.stat-grid{display:grid;grid-template-columns:1fr 1fr;gap:6px;}
.stat-card{background:#111;border:1px solid #2a2a4a;border-radius:6px;padding:8px 10px;}
.stat-label{font-size:10px;color:#64748b;font-family:monospace;margin-bottom:2px;}
.stat-val{font-size:22px;font-weight:700;font-family:monospace;}
.stat-ok  {color:#22c55e;}.stat-warn{color:#f59e0b;}.stat-err{color:#ef4444;}
/* LOG */
.log-box{background:#0a0a16;border:1px solid #2a2a4a;border-radius:6px;padding:8px 10px;
  font-family:monospace;font-size:10px;max-height:130px;overflow-y:auto;flex-shrink:0;}
.log-line{padding:1px 0;line-height:1.5;}
.log-line.ok  {color:#86efac;}.log-line.warn{color:#fcd34d;}.log-line.err{color:#fca5a5;}.log-line.info{color:#64748b;}
/* ALGO */
.algo-btns{display:grid;grid-template-columns:1fr 1fr;gap:5px;}
.algo-btn{padding:7px 5px;font-size:10px;font-weight:600;font-family:monospace;
  background:#111;border:1px solid #2a2a4a;color:#e2e8f0;border-radius:5px;cursor:pointer;transition:all .15s;}
.algo-btn:hover{background:#1e3a5f;border-color:#3b82f6;color:#38bdf8;}
.algo-btn.full{grid-column:1/-1;background:#1d3461;border-color:#3b82f6;color:#38bdf8;}
.algo-output{background:#0a0a16;border:1px solid #2a2a4a;border-radius:6px;padding:10px 12px;
  font-family:monospace;font-size:10px;line-height:1.7;overflow-y:auto;white-space:pre-wrap;
  color:#94a3b8;min-height:100px;flex:1;}
.ao-title{color:#06b6d4;font-weight:700;}
.ao-ok   {color:#22c55e;}.ao-warn{color:#f59e0b;}.ao-err{color:#ef4444;}.ao-path{color:#3b82f6;}.ao-num{color:#c084fc;}
/* ALERT */
.alert-banner{position:absolute;top:10px;left:50%;transform:translateX(-50%);
  background:#7f1d1d;border:1px solid #ef4444;border-radius:7px;padding:7px 18px;
  font-family:monospace;font-size:12px;font-weight:700;color:#fca5a5;z-index:10;
  animation:pulse 1.2s infinite;}
@keyframes pulse{0%,100%{border-color:#ef4444;}50%{border-color:#fca5a5;}}
/* TOOLTIP */
.tooltip{position:absolute;pointer-events:none;background:#1a1a2e;border:1px solid #3b4a6b;
  border-radius:7px;padding:9px 13px;font-family:monospace;font-size:11px;color:#e2e8f0;
  box-shadow:0 6px 24px rgba(0,0,0,.5);display:none;z-index:20;min-width:170px;}
.tt-name{font-weight:700;font-size:13px;margin-bottom:3px;}
.tt-row{color:#64748b;font-size:10px;line-height:1.6;}
/* LEGEND */
.legend{display:flex;flex-direction:column;gap:4px;}
.leg-item{display:flex;align-items:center;gap:7px;font-size:10px;color:#64748b;}
.leg-pipe{width:18px;height:3px;border-radius:2px;flex-shrink:0;}
.leg-dot {width:9px;height:9px;border-radius:50%;flex-shrink:0;}
/* RESET BTN */
.reset-btn{font-family:monospace;font-size:10px;padding:5px 10px;border-radius:5px;
  border:1px solid #2a2a4a;background:transparent;color:#64748b;cursor:pointer;transition:all .15s;width:100%;}
.reset-btn:hover{border-color:#22c55e;color:#22c55e;}
::-webkit-scrollbar{width:4px;}::-webkit-scrollbar-track{background:transparent;}
::-webkit-scrollbar-thumb{background:#2a2a4a;border-radius:2px;}
</style>
</head>
<body>
)";

        // ── HEADER ───────────────────────────────────────────
        f << "<header>\n";
        f << "  <div class='logo'><span class='logo-icon'>💧</span><div>";
        f << "<div class='logo-title'>Simulador de Rede de Agua &mdash; Sao Cristovao</div>";
        f << "<div class='logo-sub'>Estrutura de Dados &bull; Grafos em C++</div></div></div>\n";
        f << "  <div class='badges'>\n";
        f << "    <span class='badge " << (criticosOk == criticos ? "badge-ok" : "badge-err")
            << "'>Criticos: " << criticosOk << "/" << criticos << "</span>\n";
        f << "    <span class='badge " << (bairrosOk == bairros ? "badge-ok" : "badge-warn")
            << "'>Bairros: " << bairrosOk << "/" << bairros << "</span>\n";
        f << "    <span class='badge " << (canosRem == 0 ? "badge-ok" : "badge-warn")
            << "'>Canos: " << canosAt << " ativos</span>\n";
        f << "  </div>\n</header>\n";

        // ── MAIN ─────────────────────────────────────────────
        f << "<div class='main'>\n";

        // ── CANVAS ───────────────────────────────────────────
        f << "<div class='canvas-wrap' id='canvas'>\n";
        f << "<svg id='svg' viewBox='0 0 900 560' xmlns='http://www.w3.org/2000/svg'>\n";
        f << "<defs>\n";
        f << "  <marker id='arr' viewBox='0 0 10 10' refX='8' refY='5' markerWidth='5' markerHeight='5' orient='auto-start-reverse'>";
        f << "<path d='M2 1L8 5L2 9' fill='none' stroke='context-stroke' stroke-width='1.5' stroke-linecap='round'/></marker>\n";
        f << "  <filter id='glow'><feGaussianBlur stdDeviation='3' result='b'/><feMerge><feMergeNode in='b'/><feMergeNode in='SourceGraphic'/></feMerge></filter>\n";
        f << "</defs>\n";

        // Arestas
        for (auto& e : arestas) {
            const Vertice& n1 = vertices[e.u];
            const Vertice& n2 = vertices[e.v];
            double x1 = n1.x * 900, y1 = n1.y * 560, x2 = n2.x * 900, y2 = n2.y * 560;
            bool nc = nosCaminho(e.u, e.v);
            string cor, sw, dash, op;
            if (!e.ativo) { cor = "#7f1d1d"; sw = "2"; dash = "stroke-dasharray='6 4'"; op = "0.7"; }
            else if (nc) { cor = "#a855f7"; sw = "4"; dash = ""; op = "1"; }
            else if (ultimo_alcance[e.u] && ultimo_alcance[e.v]) { cor = "#3B8BD4"; sw = "2.5"; dash = ""; op = "1"; }
            else { cor = "#1e2d45"; sw = "1.5"; dash = ""; op = "0.35"; }

            f << "<line x1='" << x1 << "' y1='" << y1 << "' x2='" << x2 << "' y2='" << y2
                << "' stroke='" << cor << "' stroke-width='" << sw << "' " << dash
                << " stroke-opacity='" << op << "' marker-end='url(#arr)'"
                << " style='cursor:pointer'"
                << " onclick='toggleEdge(" << e.u << "," << e.v << ")'"
                << " onmouseenter='showEdgeTip(event," << e.u << "," << e.v << "," << e.cap << ")'"
                << " onmouseleave='hideTip()'/>\n";

            // Label capacidade
            double mx = (x1 + x2) / 2, my = (y1 + y2) / 2;
            double dx = x2 - x1, dy = y2 - y1, len = sqrt(dx * dx + dy * dy);
            if (len > 0) {
                double ox = -dy / len * 12, oy = dx / len * 12;
                f << "<text x='" << mx + ox << "' y='" << my + oy
                    << "' text-anchor='middle' dominant-baseline='central'"
                    << " font-size='9' font-family='monospace'"
                    << " fill='" << cor << "' opacity='" << op << "' style='pointer-events:none'>"
                    << e.cap << "L/s</text>\n";
            }
        }

        // Label ROTA nos caminhos Dijkstra
        for (int i = 0; i + 1 < (int)ultimo_caminho.size(); i++) {
            const Vertice& a = vertices[ultimo_caminho[i]];
            const Vertice& b = vertices[ultimo_caminho[i + 1]];
            double x1 = a.x * 900, y1 = a.y * 560, x2 = b.x * 900, y2 = b.y * 560;
            double mx = (x1 + x2) / 2, my = (y1 + y2) / 2;
            f << "<text x='" << mx << "' y='" << (my - 10)
                << "' text-anchor='middle' font-size='8' fill='#c084fc'"
                << " font-weight='bold' style='pointer-events:none'>ROTA</text>\n";
        }

        // Nos
        for (auto& n : vertices) {
            double cx = n.x * 900, cy = n.y * 560;
            double r = rNo(n);
            string cor = corNo(n);
            bool ok = ultimo_alcance[n.id] || (n.id == 0);
            string fillBg = ok ? (cor + "22") : "#1c1c2e";
            string strokeC = ok ? cor : "#374151";

            // Glow emergencia
            if (n.tipo == CRITICO && !ok)
                f << "<circle cx='" << cx << "' cy='" << cy << "' r='" << (r + 9)
                << "' fill='#ef4444' opacity='0.15' filter='url(#glow)'/>\n";

            // Circulo principal
            f << "<circle cx='" << cx << "' cy='" << cy << "' r='" << r
                << "' fill='" << fillBg << "' stroke='" << strokeC << "' stroke-width='2.5'"
                << " style='cursor:pointer'"
                << " onmouseenter='showNodeTip(event," << n.id << ")'"
                << " onmouseleave='hideTip()'/>\n";

            // Indicador de nivel (reservatorios e caixas)
            if ((n.tipo == RESERVATORIO) && ok) {
                f << "<rect x='" << (cx - 3) << "' y='" << (cy - 5) << "' width='6' height='8'"
                    << " rx='1' fill='rgba(255,255,255,0.3)' style='pointer-events:none'/>\n";
            }

            // Icone/label interno
            string ico = (n.tipo == FONTE) ? "ETA" :
                (n.tipo == RESERVATORIO) ? "RES" :
                (n.tipo == CRITICO && !ok) ? "!!" :
                (n.tipo == CRITICO) ? "CRI" : "BAI";
            f << "<text x='" << cx << "' y='" << cy
                << "' text-anchor='middle' dominant-baseline='central'"
                << " font-size='9' font-weight='bold' fill='" << (ok ? cor : "#6b7280") << "'"
                << " style='pointer-events:none'>" << ico << "</text>\n";

            // Nome abaixo
            f << "<text x='" << cx << "' y='" << (cy + r + 11)
                << "' text-anchor='middle' font-size='11' font-weight='600'"
                << " fill='" << (ok ? "#e2e8f0" : "#475569") << "' style='pointer-events:none'>"
                << n.nome << "</text>\n";
            f << "<text x='" << cx << "' y='" << (cy + r + 21)
                << "' text-anchor='middle' font-size='8'"
                << " fill='#475569' style='pointer-events:none'>"
                << n.descricao << "</text>\n";
        }

        f << "</svg>\n";

        // Alerta emergencia
        if (haEmerg)
            f << "<div class='alert-banner'>&#9940; EMERGENCIA: LOCAL CRITICO SEM AGUA!</div>\n";

        // Tooltip
        f << "<div class='tooltip' id='tip'><div class='tt-name' id='tt-name'></div>"
            << "<div class='tt-row' id='tt-row1'></div>"
            << "<div class='tt-row' id='tt-row2'></div></div>\n";

        f << "</div>\n"; // canvas-wrap

        // ── SIDEBAR ──────────────────────────────────────────
        f << "<div class='sidebar'>\n";
        f << "<div class='sidebar-tabs'>\n";
        f << "  <button class='tab-btn active' onclick='switchTab(\"canos\",this)'>CANOS</button>\n";
        f << "  <button class='tab-btn' onclick='switchTab(\"stats\",this)'>STATS</button>\n";
        f << "  <button class='tab-btn' onclick='switchTab(\"algo\",this)'>ALGORITMOS</button>\n";
        f << "</div>\n";

        // TAB CANOS
        f << "<div class='tab-content active' id='tab-canos'>\n";
        f << "  <div class='sec-label'>Clique para remover / restaurar</div>\n";
        f << "  <div id='pipe-list'>\n";
        for (auto& e : arestas) {
            string cor = e.ativo ? "#3B8BD4" : "#E24B4A";
            string cls = e.ativo ? "" : " removed";
            f << "    <div class='pipe-item" << cls << "' onclick='toggleEdge(" << e.u << "," << e.v << ")'>\n";
            f << "      <div class='pipe-dot' style='background:" << cor << "'></div>\n";
            f << "      <span class='pipe-name'>" << vertices[e.u].nome << "&#8596;" << vertices[e.v].nome << "</span>\n";
            f << "      <span class='pipe-cap'>" << e.cap << "L/s</span>\n";
            f << "      <span style='font-size:10px;margin-left:4px'>" << (e.ativo ? "[on]" : "[off]") << "</span>\n";
            f << "    </div>\n";
        }
        f << "  </div>\n";
        f << "  <button class='reset-btn' onclick='resetAll()'>&#8635; Restaurar todos os canos</button>\n";
        f << "  <div class='sec-label' style='margin-top:4px'>Log de eventos</div>\n";
        f << "  <div class='log-box' id='log'>\n";
        f << "    <div class='log-line info'>[inicio] Rede carregada. Use o terminal para interagir.</div>\n";
        // Ultima acao
        if (!ultima_acao.empty())
            f << "    <div class='log-line ok'>[acao] " << ultima_acao << "</div>\n";
        f << "  </div>\n";
        f << "</div>\n"; // tab-canos

        // TAB STATS
        f << "<div class='tab-content' id='tab-stats'>\n";
        f << "  <div class='sec-label'>Diagnostico da rede</div>\n";
        f << "  <div class='stat-grid'>\n";
        f << "    <div class='stat-card'><div class='stat-label'>CRITICOS</div>"
            << "<div class='stat-val " << (criticosOk == criticos ? "stat-ok" : "stat-err") << "'>"
            << criticosOk << "/" << criticos << "</div></div>\n";
        f << "    <div class='stat-card'><div class='stat-label'>BAIRROS</div>"
            << "<div class='stat-val " << (bairrosOk == bairros ? "stat-ok" : "stat-warn") << "'>"
            << bairrosOk << "/" << bairros << "</div></div>\n";
        f << "    <div class='stat-card'><div class='stat-label'>CANOS ATIVOS</div>"
            << "<div class='stat-val stat-ok'>" << canosAt << "</div></div>\n";
        f << "    <div class='stat-card'><div class='stat-label'>REMOVIDOS</div>"
            << "<div class='stat-val " << (canosRem == 0 ? "stat-ok" : "stat-warn") << "'>"
            << canosRem << "</div></div>\n";
        f << "  </div>\n";
        f << "  <div class='sec-label' style='margin-top:4px'>Status dos pontos</div>\n";
        for (int i = 0; i < total(); i++) {
            bool ok = ultimo_alcance[i] || (i == 0);
            string cor = corNo(vertices[i]);
            bool nc = find(ultimo_caminho.begin(), ultimo_caminho.end(), i) != ultimo_caminho.end();
            f << "  <div class='pipe-item' style='" << (nc ? "border-color:#a855f7;" : "") << "'>\n";
            f << "    <div class='pipe-dot' style='background:" << cor << "'></div>\n";
            f << "    <span class='pipe-name'>" << vertices[i].nome << "</span>\n";
            f << "    <span style='font-size:10px;color:" << (ok ? "#22c55e" : "#ef4444") << "'>"
                << (ok ? "COM AGUA" : "SEM AGUA") << "</span>\n";
            f << "  </div>\n";
        }
        f << "  <div class='sec-label' style='margin-top:4px'>Ultima acao</div>\n";
        f << "  <div class='log-box'><div class='log-line ok'>"
            << (ultima_acao.empty() ? "Rede iniciada" : ultima_acao) << "</div>"
            << "<div class='log-line info'>" << ultimo_resultado << "</div></div>\n";
        f << "</div>\n"; // tab-stats

        // TAB ALGORITMOS
        f << "<div class='tab-content' id='tab-algo'>\n";
        f << "  <div class='sec-label'>Resultados dos algoritmos</div>\n";
        f << "  <div class='algo-output'>";
        if (ultima_acao.empty())
            f << "Use o terminal para rodar os algoritmos.\nO resultado aparecera aqui.";
        else {
            f << "<span class='ao-title'>" << ultima_acao << "</span>\n\n";
            // Mostrar alcance atual
            f << "Status atual (BFS):\n";
            for (int i = 1; i < total(); i++) {
                bool ok = ultimo_alcance[i];
                f << (ok ? "<span class='ao-ok'>  [OK] " : "<span class='ao-err'>  [!!] ")
                    << vertices[i].nome << (vertices[i].tipo == CRITICO ? " [CRITICO]" : "")
                    << "</span>\n";
            }
            if (!ultimo_caminho.empty()) {
                f << "\n<span class='ao-path'>Rota: ";
                for (int i = 0; i < (int)ultimo_caminho.size(); i++) {
                    if (i) f << " -&gt; ";
                    f << vertices[ultimo_caminho[i]].nome;
                }
                f << "</span>\n";
            }
            if (!ultimo_resultado.empty())
                f << "\n" << ultimo_resultado;
        }
        f << "</div>\n";
        f << "  <div class='sec-label' style='margin-top:4px'>Legenda</div>\n";
        f << "  <div class='legend'>\n";
        f << "    <div class='leg-item'><div class='leg-pipe' style='background:#3B8BD4'></div>Cano ativo com fluxo</div>\n";
        f << "    <div class='leg-item'><div class='leg-pipe' style='background:#a855f7'></div>Rota Dijkstra</div>\n";
        f << "    <div class='leg-item'><div class='leg-pipe' style='background:#7f1d1d;border:1px dashed #ef4444'></div>Cano removido</div>\n";
        f << "    <div class='leg-item'><div class='leg-dot' style='background:#BA7517'></div>Estacao (ETA)</div>\n";
        f << "    <div class='leg-item'><div class='leg-dot' style='background:#3B8BD4'></div>Reservatorio</div>\n";
        f << "    <div class='leg-item'><div class='leg-dot' style='background:#E24B4A'></div>Local critico</div>\n";
        f << "    <div class='leg-item'><div class='leg-dot' style='background:#639922'></div>Bairro</div>\n";
        f << "    <div class='leg-item'><div class='leg-dot' style='background:#6b7280'></div>Sem abastecimento</div>\n";
        f << "  </div>\n";
        f << "  <div style='font-size:10px;color:#475569;text-align:center;padding:8px;margin-top:auto;border-top:1px solid #2a2a4a'>";
        f << "Volte ao terminal para interagir.<br>O mapa atualiza a cada acao.</div>\n";
        f << "</div>\n"; // tab-algo
        f << "</div>\n"; // sidebar

        f << "</div>\n"; // main

        // ── JAVASCRIPT ───────────────────────────────────────
        // Dados do grafo pro local
        f << "<script>\n";

        // Nodes JSON
        f << "const NODES=[\n";
        for (auto& n : vertices) {
            string cor = corNo(n);
            f << "  {id:" << n.id << ",nome:'" << n.nome << "',desc:'" << n.descricao
                << "',tipo:'" << (n.tipo == FONTE ? "fonte" : n.tipo == RESERVATORIO ? "reservatorio" :
                    n.tipo == CRITICO ? "critico" : "bairro")
                << "',x:" << (n.x * 900) << ",y:" << (n.y * 560) << ",cor:'" << cor << "'},\n";
        }
        f << "];\n";

        // Edges JSON
        f << "const EDGES=[\n";
        for (auto& e : arestas)
            f << "  {u:" << e.u << ",v:" << e.v << ",cap:" << e.cap << ",rem:" << (e.ativo ? "false" : "true") << "},\n";
        f << "];\n";

        f << R"(
const tip=document.getElementById('tip');
const canvas=document.getElementById('canvas');

function rNo(tipo){ return tipo==='fonte'?22:tipo==='reservatorio'?18:15; }

function moveTip(ev){
  const r=canvas.getBoundingClientRect();
  tip.style.left=(ev.clientX-r.left+14)+'px';
  tip.style.top=(ev.clientY-r.top+14)+'px';
}
function hideTip(){ tip.style.display='none'; }

function showNodeTip(ev,id){
  const n=NODES[id];
  const ok=n.tipo==='fonte'||document.querySelector('[data-id="'+id+'"]');
  document.getElementById('tt-name').textContent=n.nome;
  document.getElementById('tt-row1').textContent='Tipo: '+n.tipo;
  document.getElementById('tt-row2').textContent=n.desc;
  tip.style.display='block'; moveTip(ev);
}
function showEdgeTip(ev,u,v,cap){
  document.getElementById('tt-name').textContent=NODES[u].nome+' ↔ '+NODES[v].nome;
  document.getElementById('tt-row1').textContent='Capacidade: '+cap+' L/s';
  document.getElementById('tt-row2').textContent='Clique para remover/restaurar';
  tip.style.display='block'; moveTip(ev);
}
canvas.addEventListener('mousemove',ev=>{
  if(tip.style.display==='block') moveTip(ev);
});

function switchTab(name,btn){
  document.querySelectorAll('.tab-content').forEach(t=>t.classList.remove('active'));
  document.querySelectorAll('.tab-btn').forEach(b=>b.classList.remove('active'));
  document.getElementById('tab-'+name).classList.add('active');
  btn.classList.add('active');
}

function addLog(msg,cls){
  const box=document.getElementById('log');
  if(!box) return;
  const ts=new Date().toLocaleTimeString('pt-BR',{hour:'2-digit',minute:'2-digit',second:'2-digit'});
  const d=document.createElement('div');
  d.className='log-line '+(cls||'info');
  d.textContent='['+ts+'] '+msg;
  box.insertBefore(d,box.firstChild);
  if(box.children.length>20) box.removeChild(box.lastChild);
}

function toggleEdge(u,v){
  addLog('Acao registrada: use o terminal para remover/restaurar canos.','warn');
}
function resetAll(){
  addLog('Use opcao 9 no terminal para restaurar todos os canos.','warn');
}
)";
        f << "</script>\n</body>\n</html>\n";
        f.close();
    }
};

// ############################################################
//  BLOCO 7 — CARREGAMENTO E INTERFACE PRINCIPAL
//  criar_cidade + abrir navegador + menu + main
//  Apresentador(a): kAique
// ############################################################
// ============================================================
//  CRIAR CIDADE SAO CRISTOVAO
// ============================================================
Grafo criar_cidade() {
    Grafo g;

    // add_vertice(nome, desc, tipo, x, y)  x/y = 0.0 a 1.0
    g.add_vertice("ETA", "Est. Tratamento", FONTE, (float)0.08, (float)0.50);
    g.add_vertice("RES-1", "Reserv. Norte", RESERVATORIO, (float)0.28, (float)0.20);
    g.add_vertice("RES-2", "Reserv. Sul", RESERVATORIO, (float)0.28, (float)0.78);
    g.add_vertice("HOSPITAL", "Hosp. Municipal", CRITICO, (float)0.52, (float)0.15);
    g.add_vertice("BOMBEIROS", "Bombeiros", CRITICO, (float)0.52, (float)0.38);
    g.add_vertice("ESCOLA", "Escola Estadual", CRITICO, (float)0.52, (float)0.60);
    g.add_vertice("CENTRO", "Bairro Centro", BAIRRO, (float)0.72, (float)0.28);
    g.add_vertice("VILA-NOVA", "Vila Nova", BAIRRO, (float)0.72, (float)0.50);
    g.add_vertice("JD-VERDE", "Jardim Verde", BAIRRO, (float)0.72, (float)0.72);
    g.add_vertice("COM-NORTE", "Comercio Norte", BAIRRO, (float)0.90, (float)0.20);
    g.add_vertice("FAV-SUL", "Comunidade Sul", BAIRRO, (float)0.90, (float)0.85);

    // add_aresta(u, v, tempo(min), cap(L/s), tipo)
    g.add_aresta(0, 1, 3, 80, "tubulacao");
    g.add_aresta(0, 2, 4, 60, "tubulacao");
    g.add_aresta(1, 3, 5, 40, "tubulacao");
    g.add_aresta(1, 4, 4, 20, "tubulacao");
    g.add_aresta(1, 5, 6, 15, "tubulacao");
    g.add_aresta(1, 6, 3, 50, "tubulacao");
    g.add_aresta(1, 9, 5, 25, "ramal");
    g.add_aresta(2, 6, 4, 40, "tubulacao");
    g.add_aresta(2, 7, 5, 35, "tubulacao");
    g.add_aresta(2, 10, 6, 20, "ramal");
    g.add_aresta(6, 3, 3, 30, "emergencia");
    g.add_aresta(6, 7, 2, 25, "ramal");
    g.add_aresta(6, 8, 4, 20, "ramal");
    g.add_aresta(7, 8, 3, 15, "ramal");
    g.add_aresta(8, 10, 5, 10, "ramal");

    return g;
}

// ============================================================
//  ABRIR HTML NO NAVEGADOR
// ============================================================
void abrir_navegador(const string& arquivo) {
    ShellExecuteA(NULL, "open", arquivo.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

// ============================================================
//  GERAR E ABRIR
// ============================================================
void atualizar_mapa(Grafo& g) {
    string caminho = "mapa_rede.html";
    g.gerar_html(caminho);

    // Pegar caminho completo
    char fullPath[MAX_PATH];
    GetFullPathNameA(caminho.c_str(), MAX_PATH, fullPath, NULL);

    cout << "\n  [OK] Mapa atualizado! Abrindo no navegador...\n";
    abrir_navegador(fullPath);
    this_thread::sleep_for(chrono::milliseconds(800));
}

// ============================================================
//  MENU
// ============================================================
void limpar() { system("cls"); }

void cabecalho() {
    cout << "\n";
    cout << "===========================================================\n";
    cout << "   SIMULADOR DE REDE DE AGUA - SAO CRISTOVAO\n";
    cout << "   Estrutura de Dados - Grafos em C++\n";
    cout << "===========================================================\n";
    cout << "   O mapa visual abre automaticamente no navegador!\n";
    cout << "===========================================================\n";
}

int main() {
    system("chcp 65001 > nul");
    Grafo g = criar_cidade();

    // Abrir mapa inicial
    g.ultima_acao = "Rede iniciada";
    g.ultimo_resultado = "Todos os pontos abastecidos.";
    g.bfs(0);
    atualizar_mapa(g);

    int op = -1;
    while (op != 0) {
        limpar();
        cabecalho();

        cout << "\n";
        cout << "  -- VISUALIZACAO --------------------------------------\n";
        cout << "   1  - Ver grafo (lista de adjacencia)\n";
        cout << "   2  - Ver vertices\n";
        cout << "   3  - Ver canos ativos\n";
        cout << "\n";
        cout << "  -- ALGORITMOS ----------------------------------------\n";
        cout << "   4  - BFS  (verificar abastecimento)\n";
        cout << "   5  - DFS  (explorar componentes)\n";
        cout << "   6  - Dijkstra (melhor rota alternativa)\n";
        cout << "\n";
        cout << "  -- SIMULACAO -----------------------------------------\n";
        cout << "   7  - Simular rompimento de cano\n";
        cout << "   8  - Restaurar cano\n";
        cout << "   9  - Restaurar todos os canos\n";
        cout << "  10  - Ver locais sem agua\n";
        cout << "\n";
        cout << "  -- MAPA ----------------------------------------------\n";
        cout << "  11  - Abrir mapa no navegador\n";
        cout << "\n";
        cout << "   0  - Sair\n";
        cout << "\n";
        cout << "  Opcao: ";
        cin >> op;

        switch (op) {
        case 1: {
            cout << "\n===========================================================\n";
            cout << "   LISTA DE ADJACENCIA\n";
            cout << "===========================================================\n";
            for (int i = 0; i < g.total(); i++) {
                cout << "  [" << setw(2) << i << "] "
                    << left << setw(12) << g.vertices[i].nome << " -> ";
                for (const auto& a : g.adj[i])
                    if (a.ativo)
                        cout << g.vertices[a.destino].nome
                        << "(" << a.peso << "min/" << a.capacidade << "L/s) ";
                cout << "\n";
            }
            break;
        }
        case 2:  g.listar_vertices();    break;
        case 3:  g.listar_arestas(true); break;
        case 4:
            g.executar_bfs();
            atualizar_mapa(g);
            break;
        case 5:
            g.executar_dfs();
            atualizar_mapa(g);
            break;
        case 6: {
            g.listar_vertices();
            int d;
            cout << "\n  Destino (numero): "; cin >> d;
            if (d >= 0 && d < g.total()) {
                g.dijkstra(0, d);
                atualizar_mapa(g);
            }
            else cout << "  Invalido.\n";
            break;
        }
        case 7:
            g.simular_rompimento();
            atualizar_mapa(g);
            break;
        case 8:
            g.restaurar_cano();
            atualizar_mapa(g);
            break;
        case 9:
            g.restaurar_todas();
            g.ultima_acao = "Todos os canos restaurados";
            g.ultimo_resultado = "Rede completamente restaurada.";
            g.bfs(0);
            cout << "\n  [OK] Todos os canos restaurados!\n";
            atualizar_mapa(g);
            break;
        case 10: {
            cout << "\n===========================================================\n";
            cout << "   LOCAIS SEM AGUA PRESENTE\n";
            cout << "===========================================================\n";
            auto visitado = g.bfs(0);
            bool algum = false;
            for (int i = 1; i < g.total(); i++)
                if (!visitado[i]) {
                    algum = true;
                    cout << "  " << (g.vertices[i].tipo == CRITICO ? "[!!!] " : "[!!]  ")
                        << g.vertices[i].nome << "\n";
                }
            if (!algum) cout << "  [OK] Todos abastecidos!\n";
            break;
        }
        case 11:
            atualizar_mapa(g);
            break;
        case 0:
            cout << "\n  Encerrando. Ate mais!\n\n";
            break;
        default:
            cout << "\n  Opcao invalida.\n";
        }

        if (op != 0) {
            cout << "\n  Pressione ENTER para continuar...";
            cin.ignore(); cin.get();
        }
    }
    return 0;
}