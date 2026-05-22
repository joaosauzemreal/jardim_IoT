# 🌿 Jardim IoT — Projeto Integrador

Sistema de monitoramento e controle automatizado para jardins inteligentes desenvolvido com **ESP32** e **InfluxDB**. O projeto coleta dados de sensores ambientais, armazena as métricas em um banco de dados de séries temporais local e fornece uma interface de controle via Servidor Web.

---

## 🏗️ Arquitetura do Sistema

O projeto é dividido em três camadas principais operando em rede local:

1. **Hardware / Borda (ESP32):** Atua como o cliente que controla a bomba de água e coleta dados dos sensores (Umidade do Solo e Nível do Reservatório) e os envia para o influxDB.
2. **Banco de Dados (InfluxDB):** Banco de dados InfluxDB v2, um banco de dados otimizado para séries temporais que armazena os registros gerados pela placa.
3. **Interface de Usuário (Painel Web):** Página HTML que permite ao usuário acompanhar as métricas dos sensores e interagir em tempo real com o sistema, ligando/desligando a bomba de água.

---

## 🛠️ Tecnologias Utilizadas

* **Microcontrolador:** ESP32
* **Linguagem de Programação:** C++
* **Banco de Dados:** InfluxDB v2.7 (Otimizado para IoT e Séries Temporais)
* **Protocolo de Comunicação:** HTTP (Client para persistência / Server para controle)
