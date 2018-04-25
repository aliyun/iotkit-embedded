import os

os.system('docker run --rm -it -v {}:/app aliiotcrosscompile sh'.format(os.getcwd()))
