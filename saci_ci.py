"""
saci_ci.py
----------
Busca os dados de alocação de salas do Centro de Informática (CI)
da UFPB a partir da API do SACI: https://sa.ci.ufpb.br

Uso:
    python saci_ci.py              # exibe resumo no terminal
    python saci_ci.py --json       # salva resultado bruto em saci_ci.json
    python saci_ci.py --csv        # salva alocações em saci_ci.csv
    python saci_ci.py --id CCEN    # busca outro centro
"""

import argparse
import csv
import json
import sys
import urllib.request

API_URL = "https://sa.ci.ufpb.br/api/paas/center"
CENTRO_ID = "CI"

HEADERS = {
    "Accept": "application/json, text/plain, */*",
    "Accept-Language": "en-US,en;q=0.5",
    "Authorization": "Bearer",          # token vazio — API pública
    "Connection": "keep-alive",
    "Referer": "https://sa.ci.ufpb.br/salas/ci",
    "Sec-Fetch-Dest": "empty",
    "Sec-Fetch-Mode": "cors",
    "Sec-Fetch-Site": "same-origin",
    "User-Agent": (
        "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:130.0) "
        "Gecko/20100101 Firefox/130.0"
    ),
}


def fetch_data(centro_id: str = CENTRO_ID) -> dict:
    """Faz GET na API e retorna o JSON completo."""
    url = f"{API_URL}?id={centro_id}"
    req = urllib.request.Request(url, headers=HEADERS)
    with urllib.request.urlopen(req, timeout=15) as resp:
        raw = resp.read()
    return json.loads(raw)


def parse_salas(data: dict) -> list[dict]:
    """
    Extrai lista de alocações (uma linha por aula/sala).

    Campos retornados:
        sala, bloco, tipo, capacidade, acessivel,
        codigo, disciplina, turma, docente, departamento,
        horario, alunos
    """
    rows = []
    salas = data.get("solution", {}).get("solution", [])
    for sala in salas:
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


def print_summary(data: dict, rows: list[dict]) -> None:
    salas_raw = data.get("solution", {}).get("solution", [])

    print("=" * 60)
    print(f"  {data.get('centro')} ({data.get('sigla')})")
    print("=" * 60)
    print(f"  Descrição : {data.get('description', '').strip()}")
    print(f"  Status    : {data.get('status')}")
    print(f"  Atualizado: {data.get('date')}")
    print(f"  Salas     : {len(salas_raw)}")
    print(f"  Alocações : {len(rows)}")
    print()

    # Agrupa por sala
    salas: dict[str, list] = {}
    for r in rows:
        salas.setdefault(r["sala"], []).append(r)

    for nome, aulas in sorted(salas.items()):
        info = aulas[0]
        acesso = " | ♿" if info["acessivel"] else ""
        print(f"  Sala {nome} | {info['tipo']} | Cap. {info['capacidade']}{acesso}")
        for a in aulas:
            disc = a["disciplina"][:45]
            print(f"    [{a['horario']:>10}]  {disc:<45}  T{a['turma']}  {a['docente']}")
        print()


def save_json(data: dict, path: str = "saci_ci.json") -> None:
    with open(path, "w", encoding="utf-8") as f:
        json.dump(data, f, ensure_ascii=False, indent=2)
    print(f"JSON salvo em: {path}")


def save_csv(rows: list[dict], path: str = "saci_ci.csv") -> None:
    if not rows:
        print("Sem dados para salvar.")
        return
    with open(path, "w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fieldnames=rows[0].keys())
        writer.writeheader()
        writer.writerows(rows)
    print(f"CSV salvo em: {path}  ({len(rows)} linhas)")


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Busca dados de alocação de salas do SACI (UFPB)"
    )
    parser.add_argument(
        "--id", default=CENTRO_ID, metavar="SIGLA",
        help="Sigla do centro (padrão: CI)",
    )
    parser.add_argument("--json", action="store_true", help="Salva JSON bruto")
    parser.add_argument("--csv",  action="store_true", help="Salva alocações em CSV")
    args = parser.parse_args()

    print(f"Buscando dados de '{args.id}' em {API_URL}...")
    try:
        data = fetch_data(args.id)
    except Exception as e:
        print(f"Erro ao buscar dados: {e}", file=sys.stderr)
        sys.exit(1)

    rows = parse_salas(data)
    print_summary(data, rows)

    if args.json:
        save_json(data)
    if args.csv:
        save_csv(rows)


if __name__ == "__main__":
    main()
