#!/usr/bin/env python3

from distutils.log import error
from ipaddress import ip_address
from nntplib import NNTPDataError
import sys
from PyQt5.QtWidgets import (QMainWindow,
    QWidget, QLabel, QLineEdit, QPushButton, QDoubleSpinBox, QSpacerItem,
    QHBoxLayout, QVBoxLayout, QGridLayout, QSizePolicy, QMessageBox)
from PyQt5.QtGui import QPixmap
from PyQt5 import QtCore, Qt
from PyQt5.QtChart import QChart, QChartView, QBarSet, QAbstractAxis, QLineSeries, QValueAxis
import socket
import queue

header_app_text = "Sensor Grapher GUI"
header_app_author = "Author: Dhairya Trivedi"

ip_placeholder = "127.0.0.1"
port_placeholder = "8080"

max_voltage = 100 # max range for y-axis in chart

MAX_COMMS_TIMEOUT = 5  # Timeout for 5 second (greater than slowest rate speed of 100ms)

class TestSocketThread(QtCore.QThread):
    valsUpdated = QtCore.pyqtSignal(float, int, int)
    socketError = QtCore.pyqtSignal(str)

    def __init__(self, ip_add, port_num, rate, duration):
        QtCore.QThread.__init__(self)
        self.ip_add = ip_add
        self.port_num = port_num
        self.rate = rate
        self.duration = duration
        self.socket = None
        self.running = False

    def setup_socket(self):
        try:
            self.socket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
            self.socket.settimeout(MAX_COMMS_TIMEOUT)   # 1-second timeout for rx/tx to terminate lost connections

            # Sending start message to device
            start_message = "TEST;CMD=START;DURATION="+str(self.duration)+";RATE="+str(self.rate)+";"
            self.socket.sendto(bytes(start_message, "utf-8"), (self.ip_add, self.port_num))

            # Waiting for start ack from device
            rxMsg, addr = self.socket.recvfrom(self.port_num)
            rxMsg = rxMsg.decode()
            parameters = rxMsg.split(';')
            if parameters[1] == "RESULT=STARTED":
                self.running = True
                return 1
            else:
                self.errorMsg = parameters[2].split('=')[1]
                return -1
        except:
            self.errorMsg = "Cannot connect to device, try again!"
            return -1

    def stop_socket(self):
        self.running = False
        self.socket.close()
    
    @QtCore.pyqtSlot()
    def force_terminate(self):
        stop_message = "TEST;CMD=STOP;"
        self.socket.sendto(bytes(stop_message, "utf-8"), (self.ip_add, self.port_num))
        self.stop_socket()

    def run(self):
        if self.setup_socket() > 0:
            while(self.running):
                try:
                    rxMsg, addr = self.socket.recvfrom(self.port_num)
                    rxMsg = rxMsg.decode()
                    if "STATUS" in rxMsg:
                        try:
                            parameters = rxMsg.split(';')
                            #TIME=ms
                            time = float(parameters[1].split('=')[1])
                            #MV=mv
                            mv = int(parameters[2].split('=')[1])
                            #MA=ma
                            ma = int(parameters[3].split('=')[1])
                            self.valsUpdated.emit(time, mv, ma)
                        except Exception as e:
                            print("INVALID PACKET DETECTED!")
                    elif "TEST" in rxMsg:
                        result = rxMsg.split(';')[1]
                        if (result == "RESULT=STOPPED"):
                            self.stop_socket()
                            self.valsUpdated.emit(-1,-1,-1)
                except:
                    pass
        else:
            self.socketError.emit(self.errorMsg)


class MainWindow(QMainWindow):
    forceTerminate = QtCore.pyqtSignal()

    def __init__(self):
        super().__init__()

        #Initialise test variables
        self.socket = None
        self.test_duration = 0
        self.test_rate = 0

        self.SocketThread = None
        
        # Initialise GUI elements
        self._gui_setup()
        self._gui_widget_connect_setup()

    def test_in_progress(self):
        self.start_pushbutton.setEnabled(False)
        self.stop_pushbutton.setEnabled(True)
        self.duration_combobox.setEnabled(False)
        self.rate_combobox.setEnabled(False)

        self.curve_amp.clear()
        self.curve_vol.clear()

    def test_idle(self):
        self.start_pushbutton.setEnabled(True)
        self.stop_pushbutton.setEnabled(False)
        self.duration_combobox.setEnabled(True)
        self.rate_combobox.setEnabled(True)

        self.SocketThread = None
    
    @QtCore.pyqtSlot(str)
    def socket_fail(self, errorMsg):
        print(errorMsg)
        self.popup_error("Failed to connect", errorMsg)
        self.test_idle()

    def on_duration_combobox(self, val):
        self.test_duration = val

    def on_rate_combobox(self, val):
        self.test_rate = val

    @QtCore.pyqtSlot(float, int, int)
    def update_vals(self,time, mv, ma):
        if (time > 0):
            self.curve_vol.append(time, mv)
            self.curve_amp.append(time, ma)
            self.axisX.setMax(time)
        else:
            # Negative time value indicates test completed
            self.test_idle()

    def on_start_button(self):
        self.test_in_progress()
        
        # Retrieve test device's ip address and port
        try:
            ip_add = ip_address(self.ip_lineedit.text())
            ip_port = int(self.port_lineedit.text())
        except:
            title = "Invalid IP Address"
            msg = "Please check the IP Address and Port are valid and try again"
            self.popup_error(title, msg)
            self.test_idle()
            return

        self.SocketThread = TestSocketThread(
            format(ip_add),
            ip_port,
            self.test_rate,
            self.test_duration
        )

        self.SocketThread.valsUpdated.connect(self.update_vals)
        self.SocketThread.socketError.connect(self.socket_fail)
        self.forceTerminate.connect(self.SocketThread.force_terminate)
        self.SocketThread.start()

    def popup_error(self, title, text):
        msg = QMessageBox()
        msg.setIcon(QMessageBox.Critical)
        msg.setText(title)
        msg.setInformativeText(text)
        msg.setWindowTitle("Error")
        msg.exec_()

    def on_stop_button(self):
        # Emit close signal to socket thread
        self.forceTerminate.emit()

        # Setup GUI for idle mode
        self.test_idle()

    def _gui_setup(self):
        self.setWindowTitle("Sensor Display App")

        # base layout
        self.page_layout = QVBoxLayout()
        self.page_layout.setContentsMargins(25, 25, 25, 50)

        # header
        self.header_layout = QGridLayout()
        self.page_layout.addLayout(self.header_layout)

        app_label = QLabel(header_app_text)
        app_label.setAlignment(QtCore.Qt.AlignRight)
        author_label = QLabel(header_app_author)
        author_label.setAlignment(QtCore.Qt.AlignRight)
        self.header_layout.addWidget(app_label, 0, 2)
        self.header_layout.addWidget(author_label, 1, 2)

        self.page_layout.addStretch()

        # test
        self.test_layout = QGridLayout()
        self.page_layout.addLayout(self.test_layout)

        ip_label = QLabel("IP address")
        self.ip_lineedit = QLineEdit()
        self.ip_lineedit.setPlaceholderText(ip_placeholder)
        port_label = QLabel("Port")
        self.port_lineedit = QLineEdit()
        self.port_lineedit.setPlaceholderText(port_placeholder)
        self.test_layout.addWidget(ip_label, 0, 0)
        self.test_layout.addWidget(self.ip_lineedit, 0, 1)
        self.test_layout.addWidget(port_label, 1, 0)
        self.test_layout.addWidget(self.port_lineedit, 1, 1)

        duration_label = QLabel("Duration")
        self.duration_combobox = QDoubleSpinBox()
        self.duration_combobox.setDecimals(0)
        duration_unit_label = QLabel("s")
        rate_label = QLabel("Rate")
        self.rate_combobox = QDoubleSpinBox()
        self.rate_combobox.setDecimals(0)
        rate_unit_label = QLabel("ms")
        self.test_layout.addWidget(duration_label, 2, 0)
        self.test_layout.addWidget(self.duration_combobox, 2, 1)
        self.test_layout.addWidget(duration_unit_label, 2, 2)
        self.test_layout.addWidget(rate_label, 3, 0)
        self.test_layout.addWidget(self.rate_combobox, 3, 1)
        self.test_layout.addWidget(rate_unit_label, 3, 2)

        self.start_pushbutton = QPushButton("Start")
        self.start_pushbutton.setMinimumSize(40, 20)
        self.stop_pushbutton = QPushButton("Stop")
        self.stop_pushbutton.setMinimumSize(40, 20)
        self.stop_pushbutton.setEnabled(False)
        self.test_layout.addWidget(self.start_pushbutton, 0, 4)
        self.test_layout.addWidget(self.stop_pushbutton, 0, 5)

        test_layout_spacer = QSpacerItem(40, 20, QSizePolicy.MinimumExpanding)
        self.test_layout.addItem(test_layout_spacer, 0, 3)

        # graph
        self.graph_layout = QVBoxLayout()
        self.graph_layout.setContentsMargins(0, 25, 0, 0)
        self.page_layout.addLayout(self.graph_layout)

        graph_title_label = QLabel("Graph")
        graph_title_label.setStyleSheet("font-weight: bold;")
        self.graph_layout.addWidget(graph_title_label)

        self.chart_view = QChartView()
        self.chart_view.setMinimumHeight(250)
        self.chart = self.chart_view.chart()
        self.chart.setTitle("Device values")
        self.graph_layout.addWidget(self.chart_view)

        #######################################################################
        self.curve_vol = QLineSeries()
        self.curve_vol.setName("Votage [mv]")
        self.chart.addSeries(self.curve_vol)

        self.curve_amp = QLineSeries()
        self.curve_amp.setName("Current [mA")
        self.chart.addSeries(self.curve_amp)
        ########################################################################

        self.axisX = QValueAxis()
        self.axisX.setRange(0, 1)

        self.axisY = QValueAxis()
        self.axisY.setRange(0, max_voltage)

        self.chart.setAxisX(self.axisX)
        self.chart.setAxisY(self.axisY)

        self.curve_vol.attachAxis(self.axisX)
        self.curve_vol.attachAxis(self.axisY)

        self.curve_amp.attachAxis(self.axisX)
        self.curve_amp.attachAxis(self.axisY)
        
        self.chart.axisX().setTitleText("Time [ms]")

        centralwidget = QWidget()
        centralwidget.setLayout(self.page_layout)
        self.setCentralWidget(centralwidget)

    def _gui_widget_connect_setup(self):
        self.start_pushbutton.pressed.connect(self.on_start_button)
        self.stop_pushbutton.pressed.connect(self.on_stop_button)

        self.duration_combobox.valueChanged.connect(self.on_duration_combobox)
        self.rate_combobox.valueChanged.connect(self.on_rate_combobox)
