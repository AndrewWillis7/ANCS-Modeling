PROJECT_NAME := anc-sim
CONFIG ?= sim_config.yaml
BUILD_DIR := build
DOCKER_IMAGE := anc-sim:latest
PYTHON := python3

.PHONY: help build cmake run sim clean docker-build docker-run docker-shell format dirs

help:
	@echo "Available targets:"
	@echo "  make build          - Configure and build C sources"
	@echo "  make run            - Run Python simulation entrypoint"
	@echo "  make sim            - Alias for run"
	@echo "  make clean          - Remove build and generated outputs"
	@echo "  make docker-build   - Build Docker image"
	@echo "  make docker-run     - Run container non-interactively"
	@echo "  make docker-shell   - Open shell inside container"
	@echo "  make dirs           - Create data/build directories"

dirs:
	mkdir -p $(BUILD_DIR)
	mkdir -p data/generated
	mkdir -p data/processed
	mkdir -p data/figures

cmake: dirs
	cmake -S . -B $(BUILD_DIR)

build: cmake
	cmake --build $(BUILD_DIR)

run: dirs build
	$(PYTHON) src/sim.py --config $(CONFIG)

sim: run

clean:
	rm -rf $(BUILD_DIR)
	rm -rf data/generated/*
	rm -rf data/processed/*
	rm -rf data/figures/*

docker-build:
	docker build -t $(DOCKER_IMAGE) .

docker-run:
	docker run --rm -it \
		-v $(PWD):/app \
		-w /app \
		$(DOCKER_IMAGE) \
		make run

docker-shell:
	docker run --rm -it \
		-v $(PWD):/app \
		-w /app \
		$(DOCKER_IMAGE) \
		/bin/bash