import matplotlib.pyplot as plt
import matplotlib.animation as animation
import re

LOG_PATH = "log.txt"
THRESHOLD = 22350   # mesma linha de decisão usada na STM32MP1
RESET_TIME_S = 40    # número total de segundos monitorados

# Histórico para plotar
valores = [0] * RESET_TIME_S  # inicializa com zeros

# Tempo atual lido do log
tempo_atual = 0

def ler_valores_do_log():
    """
    Lê o log.txt e extrai todos os valores do tipo:
    Valor: XXXXX - ...
    Retorna lista de inteiros, usando 0 para linhas com "-"
    """
    global tempo_atual
    try:
        with open(LOG_PATH, "r") as f:
            linhas = f.readlines()
    except FileNotFoundError:
        return [0] * RESET_TIME_S

    valores_encontrados = [0] * RESET_TIME_S

    # Primeira linha: Tempo decorrido atual
    if linhas:
        match_tempo = re.search(r"Tempo decorrido:\s*(\d+)s", linhas[0])
        if match_tempo:
            tempo_atual = int(match_tempo.group(1))
    
    # Demais linhas: valores ou "-"
    for i, linha in enumerate(linhas[1:RESET_TIME_S+1]):
        match = re.search(r"Valor:\s*(\d+)", linha)
        if match:
            valores_encontrados[i] = int(match.group(1))
        else:
            valores_encontrados[i] = 0  # substitui "-" por 0

    return valores_encontrados

# Configuração da figura
plt.style.use("ggplot")
fig, ax = plt.subplots(figsize=(12, 5))
ax.set_title("Leitura do Sensor de Chama (tempo real)")
ax.set_xlabel("Segundos")
ax.set_ylabel("Valor do ADC")

# Linha dinâmica do sensor
linha_plot, = ax.plot([], [], lw=2, marker='o', color='blue', label="Valor do Sensor")

# Linha de decisão (fixa)
ax.axhline(THRESHOLD, color="red", linestyle="--", label="Threshold = 22350")

# Linha vertical que indica o tempo atual
linha_tempo, = ax.plot([], [], color="green", lw=2, linestyle="--", label="Tempo Atual")

# Definindo limites fixos do gráfico
ax.set_xlim(0, RESET_TIME_S - 1)
ax.set_ylim(0, THRESHOLD + 10000)

ax.legend()

def atualizar(frame):
    global valores, tempo_atual

    dados = ler_valores_do_log()
    valores = dados
    tempos = list(range(RESET_TIME_S))

    # Atualiza a linha do sensor sempre, mesmo que seja 0
    linha_plot.set_data(tempos, valores)

    # Atualiza a linha vertical do tempo atual
    linha_tempo.set_data([tempo_atual, tempo_atual], [ax.get_ylim()[0], ax.get_ylim()[1]])

    return linha_plot, linha_tempo

# Atualiza a cada 100 ms para maior frequência
ani = animation.FuncAnimation(fig, atualizar, interval=100)
plt.tight_layout()
plt.show()
