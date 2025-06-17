.PHONY: all clean install test1 test2

all: install

install:
	python3 setup.py install --user

clean:
	rm -rf build dist *.egg-info pycache cjson.cpython* tests/__pycache__

test1: install
	pytest -vs tests/test1.py

test2: install
	pytest -vs tests/test2.py
