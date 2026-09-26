PYTHON ?= python3
-include local.mk
export GOLDEN_GATE IIX NULIB2 ACX CP2
.DEFAULT_GOAL := build
.PHONY: sdk doctor build run test package clean check
sdk doctor build run test package clean:
	$(PYTHON) tools/demo.py $@

check:
	$(PYTHON) -m unittest discover -s tests/tooling -v
