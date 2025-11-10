import tkinter as tk
from tkinter import filedialog, messagebox
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import csv
import re

LOG_PATH = "log.txt"
THRESHOLD = 22350
RESET_TIME_S = 40

valores = [0] * RESET_TIME_S
tempo_atual = 0

# -------------------------------------------------------
# Função para ler o log.txt
# -------------------------------------------------------
def ler_valores_do_log():
    global tempo_atual
    try:
        with open(LOG_PATH, "r") as f:
            linhas = f.readlines()
    except FileNotFoundError:
        return [0] * RESET_TIME_S

    valores_encontrados = [0] * RESET_TIME_S

    # Extrai tempo atual
    if linhas:
        match_tempo = re.search(r"Tempo decorrido:\s*(\d+)s", linhas[0])
        if match_tempo:
            tempo_atual = int(match_tempo.group(1))

    # Extrai valores
    for i, linha in enumerate(linhas[1:RESET_TIME_S+1]):
        match = re.search(r"Valor:\s*(\d+)", linha)
        if match:
            valores_encontrados[i] = int(match.group(1))
        else:
            valores_encontrados[i] = 0

    return valores_encontrados

# -------------------------------------------------------
# Função para salvar CSV
# -------------------------------------------------------
def salvar_csv():
    caminho = filedialog.asksaveasfilename(defaultextension=".csv",
                                           filetypes=[("CSV files", "*.csv")])
    if not caminho:
        return

    with open(caminho, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["Segundo", "Valor"])

        for i, v in enumerate(valores):
            writer.writerow([i, v])

    messagebox.showinfo("Sucesso", "Arquivo CSV salvo com sucesso!")

# -------------------------------------------------------
# Função para salvar LOG original
# -------------------------------------------------------
def salvar_log():
    caminho = filedialog.asksaveasfilename(defaultextension=".txt",
                                           filetypes=[("Text files", "*.txt")])
    if not caminho:
        return

    with open(LOG_PATH, "r") as origem, open(caminho, "w") as destino:
        destino.write(origem.read())

    messagebox.showinfo("Sucesso", "Log salvo com sucesso!")

# -------------------------------------------------------
# Função que atualiza a interface e o gráfico
# -------------------------------------------------------
def atualizar(frame):
    global valores, tempo_atual

    dados = ler_valores_do_log()
    valores = dados

    tempos = list(range(RESET_TIME_S))

    # Atualiza linha do gráfico
    linha_plot.set_data(tempos, valores)
    linha_tempo.set_data([tempo_atual, tempo_atual], [0, THRESHOLD + 10000])

    # Valor atual
    valor_atual = valores[tempo_atual]
    texto_valor.config(text=f"Valor Atual: {valor_atual}")

    # Estado
    if valor_atual < THRESHOLD:
        estado = "Há chama próxima"
    else:
        estado = "Não há chama próxima"

    texto_estado.config(text=f"Estado: {estado}")

    # Alerta visual — cor da janela e labels
    if valor_atual > THRESHOLD:
        cor = "#ffb3b3"  # vermelho claro
    else:
        cor = "#b3ffb3"  # verde claro

    root.config(bg=cor)
    texto_valor.config(bg=cor)
    texto_estado.config(bg=cor)

    return linha_plot, linha_tempo

# -------------------------------------------------------
# Interface Tkinter
# -------------------------------------------------------
root = tk.Tk()
root.title("Monitor de Sensor de Chama - Tempo Real")
root.geometry("900x650")
root.config(bg="#b3ffb3")

# Valor atual
texto_valor = tk.Label(root, text="Valor Atual: 0", font=("Arial", 18), bg="#b3ffb3")
texto_valor.pack(pady=5)

# Estado
texto_estado = tk.Label(root, text="Estado: ---", font=("Arial", 16), bg="#b3ffb3")
texto_estado.pack(pady=5)

# Botões
frame_botoes = tk.Frame(root)
frame_botoes.pack(pady=10)

btn_csv = tk.Button(frame_botoes, text="Salvar CSV", command=salvar_csv, width=15)
btn_csv.grid(row=0, column=0, padx=10)

btn_log = tk.Button(frame_botoes, text="Salvar LOG", command=salvar_log, width=15)
btn_log.grid(row=0, column=1, padx=10)

# -------------------------------------------------------
# Gráfico Matplotlib dentro do Tkinter
# -------------------------------------------------------
fig, ax = plt.subplots(figsize=(10, 4))
plt.style.use("ggplot")

ax.set_title("Leitura do Sensor de Chama (tempo real)")
ax.set_xlabel("Segundos")
ax.set_ylabel("Valor do ADC")
ax.set_xlim(0, RESET_TIME_S - 1)
ax.set_ylim(0, THRESHOLD + 10000)

linha_plot, = ax.plot([], [], lw=2, marker='o', color='blue', label="Valor do Sensor")
ax.axhline(THRESHOLD, color="red", linestyle="--", label=f"Threshold = {THRESHOLD}")
linha_tempo, = ax.plot([], [], color="green", lw=2, linestyle="--", label="Tempo Atual")
ax.legend()

canvas = FigureCanvasTkAgg(fig, master=root)
canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)

ani = animation.FuncAnimation(fig, atualizar, interval=200)

# -------------------------------------------------------
# Iniciar programa
# -------------------------------------------------------
root.mainloop()
