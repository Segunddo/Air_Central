"""
saci_ci.py
----------
Busca os dados de alocação de salas do Centro de Informática (CI)
da UFPB a partir da API do SACI: https://sa.ci.ufpb.br
e gera o arquivo agendamentos.json no formato consumido pelo SaveData (Qt/C++).

Formato do horário SACI (ex: "25M34"):
  - Dígitos iniciais  → dias da semana (2=Seg, 3=Ter, 4=Qua, 5=Qui, 6=Sex)
  - Letra do meio     → turno (M=Manhã, T=Tarde, N=Noite)
  - Dígitos finais    → slots do turno

Cada aula gera duas regras por dia:
  - "Ligar"    no horário de início do primeiro slot
  - "Desligar" no horário de fim do último slot

Uso:
    python saci_ci.py                        # gera agendamentos.json
    python saci_ci.py --id CCEN              # outro centro
    python saci_ci.py --temp 24              # temperatura padrão (default: 22)
    python saci_ci.py --out meu_arquivo.json # caminho de saída
    python saci_ci.py --dry-run              # só imprime, não salva
"""

import argparse
import json
import re
import sys
import urllib.request
from collections import defaultdict

# Força o terminal a aceitar caracteres UTF-8 no Windows para evitar crash nos prints
if sys.stdout.encoding.lower() != 'utf-8':
    sys.stdout.reconfigure(encoding='utf-8')

# ---------------------------------------------------------------------------
# Configuração da API
# ---------------------------------------------------------------------------
API_URL   = "https://sa.ci.ufpb.br/api/paas/center"
CENTRO_ID = "CI"

HEADERS = {
    "Accept":          "application/json, text/plain, */*",
    "Accept-Language": "en-US,en;q=0.5",
    "Authorization":   "Bearer",
    "Connection":      "keep-alive",
    "Referer":         "https://sa.ci.ufpb.br/salas/ci",
    "Sec-Fetch-Dest":  "empty",
    "Sec-Fetch-Mode":  "cors",
    "Sec-Fetch-Site":  "same-origin",
    "User-Agent": (
        "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:130.0) "
        "Gecko/20100101 Firefox/130.0"
    ),
}

# ---------------------------------------------------------------------------
# Tabelas de decodificação
# ---------------------------------------------------------------------------

# Número do dia → nome abreviado (para exibição)
DIA_NOME = {
    "2": "Seg",
    "3": "Ter",
    "4": "Qua",
    "5": "Qui",
    "6": "Sex",
    "7": "Sab",
}

# Mapeamento exato dos blocos de horário da UFPB
SLOTS_HORARIOS = {
    "M": {
        1: ("07:00", "08:00"),
        2: ("08:00", "09:00"),
        3: ("09:00", "10:00"),
        4: ("10:00", "11:00"),
        5: ("11:00", "12:00"),
        6: ("12:00", "13:00"),
    },
    "T": {
        1: ("13:00", "14:00"),
        2: ("14:00", "15:00"),
        3: ("15:00", "16:00"),
        4: ("16:00", "17:00"),
        5: ("17:00", "18:00"),
        6: ("18:00", "19:00"),
    },
    "N": {
        1: ("19:00", "19:50"),
        2: ("19:50", "20:40"),
        3: ("20:40", "21:30"),
        4: ("21:30", "22:20"),
    }
}

# ---------------------------------------------------------------------------
# Parsing do código de horário
# ---------------------------------------------------------------------------

def parse_horario(codigo: str) -> list[dict]:
    """
    Converte um código de horário SACI em lista de eventos com horas exatas.
    Exemplos:
        "25M34"  → Seg e Qui, Manhã slots 3-4 → 09:00–11:00
        "3N12"   → Ter, Noite slots 1-2 → 19:00–20:40
    """
    # Regex: (dias)(turno)(slots)
    m = re.fullmatch(r"([2-7]+)([MTN])([1-9]+)", codigo.strip())
    if not m:
        return []

    dias_str, turno, slots_str = m.group(1), m.group(2), m.group(3)
    tabela_turno = SLOTS_HORARIOS.get(turno)

    if not tabela_turno:
        return []

    slots = [int(c) for c in slots_str if int(c) in tabela_turno]
    if not slots:
        return []

    # O início é o horário de abertura do menor slot, e o fim é o fechamento do maior slot
    hora_inicio = tabela_turno[min(slots)][0]
    hora_fim    = tabela_turno[max(slots)][1]

    return [
        {"dia": d, "inicio": hora_inicio, "fim": hora_fim}
        for d in dias_str
        if d in DIA_NOME
    ]


# ---------------------------------------------------------------------------
# Fetch + parse bruto
# ---------------------------------------------------------------------------

def fetch_data(centro_id: str = CENTRO_ID) -> dict:
    url = f"{API_URL}?id={centro_id}"
    req = urllib.request.Request(url, headers=HEADERS)
    with urllib.request.urlopen(req, timeout=15) as resp:
        return json.loads(resp.read())


def parse_salas(data: dict) -> list[dict]:
    """Extrai lista plana de alocações (uma linha por aula)."""
    rows = []
    for sala in data.get("solution", {}).get("solution", []):
        for aula in sala.get("classes", []):
            rows.append({
                "sala":        sala.get("nome"),
                "bloco":       sala.get("bloco"),
                "tipo":        sala.get("tipo"),
                "capacidade":  sala.get("capacidade"),
                "acessivel":   bool(sala.get("acessivel")),
                "codigo":      aula.get("codigo"),
                "disciplina":  aula.get("nome", "").strip(),
                "turma":       aula.get("turma"),
                "docente":     aula.get("docente"),
                "departamento":aula.get("departamento"),
                "horario":     aula.get("horario"),
                "alunos":      aula.get("alunos"),
            })
    return rows


# ---------------------------------------------------------------------------
# Geração do agendamentos.json
# ---------------------------------------------------------------------------

def build_agendamentos(rows: list[dict], temp_padrao: int = 22) -> dict:
    """
    Monta o dict no formato esperado pelo SaveData::ler_arquivo() do Qt,
    mesclando aulas consecutivas para evitar múltiplos Ligar/Desligar.
    """
    raw_intervals: dict[str, list[dict]] = defaultdict(list)

    for row in rows:
        sala    = str(row["sala"]).strip() # Evita de que fique um espaço em branco no final do nome
        horario = row.get("horario", "")
        if not horario:
            continue

        for bloco in horario.split():
            eventos = parse_horario(bloco)
            for ev in eventos:
                dia_nome = DIA_NOME.get(ev["dia"], ev["dia"])
                raw_intervals[sala].append({
                    "dia": dia_nome,
                    "inicio": ev["inicio"],
                    "fim": ev["fim"]
                })

    # Função auxiliar para converter "HH:MM" em minutos (ex: "19:50" -> 1190)
    def time_to_min(t: str) -> int:
        h, m = map(int, t.split(':'))
        return h * 60 + m

    agendamentos: dict[str, list[dict]] = defaultdict(list)

    for sala, intervalos in raw_intervals.items():
        por_dia = defaultdict(list)
        for iv in intervalos:
            por_dia[iv["dia"]].append((time_to_min(iv["inicio"]), time_to_min(iv["fim"])))

        for dia, times in por_dia.items():
            # Ordena os horários cronologicamente
            times.sort(key=lambda x: x[0])

            merged = []
            for t_inicio, t_fim in times:
                if not merged:
                    merged.append([t_inicio, t_fim])
                else:
                    last = merged[-1]
                    # Mescla se a aula atual começa no mesmo minuto (ou antes) que a anterior termina
                    if t_inicio <= last[1]:
                        last[1] = max(last[1], t_fim)
                    else:
                        merged.append([t_inicio, t_fim])

            # Converte os minutos mesclados de volta para HH:MM e cria as regras
            for m_inicio, m_fim in merged:
                str_inicio = f"{m_inicio // 60:02d}:{m_inicio % 60:02d}"
                str_fim    = f"{m_fim // 60:02d}:{m_fim % 60:02d}"

                agendamentos[sala].append({
                    "hora": str_inicio,
                    "acao": "Ligar",
                    "temp": str(temp_padrao),
                    "dia":  dia
                })
                agendamentos[sala].append({
                    "hora": str_fim,
                    "acao": "Desligar",
                    "temp": "--",
                    "dia":  dia
                })

    # Ordenação final para a visualização na interface (Por dia da semana e hora)
    ordem_dia = {v: int(k) for k, v in DIA_NOME.items()}
    for sala in agendamentos:
        agendamentos[sala].sort(
            key=lambda r: (ordem_dia.get(r["dia"], 9), r["hora"])
        )

    return dict(agendamentos)


# ---------------------------------------------------------------------------
# Saída / utilitários
# ---------------------------------------------------------------------------

def print_summary(agendamentos: dict) -> None:
    print("=" * 60)
    print(f"  Salas processadas : {len(agendamentos)}")
    total_regras = sum(len(v) for v in agendamentos.values())
    print(f"  Total de regras   : {total_regras}")
    print("=" * 60)
    for sala, regras in sorted(agendamentos.items()):
        print(f"\n  {sala}  ({len(regras)} regras)")
        for r in regras:
            # Optando por texto puro ao invés de Emojis para 100% de estabilidade no QProcess
            icone = "[ON] " if r["acao"] == "Ligar" else "[OFF]"
            temp  = f"  {r['temp']}°C" if r["acao"] == "Ligar" else ""
            print(f"    {icone} {r['dia']}  {r['hora']}{temp}")


def save_agendamentos(agendamentos: dict, path: str) -> None:
    with open(path, "w", encoding="utf-8") as f:
        json.dump(agendamentos, f, ensure_ascii=False, indent=2)
    print(f"\nAgendamentos salvos em: {path}")


# ---------------------------------------------------------------------------
# main
# ---------------------------------------------------------------------------

def main() -> None:
    parser = argparse.ArgumentParser(
        description="Gera agendamentos.json a partir do SACI (UFPB)"
    )
    parser.add_argument("--id",      default=CENTRO_ID, metavar="SIGLA",
                        help="Sigla do centro (padrão: CI)")
    parser.add_argument("--temp",    default=22, type=int, metavar="TEMP",
                        help="Temperatura padrão para regras Ligar (padrão: 22)")
    parser.add_argument("--out",     default="agendamentos.json", metavar="ARQUIVO",
                        help="Caminho do arquivo de saída (padrão: agendamentos.json)")
    parser.add_argument("--dry-run", action="store_true",
                        help="Só imprime o resultado, não salva arquivo")
    args = parser.parse_args()

    print(f"Buscando dados de '{args.id}' em {API_URL} ...")
    try:
        data = fetch_data(args.id)
    except Exception as e:
        print(f"Erro ao buscar dados: {e}", file=sys.stderr)
        sys.exit(1)

    rows         = parse_salas(data)
    agendamentos = build_agendamentos(rows, temp_padrao=args.temp)

    print_summary(agendamentos)

    if args.dry_run:
        print("\n[dry-run] Nenhum arquivo salvo.")
        print(json.dumps(agendamentos, ensure_ascii=False, indent=2))
    else:
        save_agendamentos(agendamentos, args.out)


if __name__ == "__main__":
    main()
