import socket
import threading
from collections import deque
import matplotlib.pyplot as plt
import matplotlib.animation as animation

ESP_IP = "192.168.4.1"
ESP_PORT = 4210
MAX_PUNTOS = 200  # Muestra los últimos 4 segundos a 50 Hz

# Colas de datos para la gráfica
t_vals = deque(maxlen=MAX_PUNTOS)
err_vals = deque(maxlen=MAX_PUNTOS)
p_vals = deque(maxlen=MAX_PUNTOS)
d_vals = deque(maxlen=MAX_PUNTOS)
izq_vals = deque(maxlen=MAX_PUNTOS)
der_vals = deque(maxlen=MAX_PUNTOS)

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.settimeout(1.0)
ejecutando = True


def hilo_recepcion():
    """Envía HELLO hasta recibir datos y alimenta las colas en tiempo real."""
    conectado = False
    while ejecutando:
        if not conectado:
            try:
                sock.sendto(b"HELLO", (ESP_IP, ESP_PORT))
            except OSError:
                pass
        try:
            data, _ = sock.recvfrom(1024)
            conectado = True
            partes = data.decode("utf-8").strip().split(",")
            if len(partes) == 6:
                t_vals.append(int(partes[0]) / 1000.0)
                err_vals.append(float(partes[1]))
                p_vals.append(float(partes[2]))
                d_vals.append(float(partes[3]))
                izq_vals.append(int(partes[4]))
                der_vals.append(int(partes[5]))
        except socket.timeout:
            conectado = False
        except Exception:
            pass


def hilo_comandos():
    """Permite enviar comandos al robot desde la terminal mientras corre."""
    print("\n--- CONSOLA DE CONTROL REMOTO ---")
    print("Comandos: g (arrancar) | r (parar) | p<num> (kP) | d<num> (kD) | v<num> (velBase)")
    while ejecutando:
        try:
            cmd = input("> ").strip()
            if cmd:
                sock.sendto(cmd.encode("utf-8"), (ESP_IP, ESP_PORT))
        except EOFError:
            break


# Lanzamos hilos de red y consola en segundo plano
threading.Thread(target=hilo_recepcion, daemon=True).start()
threading.Thread(target=hilo_comandos, daemon=True).start()

# Configuración de las gráficas en vivo
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 6), sharex=True)
fig.suptitle("Telemetría Wi-Fi UDP - Siguelíneas ESP32")

linea_err, = ax1.plot([], [], label="Error", color="black", linewidth=2)
linea_p, = ax1.plot([], [], label="P", color="tab:blue", alpha=0.7)
linea_d, = ax1.plot([], [], label="D", color="tab:orange", alpha=0.7)
ax1.set_ylabel("Error / Corrección")
ax1.legend(loc="upper left")
ax1.grid(True)

linea_izq, = ax2.plot([], [], label="Mot Izq", color="tab:green")
linea_der, = ax2.plot([], [], label="Mot Der", color="tab:red")
ax2.set_ylim(-270, 270)
ax2.set_ylabel("PWM Motores")
ax2.set_xlabel("Tiempo (s)")
ax2.legend(loc="upper left")
ax2.grid(True)


def actualizar(_):
    if not t_vals:
        return linea_err, linea_p, linea_d, linea_izq, linea_der

    t = list(t_vals)
    linea_err.set_data(t, list(err_vals))
    linea_p.set_data(t, list(p_vals))
    linea_d.set_data(t, list(d_vals))
    linea_izq.set_data(t, list(izq_vals))
    linea_der.set_data(t, list(der_vals))

    ax1.relim()
    ax1.autoscale_view()
    ax2.set_xlim(t[0], max(t[-1], t[0] + 0.1))
    return linea_err, linea_p, linea_d, linea_izq, linea_der


ani = animation.FuncAnimation(fig, actualizar, interval=40, cache_frame_data=False)
plt.tight_layout()
plt.show()

ejecutando = False
sock.close()