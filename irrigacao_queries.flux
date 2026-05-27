// ── Consultas Flux — Projeto de Irrigação ──────────────────────
// Bucket padrão: irrigacao_bucket
// Ajuste o range e os filtros conforme necessário.


// 1. Todos os dados dos últimos 7 dias
from(bucket: "irrigacao_bucket")
  |> range(start: -7d)
  |> filter(fn: (r) => r._measurement == "irrigacao")


// 2. Consumo total de água por dia (últimos 30 dias)
from(bucket: "irrigacao_bucket")
  |> range(start: -30d)
  |> filter(fn: (r) => r._measurement == "irrigacao")
  |> filter(fn: (r) => r._field == "agua_litros")
  |> aggregateWindow(every: 1d, fn: sum, createEmpty: false)
  |> yield(name: "consumo_diario")


// 3. Média de temperatura por planta (última semana)
from(bucket: "irrigacao_bucket")
  |> range(start: -7d)
  |> filter(fn: (r) => r._measurement == "irrigacao")
  |> filter(fn: (r) => r._field == "temperatura")
  |> group(columns: ["planta"])
  |> mean()
  |> yield(name: "temp_media_por_planta")


// 4. Total de vezes regada por zona hoje
from(bucket: "irrigacao_bucket")
  |> range(start: today())
  |> filter(fn: (r) => r._measurement == "irrigacao")
  |> filter(fn: (r) => r._field == "vezes_regada")
  |> group(columns: ["zona"])
  |> sum()
  |> yield(name: "regas_hoje_por_zona")


// 5. Umidade mínima registrada por planta (últimas 24h)
from(bucket: "irrigacao_bucket")
  |> range(start: -24h)
  |> filter(fn: (r) => r._measurement == "irrigacao")
  |> filter(fn: (r) => r._field == "umidade_pct")
  |> group(columns: ["planta"])
  |> min()
  |> yield(name: "umidade_minima")


// 6. Série temporal completa de uma planta específica
from(bucket: "irrigacao_bucket")
  |> range(start: -7d)
  |> filter(fn: (r) => r._measurement == "irrigacao")
  |> filter(fn: (r) => r.planta == "tomate")
  |> pivot(rowKey: ["_time"], columnKey: ["_field"], valueColumn: "_value")
  |> yield(name: "historico_tomate")
