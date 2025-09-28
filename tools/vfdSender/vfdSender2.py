#!/usr/bin/env python3
"""
vfd_gui.py

PySide6 GUI for sending ASCII + hex-escape sequences to VFD displays,
running linear test scripts, and showing TX/RX in ASCII + HEX side-by-side.

Requirements:
    - Python 3.13
    - PySide6
    - pyserial

Run:
    python vfd_gui.py
"""
from __future__ import annotations
import sys
import re
import time
import threading
from dataclasses import dataclass
from typing import Optional, List, Tuple

from PySide6.QtCore import (
    Qt,
    QSettings,
    QThread,
    Signal,
    Slot,
    QObject,
)
from PySide6.QtGui import QAction, QIcon, QTextCursor, QPalette, QColor
from PySide6.QtWidgets import (
    QApplication,
    QMainWindow,
    QTextEdit,
    QWidget,
    QLineEdit,
    QPushButton,
    QProgressBar,
    QHBoxLayout,
    QVBoxLayout,
    QLabel,
    QComboBox,
    QFileDialog,
    QMessageBox,
    QMenu,
    QSpinBox,
    QFormLayout,
    QDialog,
    QDialogButtonBox,
)

# serial imports - may raise ImportError if not installed
try:
    import serial
    import serial.tools.list_ports
except Exception as e:
    serial = None  # we'll handle missing pyserial at runtime


# ---------- Utilities ----------
HEX_ESCAPE_RE = re.compile(r'\\x([0-9A-Fa-f]{2})')


def parse_escaped_string(input_str: str) -> bytes:
    """
    Convert a string containing literal backslash escapes like '\\x1B' into bytes.
    Plain text is encoded as ASCII (errors='replace' to avoid Exceptions).
    Example: 'Hello\\x1B@' -> b'Hello\x1b@'
    """
    output = bytearray()
    pos = 0
    for m in HEX_ESCAPE_RE.finditer(input_str):
        # add text before escape
        part = input_str[pos:m.start()]
        output.extend(part.encode('ascii', errors='replace'))
        output.append(int(m.group(1), 16))
        pos = m.end()
    # trailing text
    output.extend(input_str[pos:].encode('ascii', errors='replace'))
    return bytes(output)


def bytes_to_hex(b: bytes) -> str:
    return " ".join(f"{x:02X}" for x in b)


def safe_decode_ascii(b: bytes) -> str:
    try:
        return b.decode('ascii', errors='replace')
    except Exception:
        # fallback
        return "".join((chr(x) if 32 <= x < 127 else "�") for x in b)


# ---------- Serial Worker (reader) ----------
class SerialReaderWorker(QThread):
    """
    Thread that reads from an open serial.Serial object and emits data lines.
    """
    received = Signal(bytes)  # raw bytes read

    def __init__(self, ser: serial.Serial):
        super().__init__(parent=None)
        self._ser = ser
        self._running = True

    def run(self):
        # read loop - use short sleeps to be responsive
        while self._running:
            try:
                if self._ser.in_waiting:
                    data = self._ser.read(self._ser.in_waiting)
                    if data:
                        self.received.emit(data)
                else:
                    # small sleep to avoid busy-waiting
                    time.sleep(0.02)
            except Exception:
                # if serial exception occurs, stop thread
                break

    def stop(self):
        self._running = False
        self.wait(1000)


# ---------- Script Runner ----------
@dataclass
class ScriptCommand:
    cmd_type: str  # 'send' | 'expect' | 'delay'
    payload: Optional[str]  # send/expect payload (string) or delay value as string
    delay_ms: Optional[int] = None  # optional inline delay for send


class ScriptRunnerWorker(QThread):
    """
    Executes parsed script commands sequentially. Communicates with main thread
    via signals to perform serial writes and to check for received data.
    """

    log = Signal(str, str, bytes)  # prefix ('TX'|'RX'|'INFO'|'ERR'), ascii_text, raw_bytes
    progress = Signal(int)  # 0..100
    finished_ok = Signal(bool)  # True if script completed with all expectations met

    # Requests
    send_bytes = Signal(bytes)  # tell main GUI to send bytes over serial

    def __init__(self, commands: List[ScriptCommand], expect_timeout_ms: int = 2000):
        super().__init__(parent=None)
        self._commands = commands
        self._expect_timeout_ms = expect_timeout_ms
        self._running = True

        # buffer of received bytes from serial - thread-safe access via lock
        self._recv_buffer = bytearray()
        self._recv_lock = threading.Lock()

        # external receiver will call append_received() to push incoming data
        # We do not own the serial port here; we emit send_bytes to ask GUI to write.

    def append_received(self, data: bytes):
        with self._recv_lock:
            self._recv_buffer.extend(data)

    def _consume_buffer(self) -> bytes:
        with self._recv_lock:
            b = bytes(self._recv_buffer)
            self._recv_buffer.clear()
            return b

    def run(self):
        total = len(self._commands)
        idx = 0
        success = True

        for cmd in self._commands:
            if not self._running:
                success = False
                break
            idx += 1
            pct = int((idx - 1) / total * 100) if total else 100
            self.progress.emit(pct)

            if cmd.cmd_type == "send":
                # parse inline delay if provided inside payload? We allow cmd.delay_ms
                payload = cmd.payload or ""
                data = parse_escaped_string(payload)
                self.log.emit("TX", safe_decode_ascii(data), data)
                # request main thread to send bytes
                self.send_bytes.emit(data)
                # optional inline delay
                if cmd.delay_ms:
                    self._sleep_ms(cmd.delay_ms)

            elif cmd.cmd_type == "expect":
                expected = cmd.payload
                # empty/None means nothing expected; skip waiting
                if expected is None:
                    self.log.emit("INFO", f"Expect: <none>", b"")
                else:
                    exp_bytes = parse_escaped_string(expected)
                    # wait until timeout for expected bytes to appear in buffer
                    start = time.time()
                    got = b""
                    matched = False
                    while (time.time() - start) * 1000 < self._expect_timeout_ms:
                        # consume available bytes
                        with self._recv_lock:
                            if self._recv_buffer:
                                got += bytes(self._recv_buffer)
                                self._recv_buffer.clear()
                        if exp_bytes == b"":
                            # expecting empty string - treat as immediate success
                            matched = True
                            break
                        if exp_bytes in got:
                            matched = True
                            break
                        time.sleep(0.02)
                    if matched:
                        self.log.emit("RX", safe_decode_ascii(exp_bytes), exp_bytes)
                    else:
                        # timed out - log what we received
                        self.log.emit("ERR", f"Expectation failed. Expected: {bytes_to_hex(exp_bytes)}", got)
                        success = False
                        # continue running; user may want full trace

            elif cmd.cmd_type == "delay":
                # payload is delay in ms
                try:
                    ms = int(cmd.payload or 0)
                    self._sleep_ms(ms)
                except Exception:
                    self.log.emit("ERR", f"Invalid delay value: {cmd.payload}", b"")
                    success = False

            # small inter-command pause
            time.sleep(0.01)

        # finish
        self.progress.emit(100)
        self.finished_ok.emit(success)

    def _sleep_ms(self, ms: int):
        end = time.time() + ms / 1000.0
        while time.time() < end and self._running:
            time.sleep(0.02)

    def stop(self):
        self._running = False
        self.wait(1000)


# ---------- Main Application GUI ----------
class SerialManager(QObject):
    """
    Encapsulate serial port open/close/send/receive operations and the reader thread.
    Emits signals when data arrives or connection status changes.
    """
    data_received = Signal(bytes)
    connected_changed = Signal(bool)
    error = Signal(str)

    def __init__(self):
        super().__init__()
        self._serial: Optional[serial.Serial] = None
        self._reader: Optional[SerialReaderWorker] = None

    def list_ports(self) -> List[Tuple[str, str]]:
        """Return list of (device, description)."""
        if serial is None:
            return []
        ports = list(serial.tools.list_ports.comports())
        return [(p.device, p.description) for p in ports]

    def open(self, port: str, baudrate: int, parity='N', stopbits=1, bytesize=8, timeout=0.1) -> bool:
        if serial is None:
            self.error.emit("pyserial not installed")
            return False
        try:
            # Map parity
            parity_map = {'N': serial.PARITY_NONE, 'E': serial.PARITY_EVEN, 'O': serial.PARITY_ODD}
            ser = serial.Serial(port=port,
                                baudrate=baudrate,
                                parity=parity_map.get(parity, serial.PARITY_NONE),
                                stopbits=stopbits,
                                bytesize=bytesize,
                                timeout=timeout)
            self._serial = ser
            # start reader
            self._reader = SerialReaderWorker(self._serial)
            self._reader.received.connect(self._on_received)
            self._reader.start()
            self.connected_changed.emit(True)
            return True
        except Exception as e:
            self.error.emit(str(e))
            return False

    def close(self):
        if self._reader:
            try:
                self._reader.stop()
            except Exception:
                pass
            self._reader = None
        if self._serial:
            try:
                self._serial.close()
            except Exception:
                pass
            self._serial = None
        self.connected_changed.emit(False)

    def write(self, data: bytes):
        try:
            if self._serial and self._serial.is_open:
                self._serial.write(data)
            else:
                self.error.emit("Serial port not open")
        except Exception as e:
            self.error.emit(str(e))

    @Slot(bytes)
    def _on_received(self, data: bytes):
        self.data_received.emit(data)

    def is_open(self) -> bool:
        return self._serial is not None and self._serial.is_open


class SettingsDialog(QDialog):
    def __init__(self, parent=None, settings: QSettings = None):
        super().__init__(parent)
        self.setWindowTitle("Serial Settings")
        self.setModal(True)
        self.settings = settings

        layout = QFormLayout(self)

        # Port is chosen from main GUI; we only allow showing/editing defaults here
        self.baud = QComboBox()
        self.baud.addItems(["9600", "19200", "38400", "57600", "115200", "230400"])
        self.parity = QComboBox()
        self.parity.addItems(["N", "E", "O"])
        self.stopbits = QComboBox()
        self.stopbits.addItems(["1", "1.5", "2"])

        layout.addRow("Default baud rate:", self.baud)
        layout.addRow("Default parity:", self.parity)
        layout.addRow("Default stop bits:", self.stopbits)

        buttons = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        buttons.accepted.connect(self.accept)
        buttons.rejected.connect(self.reject)
        layout.addWidget(buttons)

        # load settings
        if self.settings:
            self.baud.setCurrentText(str(self.settings.value("baud", "9600")))
            self.parity.setCurrentText(self.settings.value("parity", "N"))
            self.stopbits.setCurrentText(self.settings.value("stopbits", "1"))


    def accept(self) -> None:
        # save
        if self.settings:
            self.settings.setValue("baud", int(self.baud.currentText()))
            self.settings.setValue("parity", self.parity.currentText())
            self.settings.setValue("stopbits", float(self.stopbits.currentText()))
        super().accept()


class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("vfdSender GUI")
        self.resize(900, 600)
        self.settings = QSettings("VFDTools", "vfdSenderGUI")

        # Serial manager
        self.serial_mgr = SerialManager()
        self.serial_mgr.data_received.connect(self.on_serial_data)
        self.serial_mgr.connected_changed.connect(self.on_connection_changed)
        self.serial_mgr.error.connect(self.on_serial_error)

        # Script runner (worker)
        self.script_worker: Optional[ScriptRunnerWorker] = None

        # UI widgets
        self._build_ui()
        self._apply_dark_palette()

        # Load persisted settings
        self._load_settings()

        # connect updates
        self._refresh_ports()

    def closeEvent(self, event):
        # persist geometry
        self.settings.setValue("geometry", self.saveGeometry())
        self.settings.setValue("windowState", self.saveState())
        # ensure workers stopped
        if self.script_worker:
            self.script_worker.stop()
        self.serial_mgr.close()
        super().closeEvent(event)

    # ---------- UI ----------
    def _build_ui(self):
        # Central widget
        central = QWidget()
        self.setCentralWidget(central)
        vbox = QVBoxLayout(central)

        # Menu bar
        menu_bar = self.menuBar()
        file_menu = menu_bar.addMenu("&File")
        open_act = QAction("Open Script...", self)
        open_act.triggered.connect(self.load_script)
        save_act = QAction("Save Script As...", self)
        save_act.triggered.connect(self.save_script_as)
        exit_act = QAction("Exit", self)
        exit_act.triggered.connect(self.close)
        file_menu.addAction(open_act)
        file_menu.addAction(save_act)
        file_menu.addSeparator()
        file_menu.addAction(exit_act)

        serial_menu = menu_bar.addMenu("&Serial")
        refresh_ports_act = QAction("Refresh Ports", self)
        refresh_ports_act.triggered.connect(self._refresh_ports)
        connect_act = QAction("Connect", self)
        connect_act.triggered.connect(self.toggle_connect)
        serial_menu.addAction(refresh_ports_act)
        serial_menu.addAction(connect_act)

        settings_menu = menu_bar.addMenu("&Settings")
        serial_settings_act = QAction("Serial Defaults...", self)
        serial_settings_act.triggered.connect(self.open_settings_dialog)
        settings_menu.addAction(serial_settings_act)

        help_menu = menu_bar.addMenu("&Help")
        about_act = QAction("About", self)
        about_act.triggered.connect(self.about)
        help_menu.addAction(about_act)

        # Top: large text window (log)
        self.log_widget = QTextEdit()
        self.log_widget.setReadOnly(True)
        vbox.addWidget(self.log_widget, stretch=8)

        # Middle: entry + send button + port selection + baud
        hmid = QHBoxLayout()
        self.input_line = QLineEdit()
        self.input_line.setPlaceholderText(r'Enter text; use \xHH for hex escapes')
        self.send_button = QPushButton("Send")
        self.send_button.clicked.connect(self.on_send_clicked)

        self.port_combo = QComboBox()
        self.baud_combo = QComboBox()
        self.baud_combo.addItems(["9600", "19200", "38400", "57600", "115200", "230400"])
        self.port_refresh_button = QPushButton("Refresh")
        self.port_refresh_button.clicked.connect(self._refresh_ports)

        hmid.addWidget(QLabel("Port:"))
        hmid.addWidget(self.port_combo, stretch=2)
        hmid.addWidget(self.port_refresh_button)
        hmid.addWidget(QLabel("Baud:"))
        hmid.addWidget(self.baud_combo)
        hmid.addWidget(self.input_line, stretch=4)
        hmid.addWidget(self.send_button)
        vbox.addLayout(hmid)

        # Bottom: progress bar + connection indicator
        hbot = QHBoxLayout()
        self.progress = QProgressBar()
        self.progress.setRange(0, 100)
        self.conn_label = QLabel("Disconnected")
        self.conn_indicator = QLabel("\u25CF")  # circle character for LED
        self.conn_indicator.setStyleSheet("color: red; font-weight: bold;")
        hbot.addWidget(self.progress, stretch=1)
        hbot.addWidget(self.conn_indicator)
        hbot.addWidget(self.conn_label)
        vbox.addLayout(hbot)

        # Bind serial manager signals
        self.serial_mgr.data_received.connect(self._log_rx)

    # ---------- actions & helpers ----------
    def _apply_dark_palette(self):
        app = QApplication.instance()
        app.setStyle("Fusion")
        palette = QPalette()
        palette.setColor(QPalette.Window, QColor(35, 35, 35))
        palette.setColor(QPalette.WindowText, Qt.white)
        palette.setColor(QPalette.Base, QColor(18, 18, 18))
        palette.setColor(QPalette.AlternateBase, QColor(53, 53, 53))
        palette.setColor(QPalette.ToolTipBase, Qt.white)
        palette.setColor(QPalette.ToolTipText, Qt.white)
        palette.setColor(QPalette.Text, Qt.white)
        palette.setColor(QPalette.Button, QColor(48, 48, 48))
        palette.setColor(QPalette.ButtonText, Qt.white)
        palette.setColor(QPalette.Highlight, QColor(80, 160, 220))
        palette.setColor(QPalette.HighlightedText, Qt.black)
        app.setPalette(palette)

    def _load_settings(self):
        geom = self.settings.value("geometry")
        if geom:
            self.restoreGeometry(geom)
        state = self.settings.value("windowState")
        if state:
            self.restoreState(state)
        # load defaults into UI
        self.baud_combo.setCurrentText(str(self.settings.value("baud", "9600")))
        self.port_combo.setCurrentText(self.settings.value("port", ""))

    def _save_settings(self):
        self.settings.setValue("baud", int(self.baud_combo.currentText()))
        self.settings.setValue("port", self.port_combo.currentText())
        self.settings.setValue("parity", self.settings.value("parity", "N"))

    def _refresh_ports(self):
        self.port_combo.clear()
        ports = []
        if serial is not None:
            try:
                ports = list(serial.tools.list_ports.comports())
            except Exception:
                ports = []
        for p in ports:
            self.port_combo.addItem(p.device)
        if self.port_combo.count() == 0:
            self.port_combo.addItem("No ports")
        # restore persisted selection
        last_port = self.settings.value("port", "")
        if last_port:
            idx = self.port_combo.findText(last_port)
            if idx >= 0:
                self.port_combo.setCurrentIndex(idx)

    @Slot()
    def toggle_connect(self):
        if self.serial_mgr.is_open():
            self.serial_mgr.close()
            self._update_connection_ui(False)
        else:
            port = self.port_combo.currentText()
            baud = int(self.baud_combo.currentText())
            parity = self.settings.value("parity", "N")
            stopbits = float(self.settings.value("stopbits", 1))
            sb = 1 if stopbits == 1 else (2 if stopbits == 2 else 1)
            ok = self.serial_mgr.open(port, baud, parity=parity, stopbits=sb)
            if ok:
                self._save_settings()

    @Slot()
    def on_connection_changed(self, connected: bool):
        self._update_connection_ui(connected)

    def _update_connection_ui(self, connected: bool):
        if connected:
            self.conn_label.setText("Connected")
            self.conn_indicator.setStyleSheet("color: #00dd55; font-weight: bold;")
        else:
            self.conn_label.setText("Disconnected")
            self.conn_indicator.setStyleSheet("color: red; font-weight: bold;")

    @Slot(str)
    def on_serial_error(self, err: str):
        QMessageBox.critical(self, "Serial Error", err)
        self._update_connection_ui(False)

    @Slot(bytes)
    def on_serial_data(self, data: bytes):
        """
        This is called when SerialManager receives data.
        If a script worker is running, forward data to it as well.
        """
        # append to log
        self._log_rx(data)
        # forward to script runner if present
        if self.script_worker:
            self.script_worker.append_received(data)

    def _log_rx(self, data: bytes):
        ascii_text = safe_decode_ascii(data)
        hex_text = bytes_to_hex(data)
        self._append_log_line("RX", ascii_text, hex_text)

    def _append_log_line(self, tag: str, ascii_text: str, hex_text: str):
        # Format: [RX] ASCII | HEX
        cursor = self.log_widget.textCursor()
        cursor.movePosition(QTextCursor.End)
        line = f"[{tag}] ASCII: {ascii_text}  |  HEX: {hex_text}\n"
        self.log_widget.append(line)
        self.log_widget.ensureCursorVisible()

    @Slot()
    def on_send_clicked(self):
        text = self.input_line.text()
        if not text:
            return
        try:
            data = parse_escaped_string(text)
            # write
            self.serial_mgr.write(data)
            # local log
            self._append_log_line("TX", safe_decode_ascii(data), bytes_to_hex(data))
        except Exception as e:
            QMessageBox.critical(self, "Send Error", str(e))

    # ---------- Script load/save ----------
    def load_script(self):
        fname, _ = QFileDialog.getOpenFileName(self, "Open Script", "", "Text Files (*.txt);;All Files (*)")
        if not fname:
            return
        try:
            with open(fname, "r", encoding="utf-8") as fh:
                content = fh.read()
            # initial display in log
            self.log_widget.append(f"[INFO] Loaded script: {fname}\n{content}\n")
            # also parse immediately and run?
            # we leave run to user via 'Run Script' menu or add right-click later
            # For now, offer to run
            if QMessageBox.question(self, "Run Script", "Run this script now?") == QMessageBox.Yes:
                commands = self.parse_script(content)
                self.run_script(commands)
        except Exception as e:
            QMessageBox.critical(self, "Open Error", str(e))

    def save_script_as(self):
        fname, _ = QFileDialog.getSaveFileName(self, "Save Script As", "", "Text Files (*.txt);;All Files (*)")
        if not fname:
            return
        # write contents from a selection or current input line - we just save input_line for convenience
        try:
            with open(fname, "w", encoding="utf-8") as fh:
                # naive: save current input; user likely wants to edit scripts in an external editor
                fh.write(self.input_line.text() + "\n")
            QMessageBox.information(self, "Saved", f"Saved to {fname}")
        except Exception as e:
            QMessageBox.critical(self, "Save Error", str(e))

    # ---------- Script parsing ----------
    def parse_script(self, text: str) -> List[ScriptCommand]:
        """
        Parse a line-based script. Supported line forms:
            send: "Hello\x1B@" [, delay: 200]
            expect: "OK"
            delay: 500
        Lines starting with # or blank are ignored.
        """
        cmds: List[ScriptCommand] = []
        for raw in text.splitlines():
            line = raw.strip()
            if not line or line.startswith("#"):
                continue
            # Split by commas to allow inline 'delay' after send
            parts = [p.strip() for p in line.split(",")]
            primary = parts[0]
            if primary.lower().startswith("send:"):
                # extract quoted payload
                m = re.match(r'send:\s*"(.*)"\s*', primary, flags=re.IGNORECASE)
                payload = ""
                if m:
                    payload = m.group(1)
                # check for inline delay in remaining parts
                delay_ms = None
                for p in parts[1:]:
                    mm = re.match(r'delay:\s*(\d+)', p, flags=re.IGNORECASE)
                    if mm:
                        delay_ms = int(mm.group(1))
                cmds.append(ScriptCommand(cmd_type="send", payload=payload, delay_ms=delay_ms))
            elif primary.lower().startswith("expect:"):
                m = re.match(r'expect:\s*"(.*)"\s*', primary, flags=re.IGNORECASE)
                payload = None
                if m:
                    payload = m.group(1)
                    # interpret empty quotes as empty bytes (i.e. expect nothing)
                    # But we'll treat None as 'no expectation' and "" as expect empty
                cmds.append(ScriptCommand(cmd_type="expect", payload=payload))
            elif primary.lower().startswith("delay:"):
                m = re.match(r'delay:\s*(\d+)', primary, flags=re.IGNORECASE)
                if m:
                    cmds.append(ScriptCommand(cmd_type="delay", payload=m.group(1)))
            else:
                # unknown line - treat as send by default
                cmds.append(ScriptCommand(cmd_type="send", payload=line))
        return cmds

    # ---------- Script execution ----------
    def run_script(self, commands: List[ScriptCommand]):
        if not commands:
            QMessageBox.information(self, "Run Script", "No commands parsed.")
            return
        if not self.serial_mgr.is_open():
            QMessageBox.warning(self, "Serial Not Connected", "Please open a serial connection before running a script.")
            return
        # stop existing worker
        if self.script_worker:
            self.script_worker.stop()
            self.script_worker = None

        self.script_worker = ScriptRunnerWorker(commands)
        self.script_worker.log.connect(self._on_script_log)
        self.script_worker.progress.connect(self.progress.setValue)
        self.script_worker.finished_ok.connect(self._on_script_finished)
        self.script_worker.send_bytes.connect(self.serial_mgr.write)
        # also forward serial received data to script worker
        self.serial_mgr.data_received.connect(self.script_worker.append_received)
        self.script_worker.start()

    @Slot(str, str, bytes)
    def _on_script_log(self, tag: str, ascii_text: str, raw_bytes: bytes):
        # tag: 'TX'|'RX'|'INFO'|'ERR'
        hex_text = bytes_to_hex(raw_bytes) if raw_bytes else ""
        self._append_log_line(tag, ascii_text, hex_text)

    @Slot(bool)
    def _on_script_finished(self, ok: bool):
        if ok:
            self.log_widget.append("[INFO] Script finished successfully.\n")
        else:
            self.log_widget.append("[ERR] Script finished with failures.\n")
        self.progress.setValue(100)

    # ---------- dialogs ----------
    def open_settings_dialog(self):
        dlg = SettingsDialog(self, settings=self.settings)
        if dlg.exec():
            QMessageBox.information(self, "Saved", "Settings updated.")

    def about(self):
        QMessageBox.information(self, "About vfdSender GUI",
                                "vfdSender GUI\n\nA lightweight VFD testing utility.\n\n(Requires pyserial and PySide6)")

# ---------- main ----------
def main():
    app = QApplication(sys.argv)
    # instantiate and show main window
    w = MainWindow()
    w.show()
    sys.exit(app.exec())


if __name__ == "__main__":
    main()
