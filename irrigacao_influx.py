"""
Projeto de Irrigação — Envio de dados para o InfluxDB
Dependência: pip install influxdb-client

Configure as variáveis abaixo antes de rodar.
"""

from influxdb_client import InfluxDBClient, Point, WritePrecision
from influxdb_client.client.write_api import SYNCHRONOUS
from datetime import datetime, timezone

# ── Configuração ──────────────────────────────────────────────
INFLUX_URL   = "http://localhost:8086"   # URL do seu InfluxDB
INFLUX_TOKEN = "SEU_TOKEN_AQUI"          # Token de acesso
INFLUX_ORG   = "minha_org"
INFLUX_BUCKET = "irrigacao_bucket"
# ─────────────────────────────────────────────────────────────


def escrever_registro(planta: str, zona: str, temperatura: float,
                      agua_litros: float, vezes_regada: int,
                      umidade_pct: float):
    """Envia um registro de irrigação para o InfluxDB."""

    with InfluxDBClient(url=INFLUX_URL, token=INFLUX_TOKEN, org=INFLUX_ORG) as client:
        write_api = client.write_api(write_options=SYNCHRONOUS)

        ponto = (
            Point("irrigacao")
            .tag("planta", planta)
            .tag("zona", zona)
            .field("temperatura", float(temperatura))
            .field("agua_litros", float(agua_litros))
            .field("vezes_regada", int(vezes_regada))
            .field("umidade_pct", float(umidade_pct))
            .time(datetime.now(timezone.utc), WritePrecision.NS)
        )

        write_api.write(bucket=INFLUX_BUCKET, record=ponto)
        print(f"[OK] Registro gravado — {planta} / {zona} às {datetime.now().strftime('%H:%M:%S')}")


def escrever_lote(registros: list[dict]):
    """Envia múltiplos registros de uma vez (mais eficiente)."""

    with InfluxDBClient(url=INFLUX_URL, token=INFLUX_TOKEN, org=INFLUX_ORG) as client:
        write_api = client.write_api(write_options=SYNCHRONOUS)

        pontos = []
        for r in registros:
            p = (
                Point("irrigacao")
                .tag("planta", r["planta"])
                .tag("zona", r["zona"])
                .field("temperatura", float(r["temperatura"]))
                .field("agua_litros", float(r["agua_litros"]))
                .field("vezes_regada", int(r["vezes_regada"]))
                .field("umidade_pct", float(r["umidade_pct"]))
                .time(r.get("timestamp", datetime.now(timezone.utc)), WritePrecision.NS)
            )
            pontos.append(p)

        write_api.write(bucket=INFLUX_BUCKET, record=pontos)
        print(f"[OK] {len(pontos)} registros gravados.")


# ── Exemplo de uso ────────────────────────────────────────────
if __name__ == "__main__":

    # Registro único
    escrever_registro(
        planta="tomate",
        zona="estufa_1",
        temperatura=27.4,
        agua_litros=2.5,
        vezes_regada=3,
        umidade_pct=65.2,
    )

    # Lote de registros
    lote = [
        {"planta": "alface",   "zona": "horta_externa", "temperatura": 22.1, "agua_litros": 1.2, "vezes_regada": 2, "umidade_pct": 78.0},
        {"planta": "pimentao", "zona": "estufa_2",      "temperatura": 25.8, "agua_litros": 1.8, "vezes_regada": 2, "umidade_pct": 70.5},
    ]
    escrever_lote(lote)
