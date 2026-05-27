# InfluxDB Line Protocol — Projeto de Irrigação
# Formato: measurement,tag1=valor,tag2=valor field1=valor,field2=valor timestamp_ns
#
# Tags (indexadas, usadas para filtros):
#   planta  — qual planta está sendo monitorada
#   zona    — localização física do sensor
#
# Fields (valores mensuráveis):
#   temperatura   — temperatura ambiente em °C (float)
#   agua_litros   — volume de água gasto no registro (float)
#   vezes_regada  — quantidade de regas no dia (integer, sufixo i)
#   umidade_pct   — umidade relativa em % (float)
#
# Timestamp em nanossegundos (epoch ns). Omitir = usa hora atual do servidor.

# --- Exemplos de registros diários ---

irrigacao,planta=tomate,zona=estufa_1 temperatura=27.4,agua_litros=2.5,vezes_regada=3i,umidade_pct=65.2 1716998400000000000
irrigacao,planta=alface,zona=horta_externa temperatura=22.1,agua_litros=1.2,vezes_regada=2i,umidade_pct=78.0 1716998400000000000
irrigacao,planta=pimentao,zona=estufa_2 temperatura=25.8,agua_litros=1.8,vezes_regada=2i,umidade_pct=70.5 1716998400000000000
irrigacao,planta=tomate,zona=estufa_1 temperatura=28.1,agua_litros=3.0,vezes_regada=4i,umidade_pct=60.1 1717084800000000000
irrigacao,planta=alface,zona=horta_externa temperatura=20.3,agua_litros=0.9,vezes_regada=1i,umidade_pct=82.4 1717084800000000000
