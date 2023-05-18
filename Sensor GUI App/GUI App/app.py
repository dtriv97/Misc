#!/usr/bin/env python3

import sys
from PyQt5.QtWidgets import QApplication

from mainwindow import MainWindow

def main(argv):
    app = QApplication(argv)
    window = MainWindow()
    window.show()

    return app.exec()

if __name__ == "__main__":
    sys.exit(main(sys.argv))
